#pragma once

#include <QTableView>
#include <memory>

class RecordingDataModel;

class RecordingTableView : public QTableView {
public:
      RecordingTableView(const std::shared_ptr<RecordingDataModel>& model);
private:
    std::shared_ptr<RecordingDataModel> m_model;
};
