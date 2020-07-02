#include "RecordingPlotter.h"

#include "DefaultValues.h"
#include "Recording.h"
#include "RecordingDataModel.h"

#include <QAction>
#include <QColor>
#include <QtDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QSettings>
#include <QStringRef>
#include <QTextStream>
#include <QToolTip>

#include <iostream> // <<< DEBUG
#include <sstream>

// static
DefaultBools RecordingPlotter::sCurveVisible{ true };
QColor       RecordingPlotter::kGraphBkgrColor { "#F4F4FF" };
QColor       RecordingPlotter::kGraphGridColor { Qt::gray  };
QColor       RecordingPlotter::kGraphFrameColor{ Qt::black };
bool         RecordingPlotter::sShowTitle    = true;
bool         RecordingPlotter::sShowSubTitle = true;
bool         RecordingPlotter::sShowTooltip  = true;

//------------------------------------------------------------------------------

RecordingPlotter::RecordingPlotter(const std::shared_ptr<RecordingDataModel>& model,
                                   QWidget* parent)
    : QAbstractItemView(parent)
    , m_model(model)
    , m_Font(QFont("Arial", 12))
    , m_RubberBand(QRubberBand::Rectangle, this)
{
    ReadSettings();

    setModel(model.get());
    setSelectionMode(ExtendedSelection);
    setSelectionBehavior(SelectItems);

    setBackgroundRole(QPalette::Light);
    setAutoFillBackground(true);

    ComputeTicks();
    m_margin[eLEFT] = m_margin[eRIGHT] = m_margin[eUP] = m_margin[eDOWN] = 5; // it'll be set in UpdateScrMargins

    const Recording& recording = model->GetRecording();
    const size_t    numColumns = recording.numColums();

    // Initialize curve visibility from the defaults, static sCurveVisible, but 
    // only for existing columns.
    m_curveVisible.resize(numColumns);
    for (size_t ci = 0;  ci < numColumns;  ++ci) {
        m_curveVisible.at(ci) = sCurveVisible.defaultInRecording(recording, ci);
    }
}
//------------------------------------------------------------------------------
RecordingPlotter::~RecordingPlotter()
{
    WriteSettings();
}
//------------------------------------------------------------------------------

