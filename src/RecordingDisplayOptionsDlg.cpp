#include "RecordingDisplayOptionsDlg.h"
#include "MainWnd.h" // for static colors only
#include "CustomWidgets/ColorButton.h"
#include "RecordingDataModel.h"
#include"RecordingPlotter.h"
#include"RecordingTableView.h"
#include "Recording.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QGroupBox>

//------------------------------------------------------------------------------

RecordingDisplayOptionsDlg::RecordingDisplayOptionsDlg( bool showTable, bool showGraph
                                                      , const RecordingDataModel*   dataModel // IPlotSettingsMgr const& psMgr
                                                      , const RecordingPlotter*     plotter
                                                      , const RecordingTableView*   table
                                                      , int  // xAxisType
                                                      , bool imperialUnits
                                                      , bool visibleTitle
                                                      , bool visibleSubTitle
                                                      , QWidget* parent )
    : QDialog( parent )
    , m_ChargeWasVisible(true) // gets set if X-type is shown
{
    assert(dataModel != nullptr);

    const Recording& recording = dataModel->GetRecording();
    const size_t N_curves = recording.numColums();

    //Q_ASSERT( N_curves > 0 && N_curves == curveNames.size() );
    m_ColumnVisibleChBxs.resize(N_curves);
    m_ColorBtns         .resize(N_curves);
    m_CurveVisibleChBxs .resize(N_curves);


    setWindowTitle( tr("Display options") );

    QVBoxLayout* layout = new QVBoxLayout;

        m_VisibleTitle = new QCheckBox( tr("Show &title"), this );
        m_VisibleTitle->setChecked( visibleTitle );
        layout->addWidget(m_VisibleTitle);

        m_VisibleSubTitle = new QCheckBox( tr("Show &subtitle"), this );
        m_VisibleSubTitle->setChecked( visibleSubTitle );
        layout->addWidget(m_VisibleSubTitle);

        //m_LegendChkBx = new QCheckBox( tr("Show &legend"), this );
        //m_LegendChkBx->setChecked( ChannelRecordingChildWnd::kShowLegend );
        //layout->addWidget(m_LegendChkBx);

        QHBoxLayout* layTableGraph2columns = new QHBoxLayout;

        m_TableGrpBox = new QGroupBox(tr("Data table"), this);
        m_TableGrpBox->setCheckable( true );
        m_TableGrpBox->setChecked( showTable );

        QVBoxLayout* layTable = new QVBoxLayout;

            QGroupBox* columnGrpBox = new QGroupBox(tr("Columns"), this);
            QVBoxLayout* layColumnVisibility = new QVBoxLayout;

                for (size_t i=1;  i < N_curves;  ++i)
                {
                    QCheckBox* chBx = new QCheckBox(recording.SeriesName(ColumnIdx(i)), this );
                               chBx->setChecked(table->IsColumnVisible(i));
                    m_ColumnVisibleChBxs[i] = chBx;
                  
                    layColumnVisibility->addWidget(chBx);
                }

            columnGrpBox->setLayout(layColumnVisibility);

            layTable->addWidget(columnGrpBox);

            m_ColorColumns = new QCheckBox( tr("Color columns as curves"), this );
            m_ColorColumns->setChecked(RecordingTableView::sColorColumns);
            layTable->addWidget(m_ColorColumns);

        m_TableGrpBox->setLayout(layTable);

        layTableGraph2columns->addWidget(m_TableGrpBox);
        layTableGraph2columns->addStretch(1);


        m_PlotGrpBox = new QGroupBox(tr("Data plot"), this);
        m_PlotGrpBox->setCheckable( true );
        m_PlotGrpBox->setChecked(showGraph);

        QVBoxLayout* layPlotV= new QVBoxLayout;

        QHBoxLayout* layPlot = new QHBoxLayout;
            QGroupBox* curveGrpBox = new QGroupBox(tr("Curves"), this);
            QGridLayout* grLay = new QGridLayout;

                for (size_t i=1;  i < N_curves;  ++i)
                {
                    const bool visible = true; // <<< DEBUG TBD  !psMgr.isXAxis(i) && mayExist[i];
                    ColorButton* btn  = new ColorButton( dataModel->GetColumnColor(i), this );
                    btn->setVisible(visible);
                    m_ColorBtns.at(i) = btn;
                    grLay->addWidget(btn, static_cast<int>(i), 0);

                    QCheckBox* chBx = new QCheckBox(recording.SeriesName(ColumnIdx(i)), this );
                               chBx->setChecked(plotter->IsCurveVisible(i) );
                               chBx->setVisible(visible);
                    m_CurveVisibleChBxs.at(i) = chBx;
                    grLay->addWidget(chBx, static_cast<int>(i), 1, Qt::AlignLeft);
                }

            curveGrpBox->setLayout(grLay);
        layPlot->addWidget(curveGrpBox);

        m_ChargeWasVisible = true; // <<< DEBUG TBD  psMgr.IsCurveVisible( kIdxCharge );

        QVBoxLayout* laySecondColumn = new QVBoxLayout;

        QGroupBox* XaxisGrpBox = new QGroupBox(tr("X-Axis"), this);
        XaxisGrpBox->setEnabled(false); // <<< DEBUG TBD  
        QVBoxLayout* lay01 = new QVBoxLayout(XaxisGrpBox);

            m_Xaxis_Time = new QRadioButton( tr("Time only"), this );
            //connect( m_Xaxis_Time, SIGNAL(toggled(bool)), this, SLOT(ToggledTimeRBtn(bool)) );
            lay01->addWidget(m_Xaxis_Time);

            m_Xaxis_Charge = new QRadioButton( tr("Charge only"), this );
            //connect( m_Xaxis_Charge, SIGNAL(toggled(bool)), this, SLOT(ToggledChargeRBtn(bool)) );
            lay01->addWidget(m_Xaxis_Charge);

            m_Xaxis_TimeCharge = new QRadioButton( tr("Uniform time and measured charge"), this );
            //connect( m_Xaxis_TimeCharge, SIGNAL(toggled(bool)), this, SLOT(ToggledTimeChargeRBtn(bool)) );
            lay01->addWidget(m_Xaxis_TimeCharge);

            m_Xaxis_ChargeTime = new QRadioButton( tr("Uniform charge and measurement time"), this );
            //connect( m_Xaxis_ChargeTime, SIGNAL(toggled(bool)), this, SLOT(ToggledChargeTimeRBtn(bool)) );
            lay01->addWidget(m_Xaxis_ChargeTime);

        laySecondColumn->addWidget(XaxisGrpBox);

        QGroupBox* colorsGrpBox = new QGroupBox(tr("Plot area colors"), this);
        lay01 = new QVBoxLayout(colorsGrpBox);

            QHBoxLayout* hLay = new QHBoxLayout();
            m_BkgrColorBtn = new ColorButton( RecordingPlotter::kGraphBkgrColor, this);
            hLay->addWidget(m_BkgrColorBtn);

            QLabel* txt = new QLabel( tr("Background color"), this);
            hLay->addWidget(txt);

        lay01->addLayout( hLay );

            hLay = new QHBoxLayout();
            m_FrameColorBtn = new ColorButton(RecordingPlotter::kGraphFrameColor, this);
            hLay->addWidget(m_FrameColorBtn);

            txt = new QLabel( tr("Frame color"), this);
            hLay->addWidget(txt);

        lay01->addLayout( hLay );

            hLay = new QHBoxLayout();
            m_GridlineColorBtn = new ColorButton(RecordingPlotter::kGraphGridColor, this);
            hLay->addWidget(m_GridlineColorBtn);

            txt = new QLabel( tr("Gridline color"), this);
            hLay->addWidget(txt);

        lay01->addLayout( hLay );

        laySecondColumn->addWidget(colorsGrpBox);
        laySecondColumn->addStretch(1);


        layPlot->addLayout(laySecondColumn);

        layPlotV->addLayout(layPlot);

        m_ShowTooltip = new QCheckBox( tr("When mouse over plot, show nearest point value in the tool tip"), this );
        m_ShowTooltip->setChecked(true); // <<< DEBUG TBD  MeasurementRecordingPlotter::kShowTooltip );
        layPlotV->addWidget(m_ShowTooltip);

        m_PlotGrpBox->setLayout(layPlotV);
        layTableGraph2columns->addWidget(m_PlotGrpBox);

    layout->addLayout(layTableGraph2columns);

    // Units .................................................................
    QVBoxLayout* unitsVLayout = new QVBoxLayout;
    m_UnitsGrpBox = new QGroupBox(tr("Unit system"), this);
    m_UnitsGrpBox->setEnabled(false); // <<< DEBUG TMP 

    m_imperialUnitsRBtn  = new QRadioButton("&Imperial", this);
    connect(m_imperialUnitsRBtn, SIGNAL(toggled(bool)), this, SLOT(ToggledImperialUnits(bool)));
    unitsVLayout->addWidget(m_imperialUnitsRBtn);

    QRadioButton* SIUnitsRBtn = new QRadioButton("&Metric", this);
    unitsVLayout->addWidget(SIUnitsRBtn);

    m_UnitsExampleTxt = new QLabel(this);
    unitsVLayout->addWidget(m_UnitsExampleTxt);

    m_UnitsGrpBox->setLayout(unitsVLayout);
    layout->addWidget(m_UnitsGrpBox);


    m_imperialUnitsRBtn->setChecked( imperialUnits );
    SIUnitsRBtn       ->setChecked(!imperialUnits );

    ToggledImperialUnits( imperialUnits );
    //..........................................................................


    QHBoxLayout* lay2 = new QHBoxLayout;

    QPushButton* pb = new QPushButton( "Ok", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(accept()) );
    pb->setDefault (true);
    lay2->addWidget( pb );
    lay2->addStretch(1);

    pb = new QPushButton( "Cancel", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(reject()) );
    lay2->addWidget( pb );

    layout->addLayout(lay2);

    setLayout(layout);


    // Set X-axis type (it's done now to invoke slots)
    m_Xaxis_Time      ->setChecked(true);  // <<< DEBUG TBD   xAxisType == kXAxis_Time );
    m_Xaxis_Charge    ->setChecked(false); // <<< DEBUG TBD   xAxisType == kXAxis_Charge );
    m_Xaxis_TimeCharge->setChecked(false); // <<< DEBUG TBD   xAxisType == kXAxis_Time_slaveCharge );
    m_Xaxis_ChargeTime->setChecked(false); // <<< DEBUG TBD   xAxisType == kXAxis_Charge_slaveTime );

    // m_UnitsGrpBox->setEnabled(unitsMatter);
}

