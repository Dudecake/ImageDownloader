#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QPlainTextEdit>
#include <memory>

#include "searchoptions.h"

namespace Ui {
    class SearchDialog;
}

class SearchDialog : public QDialog {
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = nullptr);
    ~SearchDialog();
    SearchOptions getOptions();
    void setFilterText(const std::string &filterText);

private:
    std::unique_ptr<Ui::SearchDialog> ui;
    uint8_t ratingIndex;
    uint8_t workerIndex;
private slots:
    void setRatingIndex(const uint8_t &index);
    void setWorkerIndex(const uint8_t &index);
};

#endif // SEARCHDIALOG_H
