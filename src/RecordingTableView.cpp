#include "RecordingTableView.h"
#include "RecordingDataModel.h"

#include <QScrollBar>
#include <QHeaderView>

RecordingTableView::RecordingTableView(QAbstractItemModel * model)
    : m_model(model)
    , m_tableView(new QTableView(this))
{
      setModel(model);

      init();

      //connect the headers and scrollbars of both tableviews together
      connect(horizontalHeader(),&QHeaderView::sectionResized, this, &RecordingTableView::updateSectionWidth);
      connect(verticalHeader(),  &QHeaderView::sectionResized, this, &RecordingTableView::updateSectionHeight);

      connect(m_tableView->verticalScrollBar(), &QAbstractSlider::valueChanged, verticalScrollBar(), &QAbstractSlider::setValue);
      connect(verticalScrollBar(), &QAbstractSlider::valueChanged, m_tableView->verticalScrollBar(), &QAbstractSlider::setValue);

      setAttribute(Qt::WA_DeleteOnClose);
}

void RecordingTableView::init()
{
      m_tableView->setModel(model());
      m_tableView->setFocusPolicy(Qt::NoFocus);
      m_tableView->verticalHeader()->hide();
      m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

      viewport()->stackUnder(m_tableView.get());

      //m_tableView->setStyleSheet("QTableView { border: none;"
      //                           "background-color: #8EDE21;"
      //                           "selection-background-color: #999}"); //for demo purposes
      m_tableView->setStyleSheet("QTableView { border: none;"
                                 "background-color: #CCC;"
                                 "selection-background-color: #999}"); //for demo purposes
      m_tableView->setSelectionModel(selectionModel());
      for (int col = 1; col < model()->columnCount(); ++col)
            m_tableView->setColumnHidden(col, true);

      m_tableView->setColumnWidth(0, columnWidth(0) );

      m_tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      m_tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      m_tableView->show();

      updateFrozenTableGeometry();

      setHorizontalScrollMode(ScrollPerPixel);
      setVerticalScrollMode(ScrollPerPixel);
      m_tableView->setVerticalScrollMode(ScrollPerPixel);
}

void RecordingTableView::updateSectionWidth(int logicalIndex, int /* oldSize */, int newSize)
{
      if (logicalIndex == 0){
            m_tableView->setColumnWidth(0, newSize);
            updateFrozenTableGeometry();
      }
}

void RecordingTableView::updateSectionHeight(int logicalIndex, int /* oldSize */, int newSize)
{
      m_tableView->setRowHeight(logicalIndex, newSize);
}

void RecordingTableView::resizeEvent(QResizeEvent * event)
{
      QTableView::resizeEvent(event);
      updateFrozenTableGeometry();
 }

QModelIndex RecordingTableView::moveCursor(CursorAction cursorAction,
                                          Qt::KeyboardModifiers modifiers)
{
      QModelIndex current = QTableView::moveCursor(cursorAction, modifiers);

      if (cursorAction == MoveLeft && current.column() > 0
              && visualRect(current).topLeft().x() < m_tableView->columnWidth(0) ){
            const int newValue = horizontalScrollBar()->value() + visualRect(current).topLeft().x()
                                 - m_tableView->columnWidth(0);
            horizontalScrollBar()->setValue(newValue);
      }
      return current;
}

void RecordingTableView::scrollTo (const QModelIndex & index, ScrollHint hint){
    if (index.column() > 0)
        QTableView::scrollTo(index, hint);
}

void RecordingTableView::updateFrozenTableGeometry()
{
      m_tableView->setGeometry(verticalHeader()->width() + frameWidth(),
                                   frameWidth(), columnWidth(0),
                                   viewport()->height()+horizontalHeader()->height());
}
