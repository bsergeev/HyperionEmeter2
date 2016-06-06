#pragma once

#include <QAbstractItemView>
#include <QWidget>
#include <QImage>
#include <QString>
#include <QColor>
#include <QScrollBar>
#include <QTimer>

#include <array>
#include <memory>
#include <vector>

class QString;
class QResizeEvent;
class QPaintEvent;
class QColor;

class RecordingDataModel;

//------------------------------------------------------------------------------

class RecordingPlotter : public QAbstractItemView
{
//    Q_OBJECT
    enum eMarginIdx {
        eLEFT = 0,
        eRIGHT,
        eUP,
        eDOWN,
        eNMARGINS // should be last
    };

public:
    RecordingPlotter(const std::shared_ptr<RecordingDataModel>& model, 
                     QWidget* parent = nullptr);
   ~RecordingPlotter();

    QSize minimumSizeHint() const { return QSize(600, 400); }
    QSize sizeHint       () const { return QSize(700, 500); }

	bool IsCurveVisible (size_t curveIdx) const;
	void SetCurveVisible(size_t curveIdx, bool visible);

    void   AdjustScrMargins();
    void   ComputeTicks(int minTickNumber = -1);

    // Overrides for QAbstractItemView pure virtual functions . . . . . . . . .
    virtual void scrollTo(const QModelIndex&, ScrollHint = EnsureVisible) override {}
    virtual QModelIndex indexAt(const QPoint& p) const override;
    virtual QRect visualRect(const QModelIndex&) const override { return QRect(); }
 protected:
    virtual QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers) override {return QModelIndex();}
    virtual int  horizontalOffset() const override { return horizontalScrollBar()->value(); }
    virtual int  verticalOffset  () const override { return verticalScrollBar  ()->value(); }
    virtual bool isIndexHidden(const QModelIndex &) const override { return false; }
    virtual void setSelection(const QRect&, QItemSelectionModel::SelectionFlags) override {}
    virtual QRegion visualRegionForSelection(const QItemSelection&) const override { return QRegion(); }
    // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

protected:
    void paintEvent       (QPaintEvent*  evt) override;
    //void resizeEvent      (QResizeEvent* evt) override;
    //void mousePressEvent  (QMouseEvent*  evt) override;
    //void mouseMoveEvent   (QMouseEvent*  evt) override;
    //void mouseReleaseEvent(QMouseEvent*  evt) override;
    //void keyPressEvent    (QKeyEvent*    evt) override;
    //void wheelEvent       (QWheelEvent*  evt) override;
    //void mouseDoubleClickEvent(QMouseEvent* ) override;

private:
    size_t GetNRightAxes() const;
    int    ComputeScrCoord(double v, double minV, double maxV, bool horizontal) const;

//data:
    std::shared_ptr<RecordingDataModel> m_model;

    std::vector<std::vector<double>> m_tickV; // indexed by column, i.e. only for existing curves
    std::vector<bool>         m_curveVisible; // indexed by column, i.e. only for existing curves

    std::array<int, eNMARGINS> m_margin;
    QFont                      m_Font;

    bool m_showTooltip  = true;
    bool m_showTitle    = true;
    bool m_showSubTitle = true;

public: // statics
    static QString SecondsTxt(double sec);
};

//------------------------------------------------------------------------------
