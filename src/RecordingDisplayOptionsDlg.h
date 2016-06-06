#pragma once

#include <QDialog>
#include <QVector>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>

#include <vector>

class QLabel;
class QWidget;

class IPlotSettingsMgr;
class ColorButton;
class RecordingDataModel;
class RecordingPlotter;
class RecordingTableView;

class RecordingDisplayOptionsDlg : public QDialog
{
 Q_OBJECT

public:
    RecordingDisplayOptionsDlg( bool showTable, bool showGraph
                      , const RecordingDataModel* dataModel // IPlotSettingsMgr const& psMgr
                      , const RecordingPlotter*   plotter
                      , const RecordingTableView* table
                      , int  xAxisType
                      , bool imperialUnits   = false
                      , bool visibleTitle    = true
                      , bool visibleSubTitle = true
                      , QWidget* parent = nullptr );
   ~RecordingDisplayOptionsDlg() {}

    QColor GetBackgroundColor() const;
    QColor GetFrameColor     () const;
    QColor GetGridlineColor  () const;
    QColor GetCurveColor (int curveIdx) const;
    bool GetCurveVisible (int curveIdx) const;
    bool GetColumnVisible(int curveIdx) const;
    bool GetShowTitle()      const { return (m_VisibleTitle)?      m_VisibleTitle->     isChecked() : false; }
    bool GetShowSubTitle()   const { return (m_VisibleSubTitle)?   m_VisibleSubTitle->  isChecked() : false; }
    bool GetShowLegend()     const { return (m_LegendChkBx )?      m_LegendChkBx->      isChecked() : false; }
    bool GetClrColumns()     const { return (m_ColorColumns)?      m_ColorColumns->     isChecked() : false; }
    bool GetShowTooltip()    const { return (m_ShowTooltip )?      m_ShowTooltip->      isChecked() : false; }
    bool GetIsBritishUnits() const { return (m_imperialUnitsRBtn)? m_imperialUnitsRBtn->isChecked() : false; }
    int  GetXAxisType()      const;

    bool IsTableEnabled()    const { return (m_TableGrpBox)? m_TableGrpBox->isChecked() : false; } 
    bool IsPlotEnabled ()    const { return (m_PlotGrpBox )? m_PlotGrpBox-> isChecked() : false; }

private slots:
    //void ToggledTimeRBtn  (bool);
    //void ToggledChargeRBtn(bool);
    //void ToggledTimeChargeRBtn(bool);
    //void ToggledChargeTimeRBtn(bool);
    void ToggledImperialUnits(bool britishUnits);

private:
    void ShowCrvGroup(int crvIdx, bool enable, bool* checkedPtr = nullptr);

    QCheckBox*    m_VisibleTitle     = nullptr;
    QCheckBox*    m_VisibleSubTitle  = nullptr;

    QGroupBox*    m_TableGrpBox      = nullptr;
    QVector<QCheckBox*> m_ColumnVisibleChBxs;
    QCheckBox*    m_ColorColumns     = nullptr;

    QGroupBox*    m_PlotGrpBox       = nullptr;
    QVector<ColorButton*> m_ColorBtns;
    QVector<QCheckBox*> m_CurveVisibleChBxs;
    QCheckBox*    m_LegendChkBx      = nullptr;
    QCheckBox*    m_ShowTooltip      = nullptr;
    QGroupBox*    m_XAxisTypeGrpBox  = nullptr;
    QRadioButton* m_Xaxis_Time       = nullptr;
    QRadioButton* m_Xaxis_Charge     = nullptr;
    QRadioButton* m_Xaxis_TimeCharge = nullptr;
    QRadioButton* m_Xaxis_ChargeTime = nullptr;
    ColorButton*  m_BkgrColorBtn     = nullptr;
    ColorButton*  m_FrameColorBtn    = nullptr;
    ColorButton*  m_GridlineColorBtn = nullptr;

    QGroupBox*    m_UnitsGrpBox      = nullptr;
    QRadioButton* m_imperialUnitsRBtn= nullptr;
    QLabel*       m_UnitsExampleTxt  = nullptr;

    bool m_ChargeWasVisible = false;
};