void RecordingPlotter::AdjustScrMargins()
{
    QFontMetrics metrics(m_Font);
    int h = metrics.height();
    int w = metrics.horizontalAdvance("0.0000");
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

    m_margin[eDOWN] = static_cast<int>(floor(2.5*h));

    m_margin[eLEFT] = w + h + 2 * gapX;

    // Depending on the current curve visibility (and active sensors)
    // there may be up to two extra Y-axes to the right
    m_margin[eRIGHT] = static_cast<int>(GetNRightAxes()*(w + h + 2*gapX));
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
                else               minV = maxV / 2;
                r = maxV - minV;
            }
            double step = 0.0;
            if (col == 0  &&  maxV >= 60.0) // below 60 seconds it doesn't matter
            {
                // Values are seconds.
                // Since time is more than 60sec, values are displayed as M:SS. 
                // For times to look pretty, we'd like to have 5, 10, 15, 20, 30, 60sec steps.
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
size_t RecordingPlotter::GetNRightAxes() const
{
    const Recording& rec = m_model->GetRecording();
    const size_t N_curves = rec.numColums();
    size_t N_visible_curves = 1;
    for (size_t i = 0; i < N_curves; ++i) {
        if (m_curveVisible.at(i)) {
            ++N_visible_curves;
        }
    }
    const size_t nAxes = 1 + (N_visible_curves > 3)? (N_visible_curves-3) : 0;
    return nAxes;
}
//------------------------------------------------------------------------------
void RecordingPlotter::paintEvent(QPaintEvent*)
{
    AdjustScrMargins();

    QPainter painter(viewport());
    painter.setFont(m_Font);

    // Draw chart area
    const QRect scrRect{ 0, 0, width(), height() };
    painter.fillRect(scrRect, Qt::white);

    QRect chartRect = scrRect.adjusted(m_margin[eLEFT], m_margin[eUP], -m_margin[eRIGHT], -m_margin[eDOWN]);
    painter.fillRect(chartRect, kGraphBkgrColor);

    painter.setPen(Qt::black);
    painter.drawRect(chartRect);

    QFontMetrics metrics = painter.fontMetrics();

    if (RecordingDataModel* const model = m_model.get())
    {
        const Recording& recording = model->GetRecording();

        // Title [& sub-title] . . . . . . . . . . . . . . . . . . . . . . . . .
        const QString plotDescription = QString::fromStdString(recording.GetTitle()); //  "Downloaded from RDU/MDU";
        std::stringstream ss;
        const QString subtitle = QString::fromStdString(recording.GetSubtitle());
        bool title_shown = false;
        if ((title_shown = (m_showTitle && !plotDescription.isEmpty())) == true)
        {
            QRect topRect(0, 0, width(), metrics.height());
            painter.fillRect(topRect, Qt::white);
            painter.drawText(topRect, Qt::AlignHCenter | Qt::AlignVCenter, plotDescription);
        }
        if (m_showSubTitle && !subtitle.isEmpty())
        {
            QRect topRect(0, (title_shown) ? 1.5*metrics.height() : 0, width(), metrics.height());
            painter.fillRect(topRect, Qt::white);
            painter.drawText(topRect, Qt::AlignHCenter | Qt::AlignVCenter, subtitle);
        }


        // Tick values and names . . . . . . . . . . . . . . . . . . . . . . . .
        const size_t N_points = recording.size();
        const size_t N_curves = recording.numColums();
        const ColumnIdx ci0 = ColumnIdx{ 0 };

        // Tick values along X-axis . . . . . . . . . . . . . . . . . . . . . . .
        painter.setPen(Qt::black);
        const auto& ticsX = m_tickV.at(0);
        const double minX = ticsX.front();
        const double maxX = ticsX.back();
        for (auto t : ticsX) {
            const int x = ComputeScrCoord(t, minX, maxX, true); // horizontal
            QString txt = SecondsTxt(t);
            painter.drawText(x-50, chartRect.bottom()+5, 100, metrics.height(), Qt::AlignHCenter|Qt::AlignTop, txt);
        }
        // Draw horizontal axis title
        QRect bottomRect(m_margin[eLEFT], height() - metrics.height() - 1, 
                         width()-(m_margin[eLEFT]+m_margin[eRIGHT]), metrics.height());
        const QString xAxisTitle = (maxX > 60)? tr("Time") : tr("Seconds");
        painter.drawText(bottomRect, Qt::AlignHCenter | Qt::AlignVCenter, xAxisTitle);


        // Draw vertical axes/values/titles . . . . . . . . . . . . . . . . . . .
        const int strPixelHeight = metrics.height();
        const int tickValueWidth = metrics.horizontalAdvance("0.00");// 00");
        const size_t gapX = 4;
        const size_t ticW = 4;
        const size_t N_rAxes = GetNRightAxes();
        const int dX_per_RAxis = static_cast<int>(floor(m_margin[eRIGHT] / static_cast<double>(N_rAxes)));

        unsigned short RAxis_processed = 0;
        int Xtitle_Voltage = -1;
        int Xtitle_Current = -1;

        QPen bgPen(QBrush(Qt::white, Qt::SolidPattern), 0, Qt::SolidLine);
        for (size_t crvIdx = 1; crvIdx < N_curves; ++crvIdx)
        {
            if (!m_curveVisible[crvIdx])
                continue;

            const ColumnIdx colIdx = ColumnIdx{ crvIdx };
            const SamplePoint::ValueIndex curveType = recording.GetColumnType(colIdx);

            // These will be set depending on which Y-axis we are drawing
            Qt::AlignmentFlag alignH = Qt::AlignRight;
            float shiftAxisTitle = 0.0;
            int scrX = 0;
            int Xtitle = 0;

            switch (curveType)
            {
            case SamplePoint::eVolts:
                alignH = Qt::AlignRight;
                scrX = chartRect.left();
                Xtitle = Xtitle_Voltage = gapX;
                //shiftAxisTitle = ((m_curveVisible[recording.GetColumnOfType(SamplePoint::ePowerIn )])? -0.5 : 0) + 
                //                 ((m_curveVisible[recording.GetColumnOfType(SamplePoint::ePowerOut)])? -0.5 : 0);
                break;
            case SamplePoint::eAmps:
                alignH = Qt::AlignLeft;
                scrX = chartRect.right();
                Xtitle_Current = static_cast<int>(Xtitle = scrX + gapX + tickValueWidth);
                shiftAxisTitle = (m_curveVisible[recording.GetColumnOfType(SamplePoint::emAh_Out)])? -0.5 : 0;
                break;
            //case kIdxPower:
            //    if (m_PlotSettingsMgr.IsCurveVisible(kIdxVoltage)) {
            //        alignH = Qt::AlignLeft;
            //        scrX = chartRect.left();
            //        Xtitle = gapX;
            //        shiftAxisTitle = 0.5;
            //    }
            //    else { // Voltage not visible
            //        alignH = Qt::AlignRight;
            //        scrX = chartRect.left();
            //        Xtitle = Xtitle_Voltage = gapX;
            //        shiftAxisTitle = (m_PlotSettingsMgr.IsCurveVisible(kIdxPOut)) ? -0.5 : 0;
            //    }
            //    break;
            case SamplePoint::emAh_Out:
                alignH = Qt::AlignRight;
                scrX = chartRect.right();
                Xtitle = scrX + gapX + tickValueWidth;
                shiftAxisTitle = (m_curveVisible[recording.GetColumnOfType(SamplePoint::eAmps)])? 0.5 : 0;
                break;
            case SamplePoint::eRPM:
                alignH = Qt::AlignLeft;
                scrX = chartRect.right() + (++RAxis_processed)*dX_per_RAxis;
                break;
            //case kIdxPOut:
            //    if (m_PlotSettingsMgr.IsCurveVisible(kIdxVoltage)) {
            //        alignH = Qt::AlignLeft;
            //        scrX = chartRect.left();
            //        Xtitle = gapX;
            //        shiftAxisTitle = (m_PlotSettingsMgr.IsCurveVisible(kIdxPower)) ? 1.5 : 0.5;
            //    }
            //    else { // Voltage not visible
            //        alignH = Qt::AlignRight;
            //        scrX = chartRect.left();
            //        Xtitle = Xtitle_Voltage = gapX;
            //        shiftAxisTitle = (m_PlotSettingsMgr.IsCurveVisible(kIdxPower)) ? 0.5 : 0;
            //    }
            //    break;
            case SamplePoint::eTemp1:
                alignH = Qt::AlignLeft;
                scrX = chartRect.right() + (++RAxis_processed)*dX_per_RAxis;
                shiftAxisTitle = (m_curveVisible[recording.GetColumnOfType(SamplePoint::eTemp2)])? -0.5 : 0;
                break;
            case SamplePoint::eTemp2:
                alignH = Qt::AlignLeft;
                //if (m_curveVisible[recording.GetColumnOfType(SamplePoint::eTemp1)])
                //    --RAxis_processed;
                scrX = chartRect.right() + (++RAxis_processed)*dX_per_RAxis;
                shiftAxisTitle = (m_curveVisible[recording.GetColumnOfType(SamplePoint::eTemp1)])? 0.5 : 0;
                break;
            case SamplePoint::eAltitude:
            default:
                alignH = Qt::AlignLeft;
                scrX = chartRect.right() + (++RAxis_processed)*dX_per_RAxis;
                break;
            }
            if (curveType >= SamplePoint::eRPM) { // && curveType != SamplePoint::ePowerOut) {
                Xtitle = scrX + gapX + tickValueWidth;
                painter.setPen(QPen(QBrush(Qt::black, Qt::SolidPattern), 0, Qt::SolidLine));
                painter.drawLine(scrX, chartRect.top(), scrX, chartRect.bottom());
            }

            int MAXvalueStrWidth = 0;
            QPen colorPen(QBrush(m_model->GetColumnColor(crvIdx), Qt::SolidPattern), 0, Qt::SolidLine);

            const auto& ticks = m_tickV.at(crvIdx);
            const double firstY = ticks.front();
            const double lastY  = ticks.back();

            for (short tickIdx = 0; tickIdx < ticks.size(); ++tickIdx)
            {
                const double t = ticks[tickIdx];
                const int scrY = ComputeScrCoord(t, firstY, lastY, false); // vertical
                if (scrY >= chartRect.top() && scrY <= chartRect.bottom())
                {
                    // Draw little tick marks left of the additional Y-axes
                    if (curveType >= SamplePoint::eRPM) {
                        painter.setPen(QPen(QBrush(Qt::black, Qt::SolidPattern), 0, Qt::SolidLine));
                        painter.drawLine(scrX, scrY, scrX - ticW, scrY);
                    }

                    QString valueStr = QString::number(t);
                    int valueStrWidth = metrics.horizontalAdvance(valueStr);
                    if (MAXvalueStrWidth < valueStrWidth) MAXvalueStrWidth = valueStrWidth;

                    // For text inside the plot area, get the bounding rectangle
                    // and fill it with background color.
                    QRect br(scrX - ((alignH == Qt::AlignRight) ? 1 : 0)*valueStrWidth - ((alignH == Qt::AlignRight) ? 1 : -1)*gapX,
                        scrY - (strPixelHeight >> 1), valueStrWidth, strPixelHeight);
                    if ((//(curveType == SamplePoint::ePowerIn && m_curveVisible[recording.GetColumnOfType(eVolts)]) ||
                         curveType == SamplePoint::emAh_Out)  // only Power & Charge can be inside
                        && tickIdx > 0 && tickIdx + 1 < ticks.size()) // don't do it for the 1st and the last
                    {
                        painter.setPen(bgPen);
                        painter.drawText(br, alignH | Qt::AlignVCenter, valueStr, &br);
                        painter.fillRect(br, QBrush(kGraphBkgrColor, Qt::SolidPattern));
                    }
                    painter.setPen(colorPen);
                    painter.drawText(br, alignH | Qt::AlignVCenter, valueStr);
                }
            }

            // Draw the axis title
            const QString txt = recording.SeriesName(colIdx);
            int strPixelWidth = metrics.horizontalAdvance(txt);
            int yTitle = -height() / 2 + shiftAxisTitle*(strPixelWidth + 2 * gapX);

            // Move axes titles closer to the axes, if the values are shorter than anticipated
            //if (crvIdx != kIdxPower &&  crvIdx != kIdxPOut && crvIdx != kIdxCharge)
            //{
            //    if (tickValueWidth > MAXvalueStrWidth)
            //        Xtitle += ((crvIdx == kIdxVoltage) ? 1 : -1) * (tickValueWidth - MAXvalueStrWidth);

            //    if (crvIdx == kIdxVoltage)
            //        Xtitle_Voltage = Xtitle;
            //    else if (crvIdx == kIdxCurrent)
            //        Xtitle_Current = Xtitle;
            //}
            //else //  crvIdx == kIdxPower  ||  crvIdx == kIdxCharge
            //{
            //    if (Xtitle_Voltage > 0 && (crvIdx == kIdxPower || crvIdx == kIdxPOut))
            //        Xtitle = Xtitle_Voltage;
            //    else if (Xtitle_Current > 0 && crvIdx == kIdxCharge)
            //        Xtitle = Xtitle_Current;
            //}

            painter.rotate(-90);
            painter.drawText(yTitle, Xtitle, strPixelWidth, strPixelHeight, Qt::AlignHCenter | Qt::AlignVCenter, txt);
            painter.resetTransform();
        } // end of "crvIdx" loop


        // Restrict all further drawing to the chart area
        painter.setClipRect(chartRect.x() + 1, chartRect.y() + 1, chartRect.width() - 2, chartRect.height() - 2);

        // Vertical then horizontal grid lines . . . . . . . . . . . . . . . . . 
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

        // Draw curves . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
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
        QItemSelectionModel* selections = selectionModel();
        for (size_t col = 1; col < N_curves; ++col) 
        {
            if (m_curveVisible.at(col)) {
                painter.setPen(QPen(QBrush(m_model->GetColumnColor(col), Qt::SolidPattern), 2, Qt::SolidLine));
                const auto&  ticks = m_tickV.at(col);
                const double firstY = ticks.front();
                const double lastY  = ticks.back();

                const ColumnIdx colIdx = ColumnIdx{ col };
                int prevY = 0;
                for (size_t row = 0; row < N_points; ++row)
                {
                    const double y = recording.GetValue(row, colIdx);
                    const int scrY = ComputeScrCoord(y, firstY, lastY); // vertical
                    if (row > 0) {
                        painter.drawLine(scrX[row - 1], prevY, scrX[row], scrY);
                    }

                    // Draw selection markers
                    if (selections->isSelected(model->index((int)row,        0, rootIndex()))
                     || selections->isSelected(model->index((int)row, (int)col, rootIndex())))
                    {
                        int const D = 6; // <<< DEBUG
                        painter.drawLine(scrX[row] - D, scrY, scrX[row], scrY - D);
                        painter.drawLine(scrX[row], scrY - D, scrX[row] + D, scrY);
                        painter.drawLine(scrX[row] + D, scrY, scrX[row], scrY + D);
                        painter.drawLine(scrX[row], scrY + D, scrX[row] - D, scrY);
                    }
                    prevY = scrY;
                }
            }
        }
    }
}

void RecordingPlotter::mouseMoveEvent(QMouseEvent* event)
{
    bool done = false;

    if (event->buttons() & Qt::LeftButton)
    {
        if (RecordingDataModel* const model = m_model.get())
        {
            int xp = event->pos().x() + 1;
            if (xp < m_margin[eLEFT] || xp > width() - m_margin[eRIGHT]) {
                m_RubberBand.hide();
                return;
            }

            const Recording& recording = model->GetRecording();
            const size_t N_points = recording.size();
            const size_t N_curves = recording.numColums();
            const ColumnIdx ci0 = ColumnIdx{ 0 };


            QRect rubberBandRect(xp, m_margin[eUP] + 1, 1, height() - m_margin[eUP] - m_margin[eDOWN]);
            m_RubberBand.setGeometry(rubberBandRect);
            m_RubberBand.show();

            // First, find where xp falls
            const auto ticksSec = m_tickV.at(0);
            const double firstSec = ticksSec.front();
            const double lastSec  = ticksSec.back();
            for (size_t row = 0; row < N_points; ++row) 
            {
                const double xV = recording.GetValue(row, ci0);
                int x = ComputeScrCoord(xV, firstSec, lastSec, true); // horizontal
                if (x >= xp) // for the first time
                {
                    assert(xp == x || row > 0);

                    // Add stuff to selection
                    QItemSelection selection;
                    const size_t N_add_to_selection = (x == xp && row > 0)? 1 : 2;
                    for (size_t i = 0; i < N_add_to_selection; ++i) {
                        for (size_t col = 1; col < N_curves; ++col) {
                            if (m_curveVisible.at(col)) {
                                QModelIndex index = model->index((int)(row-i), (int)col);
                                selection.append(QItemSelectionRange(index, index));
                            }
                        }
                        QModelIndex index = model->index((int)(row-i), 0);
                        selection.append(QItemSelectionRange(index, index));
                    }
                    selectionModel()->select(selection, QItemSelectionModel::SelectCurrent);
                    selectionModel()->setCurrentIndex(selection.back().bottomRight(), QItemSelectionModel::Current);

                    double seconds = xV;
                    double between = 0.0;
                    if (x > xp)
                    {
                        assert(row > 0);
                        const double prev_xV = recording.GetValue(row - 1, ci0);
                        between = (row == 0) ? 0.0 // should never happen, but just in case
                            : 1.0 - double(x - xp) / (x - ComputeScrCoord(prev_xV, firstSec, lastSec, true)); // horizontal
                        assert(0.0 <= between && between < 1.0);
                        seconds = prev_xV + between*(xV - prev_xV);
                    }

                    QString info = "<b>"+ QString::number(seconds, 'f', 2) + " Sec</b>";
                    for (size_t col = 1; col < N_curves; ++col) {
                        if (m_curveVisible.at(col)) 
                        {
                            const auto& colIdx = ColumnIdx{ col };

                            double y = recording.GetValue(row, colIdx);
                            if (between > 0.0 && row > 0) {
                                const double prev_y = recording.GetValue(row-1, colIdx);
                                y = prev_y + between*(y - prev_y);
                            }
                            const QString yStr = QString::number(y, 'f', static_cast<int>(recording.SeriesPrecision(colIdx)));

                            info += "<br>";
                            QString title(recording.SeriesName(colIdx));
                            QStringRef unit;
                            int commaPos = title.indexOf(", ");
                            if (commaPos != -1) {
                                unit = QStringRef(&title, commaPos + 2, title.length()-commaPos-2);
                                info += QStringRef(&title, 0, commaPos).toString() +": "+ yStr +" "+ unit.toString();
                            } else {
                                info += title +": " + yStr;
                            }
                        }
                    }
                    QPoint p = event->globalPos();
                    QToolTip::showText(QPoint(p.x(), p.y()+10), info);

                    break;
                }
            } // end of " for (size_t row = 0; row < N_points && !done; ++row) " loop
        }
    }

    if (done) {
        update();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

//------------------------------------------------------------------------------

void RecordingPlotter::mouseReleaseEvent(QMouseEvent*)
{
    m_RubberBand.hide();
}

//------------------------------------------------------------------------------

bool RecordingPlotter::IsCurveVisible(size_t curveIdx) const
{
    bool visible = false;
    if (curveIdx < m_curveVisible.size()) {
        visible = m_curveVisible.at(curveIdx);
    } else {
        assert(!"Invalid curve index");
    }
    return visible;
}

void RecordingPlotter::SetCurveVisible(size_t curveIdx, bool visible)
{
    if (curveIdx < m_curveVisible.size()) {
        m_curveVisible.at(curveIdx) = visible;
        sCurveVisible.setDefaultInRecording(m_model->GetRecording(), curveIdx, visible);
    } else {
        assert(!"Invalid curve index");
    }
}

//static
void RecordingPlotter::ReadSettings()
{
    static bool alreadyLoaded = false;
    if (!alreadyLoaded) {
        sCurveVisible.loadSettings("curvesVisible");

        QSettings settings;
        kGraphBkgrColor  = settings.value("Graph/BkgrColor",  kGraphBkgrColor ).toString();
        kGraphGridColor  = settings.value("Graph/GridColor",  kGraphGridColor ).toString();
        kGraphFrameColor = settings.value("Graph/FrameColor", kGraphFrameColor).toString();
        sShowTitle       = settings.value("Graph/showTitle",    sShowTitle   ).toBool();
        sShowSubTitle    = settings.value("Graph/showSubtitle", sShowSubTitle).toBool();

        alreadyLoaded = true;
    }
}

//static
void RecordingPlotter::WriteSettings()
{
    sCurveVisible.saveSettings("curvesVisible");
    
    QSettings settings;
    settings.setValue("Graph/BkgrColor ", kGraphBkgrColor );
    settings.setValue("Graph/GridColor ", kGraphGridColor );
    settings.setValue("Graph/FrameColor", kGraphFrameColor);
    settings.setValue("Graph/showTitle",    sShowTitle   );
    settings.setValue("Graph/showSubtitle", sShowSubTitle);
}
