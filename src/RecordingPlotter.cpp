#include "RecordingPlotter.h"
#include "Recording.h"
#include "RecordingDataModel.h"

#include <QPaintEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QTextStream>
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
//static 
QString RecordingPlotter::SecondsTxt(double sec)
{
	QString txt;
	QTextStream s(&txt, QIODevice::WriteOnly);
	s.setPadChar('0');
	s.setFieldAlignment(QTextStream::AlignRight);

	if (sec >= 60.0)
	{
		int hours = (int)floor(sec / 3600);
		int minutes = (int)floor((sec - hours * 3600) / 60);
		sec -= hours * 3600 + minutes * 60;
		if (hours) {
			s << hours << ":" << qSetFieldWidth(2);
		}
		s << minutes << qSetFieldWidth(0) << ":" << qSetFieldWidth(2) << sec;
	}
	else // less than a minute
	{
		txt = QString("%1").arg(sec);
	}

	return txt;
}
//------------------------------------------------------------------------------
void RecordingPlotter::paintEvent(QPaintEvent*)
{
	AdjustScrMargins();

	QPainter painter(viewport());

	// Draw chart area
	const QRect scrRect{ 0, 0, width(), height() };
	painter.fillRect(scrRect, Qt::white);

	QRect chartRect = scrRect.adjusted(m_margin[eLEFT], m_margin[eUP], -m_margin[eRIGHT], -m_margin[eDOWN]);
	painter.fillRect(chartRect, QColor("#F4F4FF")); //  MainWnd::kGraphBkgrColor

	painter.setPen(Qt::black);
	painter.drawRect(chartRect);

	// Tick values and names . . . . . . . . . . . . . . . . . . . . . . . . . .
	QFontMetrics metrics = painter.fontMetrics();

	if (RecordingDataModel* const model = m_model.get())
	{
		const Recording& recording = model->GetRecording();
		const size_t N_points = recording.size();
		const size_t N_curves = recording.numColums();
		const ColumnIdx ci0 = ColumnIdx{ 0 };

		// Tick values along X-axis
		painter.setPen(Qt::black);
		const auto& ticsX = m_tickV.at(0);
		const double minX = ticsX.front();
		const double maxX = ticsX.back();
		for (auto t : ticsX)
		{
			const int x = ComputeScrCoord(t, minX, maxX, true); // horizontal
			QString txt = SecondsTxt(t);
			painter.drawText(x-50, chartRect.bottom()+5, 100, metrics.height(), Qt::AlignHCenter|Qt::AlignTop, txt);
		}
		// Draw horizontal axis title
		QRect bottomRect(0, height() - metrics.height() - 1, width(), metrics.height());
		painter.drawText(bottomRect, Qt::AlignHCenter|Qt::AlignVCenter, recording.SeriesName(ci0));


		painter.setClipRect(chartRect.x() + 1, chartRect.y() + 1, chartRect.width() - 2, chartRect.height() - 2);

		// Vertical then horizontal lines
		painter.setPen(QPen(QBrush(Qt::gray, Qt::SolidPattern), 0, Qt::DashLine));
		for (size_t i = 0; i < 2; ++i)
		{
			const auto& tics = m_tickV[i]; // seconds then 1st vertical
			const double minV = tics.front();
			const double maxV = tics.back();
			for (size_t tickIdx = 0; tickIdx < tics.size(); ++tickIdx)
			{
				double t = tics[tickIdx];
				if (minV <= t  &&  t <= maxV)
				{
					int v = ComputeScrCoord(t, minV, maxV, i == 0); // 0 => horizontal
					if (i == 0) {
						painter.drawLine(v, chartRect.top(), v, chartRect.bottom());
					} else {
						painter.drawLine(chartRect.left(), v, chartRect.right(), v);
					}
				}
			}
		}

		// Draw curves
		// First, pre-compute X screen coords for Seconds column
		std::vector<double> scrX(N_points, 0.0);
		const auto ticksSec = m_tickV.at(0);
		const double firstSec = ticksSec.front();
		const double lastSec  = ticksSec.back();
		for (size_t row = 0; row < N_points; ++row) {
			const double x = recording.GetValue(row, ci0);
			scrX[row] = ComputeScrCoord(x, firstSec, lastSec, true); // horizontal
		}

		// Next, process all other columns skipping 1st column, as it's X-axis
		const size_t N_COLORS = 13;
		static const std::array<Qt::GlobalColor, N_COLORS> curveColor = {
			Qt::black,
			Qt::red,
			Qt::green,
			Qt::blue,
			Qt::cyan,
			Qt::magenta,
			Qt::yellow,
			Qt::darkRed,
			Qt::darkGreen,
			Qt::darkBlue,
			Qt::darkCyan,
			Qt::darkMagenta,
			Qt::darkYellow
		};
		for (size_t col = 1; col < N_curves; ++col) 
		{
			painter.setPen(QPen(QBrush(curveColor[col], Qt::SolidPattern), 0, Qt::SolidLine));
			const auto&  ticks  = m_tickV.at(col);
			const double firstY = ticks.front();
			const double lastY  = ticks.back();

			const ColumnIdx colIdx = ColumnIdx{ col };
			int prevY = 0;
			for (size_t row = 0; row < N_points; ++row) 
			{
				const double y = recording.GetValue(row, colIdx);
				const int scrY = ComputeScrCoord(y, firstY, lastY, false); // vertical
				if (row > 0) {
					painter.drawLine(scrX[row-1], prevY, scrX[row], scrY);
				}
				prevY = scrY;
			}
		}
	}

}