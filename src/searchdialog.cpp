#include "searchdialog.h"
#include "ui_searchdialog.h"

#include "image/image.h"
#include "network/imageworker.h"
#include <array>

SearchDialog::SearchDialog(QWidget *parent) : QDialog(parent), ui(std::make_unique<Ui::SearchDialog>()), ratingIndex(2), workerIndex(0)
{
    ui->setupUi(this);
    connect(ui->ratingBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SearchDialog::setRatingIndex);
    connect(ui->siteBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SearchDialog::setWorkerIndex);
}

void SearchDialog::setRatingIndex(const uint8_t &index)
{
    ratingIndex = index;
}

void SearchDialog::setWorkerIndex(const uint8_t &index)
{
    workerIndex = index;
}

SearchOptions SearchDialog::getOptions()
{
    static constexpr std::array ratings = {
        image::Image::Rating::All,
        image::Image::Rating::Safe,
        image::Image::Rating::Safe_Questionable,
        image::Image::Rating::Questionable,
        image::Image::Rating::Questionable_Explicit,
        image::Image::Rating::Explicit
    };
    network::ImageWorker::WorkerType worker = network::ImageWorker::WorkerType{workerIndex};
    return SearchOptions(ui->tagField->toPlainText().toStdString(), ratings.at(ratingIndex), worker);
}

void SearchDialog::setFilterText(const std::string &filterText)
{
    ui->tagField->setPlainText(QString::fromStdString(filterText));
}

SearchDialog::~SearchDialog() = default;
