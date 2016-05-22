#pragma once

#include <QWidget>
#include <memory>

class QSplitter;
class RecordingPlotter;
class RecordingTableView;

class RecordingDisplayWnd : public QWidget // QFrame
{
//    Q_OBJECT
public:
    enum WindowStyle {
        kDefaultWindowStyle,
        kTableOnly,
        kTableAndGraph,
        kGraphOnly
    };

    explicit RecordingDisplayWnd(std::unique_ptr<RecordingTableView> table,
                                 std::unique_ptr<RecordingPlotter>   plotter,
                                 QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

private:
    QSplitter* m_Splitter; // between Table & Plotter

    std::unique_ptr<RecordingPlotter>   m_Plotter;
    std::unique_ptr<RecordingTableView> m_Table;

};