//------------------------------------------------------------------------------

void RecordingDisplayOptionsDlg::ShowCrvGroup(int crvIdx, bool show, bool* checkedPtr)
{
    if (0 <= crvIdx  &&  crvIdx < m_CurveVisibleChBxs.size())
    {
        if (QCheckBox* chBx = m_CurveVisibleChBxs[crvIdx]) {
            if (checkedPtr != nullptr) {
                chBx->setChecked(*checkedPtr);
            }
            chBx->setVisible(show);
        }
        if (ColorButton* cBtn = m_ColorBtns.at(crvIdx)) {
            cBtn->setVisible(show);
        }
    }
    else
    {
        Q_ASSERT_X( false, "RecordingDisplayOptionsDlg::ShowCrvGroup()", "Wrong crvIdx" );
    }
}

//------------------------------------------------------------------------------
/*
void RecordingDisplayOptionsDlg::ToggledTimeRBtn(bool checked)
{
    if (!checked)
        return; // only do something if it's checked

    ShowCrvGroup( kIdxTime,   false );
    ShowCrvGroup( kIdxCharge, true, &m_ChargeWasVisible );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void RecordingDisplayOptionsDlg::ToggledChargeRBtn(bool checked)
{
    if (!checked)
        return; // only do something if it's checked

    ShowCrvGroup( kIdxTime,   true );
    ShowCrvGroup( kIdxCharge, false, &m_ChargeWasVisible );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void RecordingDisplayOptionsDlg::ToggledTimeChargeRBtn(bool checked)
{
    if (checked) {
        ShowCrvGroup( kIdxTime,   false );
        ShowCrvGroup( kIdxCharge, false, &m_ChargeWasVisible );
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void RecordingDisplayOptionsDlg::ToggledChargeTimeRBtn(bool checked)
{
    if (checked) {
        ShowCrvGroup( kIdxTime,   false );
        ShowCrvGroup( kIdxCharge, false, &m_ChargeWasVisible );
    }
}
*/
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void RecordingDisplayOptionsDlg::ToggledImperialUnits(bool checked)
{
    m_UnitsExampleTxt->setText((checked)? tr("Thrust = oz, Temperature = F, Altitude = feet")
                                        : tr("Thrust = g, Temperature = C, Altitude = m"));
}

