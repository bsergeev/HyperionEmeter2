#pragma once

#include "DefaultValues.h"

#include <QTableView>

#include <memory>
#include <vector>

class RecordingDataModel;

class RecordingTableView : public QTableView {
public:
      RecordingTableView(const std::shared_ptr<RecordingDataModel>& model, QWidget* parent = nullptr);
      ~RecordingTableView();

      int  GetTotalWidth() const;

      bool IsColumnVisible (size_t columnIdx) const;
      void SetColumnVisible(size_t columnIdx, bool visible);

private:
    void UpdateColumnVisibility();

    std::shared_ptr<RecordingDataModel> m_model;
    std::vector<bool> m_columnVisible; // indexed by column, i.e. only for existing curves

private: // static
    static void ReadSettings();
    static void WriteSettings();

    static DefaultBools sColumnVisible;
public:
    static bool         sColorColumns;
};
