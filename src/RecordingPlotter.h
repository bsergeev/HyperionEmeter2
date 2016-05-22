#pragma once

#include <QAbstractItemView>
#include <QWidget>
#include <QImage>
#include <QColor>
#include <QScrollBar>
#include <QTimer>

#include <memory>

class QString;
class QResizeEvent;
class QPaintEvent;
class QColor;

class RecordingDataModel;

//------------------------------------------------------------------------------

class RecordingPlotter : public QAbstractItemView
{
//    Q_OBJECT
public:
    RecordingPlotter(const std::shared_ptr<RecordingDataModel>& model, 
                     QWidget* parent = nullptr);
   ~RecordingPlotter();

    QSize minimumSizeHint() const { return QSize(450, 350); }
    QSize sizeHint       () const { return QSize(600, 400); }

    // Overrides for QAbstractItemView pure virtual functions . . . . . . . . .
    virtual void scrollTo( const QModelIndex& index, ScrollHint hint=EnsureVisible ) {}
    virtual QModelIndex indexAt(const QPoint& p) const;
    virtual QRect    visualRect(const QModelIndex& index) const { return QRect(); }
 protected:
    virtual QModelIndex moveCursor( CursorAction cursorAction, Qt::KeyboardModifiers modifiers ) {return QModelIndex();}
    virtual int horizontalOffset() const { return horizontalScrollBar()->value(); }
    virtual int verticalOffset  () const { return verticalScrollBar  ()->value(); }
    virtual bool isIndexHidden(const QModelIndex &) const { return false; }
    virtual void setSelection( const QRect & rect, QItemSelectionModel::SelectionFlags flags ){}
    virtual QRegion visualRegionForSelection ( const QItemSelection & selection ) const {return QRegion();}
    // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

protected:
    //void paintEvent       ( QPaintEvent*  event );
    //void resizeEvent      ( QResizeEvent* event );
    //void mousePressEvent  ( QMouseEvent*  event );
    //void mouseMoveEvent   ( QMouseEvent*  event );
    //void mouseReleaseEvent( QMouseEvent*  event );
    //void keyPressEvent    ( QKeyEvent*    event );
    //void wheelEvent       ( QWheelEvent*  event );
    //void mouseDoubleClickEvent( QMouseEvent* e );

private:
    std::shared_ptr<RecordingDataModel> m_model;

    QWidget* m_ParentWnd;
    //MainWnd* m_MainWnd;

public: // statics

    static const short GAP_X; // = 4;
    static const int NO_STEPS_PROCESSED; // = -1;

    static bool    kShowTooltip;
    static bool    kShowTitle;
    static bool    kShowSubTitle;
};

//------------------------------------------------------------------------------