//------------------------------------------------------------------------------

QColor RecordingDisplayOptionsDlg::GetCurveColor(size_t curveIdx) const
{
    if (0 <= curveIdx  &&  curveIdx < m_ColorBtns.size()
     && m_ColorBtns[curveIdx] != nullptr) {
        return m_ColorBtns[curveIdx]->GetColor();
    }
    return QColor();
}

//------------------------------------------------------------------------------

QColor RecordingDisplayOptionsDlg::GetBackgroundColor() const
{
    return ((m_BkgrColorBtn != nullptr)? m_BkgrColorBtn->GetColor() 
                                       : RecordingPlotter::kGraphBkgrColor);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

QColor RecordingDisplayOptionsDlg::GetFrameColor() const
{
    return ((m_FrameColorBtn != nullptr)? m_FrameColorBtn->GetColor() 
                                        : RecordingPlotter::kGraphFrameColor);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

QColor RecordingDisplayOptionsDlg::GetGridlineColor() const
{
    return ((m_GridlineColorBtn != nullptr)? m_GridlineColorBtn->GetColor() 
                                           : RecordingPlotter::kGraphGridColor);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


//------------------------------------------------------------------------------

bool RecordingDisplayOptionsDlg::GetCurveVisible(size_t curveIdx) const
{
    if (0 <= curveIdx  &&  curveIdx < m_CurveVisibleChBxs.size()
     && m_CurveVisibleChBxs[curveIdx] != nullptr) {
        return m_CurveVisibleChBxs[curveIdx]->isChecked();
    }
    return false;
}

//------------------------------------------------------------------------------

bool RecordingDisplayOptionsDlg::GetColumnVisible(size_t curveIdx) const
{
    if (0 <= curveIdx  &&  curveIdx < m_ColumnVisibleChBxs.size()
     && m_ColumnVisibleChBxs[curveIdx] != nullptr)
    {
        return m_ColumnVisibleChBxs[curveIdx]->isChecked();
    }
    return false;
}

//------------------------------------------------------------------------------

int RecordingDisplayOptionsDlg::GetXAxisType() const
{
    //XAxisType xAxisType = kXAxis_NotSet;
    //if (m_Xaxis_Time  &&  m_Xaxis_Time->isChecked())
    //    xAxisType = kXAxis_Time;
    //else if(m_Xaxis_Charge  &&  m_Xaxis_Charge->isChecked())
    //    xAxisType = kXAxis_Charge;
    //else if(m_Xaxis_TimeCharge  &&  m_Xaxis_TimeCharge->isChecked())
    //    xAxisType = kXAxis_Time_slaveCharge;
    //else if(m_Xaxis_ChargeTime  &&  m_Xaxis_ChargeTime->isChecked())
    //    xAxisType = kXAxis_Charge_slaveTime;
    //
    //return xAxisType;
    return 0;
}

//------------------------------------------------------------------------------
