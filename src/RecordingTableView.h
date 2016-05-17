#pragma once

#include <QTableView>
#include <memory>

class RecordingTableView : public QTableView {
     Q_OBJECT

public:
      RecordingTableView(QAbstractItemModel* model);

protected:
      virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
      virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) Q_DECL_OVERRIDE;
      void scrollTo (const QModelIndex & index, ScrollHint hint = EnsureVisible) Q_DECL_OVERRIDE;

private slots:
      void updateSectionWidth(int logicalIndex, int oldSize, int newSize);
      void updateSectionHeight(int logicalIndex, int oldSize, int newSize);

private:
      void init();
      void updateFrozenTableGeometry();

//data:
      std::unique_ptr<QAbstractItemModel> m_model;
      std::unique_ptr<QTableView>     m_tableView;
};
