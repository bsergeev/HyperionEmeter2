#include "RecordingPlotter.h"
#include "Recording.h"
#include "RecordingDataModel.h"

#include <QPaintEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QColor>
#include <QAction>
#include <QToolTip>

//------------------------------------------------------------------------------

const short RecordingPlotter::GAP_X = 4;
const int   RecordingPlotter::NO_STEPS_PROCESSED = -1;
//bool      RecordingPlotter::kShowTooltip  = true;
//bool      RecordingPlotter::kShowTitle    = true;
//bool      RecordingPlotter::kShowSubTitle = true;

//------------------------------------------------------------------------------

RecordingPlotter::RecordingPlotter(const std::shared_ptr<RecordingDataModel>& model,
    QWidget* parent)
    : QAbstractItemView(parent)
    , m_model(model)
    , m_ParentWnd(parent)
	, m_Font(QFont("Arial", 12))
{
	ComputeTicks();
	m_margin[eLEFT] = m_margin[eRIGHT] = m_margin[eUP] = m_margin[eDOWN] = 5; // it'll be set in UpdateScrMargins
}
//------------------------------------------------------------------------------
RecordingPlotter::~RecordingPlotter()
{
}
//------------------------------------------------------------------------------

void RecordingPlotter::AdjustScrMargins()
{
	QFontMetrics metrics(m_Font);
	int h = metrics.height();
	int w = metrics.width("0.0000");
	short const gapX = 4;

	QString title;
	QString subTitle;
	if (RecordingDataModel* const model = m_model.get()) {
		title    = model->GetRecordingTitle();
		subTitle = model->GetRecordingSubTitle();
	}
	m_margin[eUP] = h * (1
		+ ((m_showTitle    && !title.   isEmpty())? 1 : 0)
		+ ((m_showSubTitle && !subTitle.isEmpty())? 1 : 0));

	m_margin[eDOWN] = (short)floor(2.5*h);

	m_margin[eLEFT] = w + h + 2 * gapX;


	// Depending on the current curve visibility (and active sensors)
	// there may be up to two extra Y-axes to the right
	m_margin[eRIGHT] = /* GetNRightAxes()* */(w + h + 2 * gapX);
}
//------------------------------------------------------------------------------
QModelIndex RecordingPlotter::indexAt(const QPoint&) const
{
    return QModelIndex();
}
//------------------------------------------------------------------------------
void RecordingPlotter::ComputeTicks(int minTickNumber)
{
	if (RecordingDataModel* const model = m_model.get())
	{
		const Recording& recording = model->GetRecording();
		const size_t N = recording.numColums();
		m_tickV.resize(N);

		size_t maxNticks = 0; // for vertical only
		for (size_t col = 0; col < N; ++col)
		{
			const CurveInfo& ci = recording.GetCurveInfo(ColumnIdx(col));
			double maxV = ci.maxV;
			double minV = ci.minV;
			double r = (minV <= maxV)? maxV - minV : 0.0;
			if (r == 0.0) {
				if (minV <= 0.0) maxV = 1;
				else  	         minV = maxV / 2;
				r = maxV - minV;
			}
			double step = 0.0;
			if (col == 0  &&  maxV >= 60.0) // below 60 seconds it doesn't matter
			{
				// Values are seconds.
				// If time is more than 60sec (which we've just checked),
				// values are displayed as M:SS. For times to look pretty,
				// we'd like to have 5, 10, 15, 20, 30, 60sec steps.
				int Nmin = 8;
				double grossStep = r / Nmin;

				if (grossStep < 10) step = 10; // and it's > 5, as 60sec / 8 = ca. 7
				else if (grossStep <  15) step = 15;
				else if (grossStep <  20) step = 20;
				else if (grossStep <  30) step = 30;
				else if (grossStep <= 60) step = 60;
				else // grossStep > 60
				{
					// step =  floor(grossStep/60)*60;
					Nmin = 7;
					const int    Nopt = 8;
					constexpr double log2 = .69314718055994530941723212145818;

					r /= 60; //  seconds -> minutes
					step = pow(10.0, floor(log10(r)));
					// for 10 <= r < 100,  step = 10
					//    100 <= r < 1000, step = 100

					int n = floor(r / step) + 1;
					if (n < Nmin)
					{
						int m = (int)floor(log((double)Nopt / n) / log2); // = log2(Nopt/n)
						if (m == 0)
							m = 1;
						step /= pow(2.0, m);
					}
					step *= 60;
				}
			}
			else
			{
				int Nmin = (minTickNumber > 0) ? minTickNumber : 4;
				double grossStep = r / Nmin;
				step = pow(10.0, floor(log10(grossStep)));
				// for 10 <= grossStep < 100,  step = 10
				//    100 <= grossStep < 1000, step = 100

				if (5 * step < grossStep)
					step *= 5;
				else if (2 * step < grossStep)
					step *= 2;
			}

			size_t iMin = (size_t)floor(minV / step);
			size_t iMax = (size_t)ceil (maxV / step);
			if (iMax * step < maxV + 0.00001) {
				++iMax;
			}

			auto& ticks = m_tickV[col];
			const size_t Nticks = iMax - iMin + 1;
			if (col > 0 && maxNticks < Nticks) { maxNticks = Nticks; }
			ticks.resize(Nticks);
			for (size_t i = iMin; i <= iMax; ++i) {
				ticks[i - iMin] = step * i;
			}
		}

		// Adjust curves with fewer ticks by adding extra spans up (as all our values are positive)
		for (size_t col = 1; col < N; ++col) // don't change Seconds
		{
			auto& ticks = m_tickV[col];
			size_t NTicks = ticks.size();
			if (NTicks > 1 && NTicks < maxNticks)
			{
				const double dTick = ticks[1] - ticks[0];
				double lastTickValue = ticks.back();
				for (size_t i = 0; i < maxNticks - NTicks; ++i) {
					ticks.push_back(lastTickValue += dTick);
				}
			}
		}
	}
}
//------------------------------------------------------------------------------
int RecordingPlotter::ComputeScrCoord(double v, double minV, double maxV, bool horizontal) const
{
	int scrCoord = 0;
	if (horizontal) {
		scrCoord = m_margin[eLEFT]
			+ (int)floor((v - minV)*(width() - (m_margin[eLEFT] + m_margin[eRIGHT]) - 1)
				                   / (maxV - minV)); // is "-1" needed?
	} else { // X
		scrCoord = m_margin[eUP]
			+ (int)floor((maxV - v)*(height() - (m_margin[eUP] + m_margin[eDOWN]) - 1)
				                   / (maxV - minV)); // is "-1" needed?
	}
	return scrCoord;
}
//------------------------------------------------------------------------------
void RecordingPlotter::paintEvent(QPaintEvent*)
{
	//QImage image(width(), height(), QImage::Format_RGB32);
	QPainter painter(viewport()); //  &image);

	// Draw grid
	const QRect scrRect{ 0, 0, width(), height() };
	QRect chartRect = scrRect.adjusted(m_margin[eLEFT],
									   m_margin[eUP],
									  -m_margin[eRIGHT],
									  -m_margin[eDOWN]);
	painter.fillRect(QRect(0,0,width(), height()), Qt::white);

	// Draw grid .......................................................
	painter.fillRect(chartRect, QColor("#F4F4FF")); //  MainWnd::kGraphBkgrColor
	painter.setPen(Qt::black);
	painter.drawRect(chartRect);

	// First, draw the grid lines - - - - - - - - - - - - - - - - - - - - - - -
	painter.setClipRect(chartRect.x() + 1, chartRect.y() + 1, chartRect.width() - 2, chartRect.height() - 2);

	// Vertical then horizontal lines
	for (size_t i=0; i<2; ++i)
	{
		const auto& tics = m_tickV[i]; // seconds then 1st vertical
		const double minV = tics.front();
		const double maxV = tics.back();
		for (size_t tickIdx = 0; tickIdx < tics.size(); ++tickIdx)
		{
			double t = tics[tickIdx];
			if (minV <= t  &&  t <= maxV)
			{
				int v = ComputeScrCoord(t, minV, maxV, i==0); // 0 => horizontal
				painter.setPen(QPen(QBrush(Qt::gray, Qt::SolidPattern), 0, Qt::DashLine));
				if (i == 0) {
					painter.drawLine(v, chartRect.top(), v, chartRect.bottom());
				} else {
					painter.drawLine(chartRect.left(), v, chartRect.right(), v);
				}
			}
		}
    }

}


