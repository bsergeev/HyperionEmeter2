#pragma once

#include <QWidget>
#include <memory>

class QAction;
class QContextMenuEvent;
class QResizeEvent;
class QSplitter;
class RecordingDataModel;
class RecordingPlotter;
class RecordingTableView;

class RecordingDisplayWnd : public QWidget // QFrame
{
public:
    enum WindowStyle {
        kDefaultWindowStyle,
        kTableOnly,
        kTableAndGraph,
        kGraphOnly
    };

    explicit RecordingDisplayWnd(std::shared_ptr<RecordingDataModel> dataModel,
                                 QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    QSize minimumSizeHint() const { return QSize( 800, 500); }
    QSize sizeHint()        const { return QSize(1000, 600); }
    
    QAction* GetGraphOptionsAction() const { return m_graphOptions_Actn; }

protected:
    virtual void resizeEvent     (QResizeEvent*      evt) override;
    virtual void contextMenuEvent(QContextMenuEvent* evt) override;

private:
    void ResizeTable();
    void SetGraphOptions();

//data:
    QSplitter* m_Splitter; // between Table & Plotter

    QAction*   m_graphOptions_Actn;
//    QAction*   m_EditTitles_Act;

    std::shared_ptr<RecordingDataModel> m_dataModel;
    std::unique_ptr<RecordingPlotter>   m_Plotter;
    std::unique_ptr<RecordingTableView> m_Table;
};
