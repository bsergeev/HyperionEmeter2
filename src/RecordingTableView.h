#pragma once

#include <QTableView>
#include <memory>
#include <vector>

class RecordingDataModel;

class RecordingTableView : public QTableView {
public:
      RecordingTableView(const std::shared_ptr<RecordingDataModel>& model);

      int  GetTotalWidth() const;

	  bool IsColumnVisible (size_t columnIdx) const;
	  void SetColumnVisible(size_t columnIdx, bool visible);

private:
    std::shared_ptr<RecordingDataModel> m_model;
	std::vector<bool> m_columnVisible; // indexed by column, i.e. only for existing curves
};
