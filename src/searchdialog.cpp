#include "searchdialog.h"
#include "ui_searchdialog.h"

#include "image/image.h"

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent), ui(std::make_unique<Ui::SearchDialog>()), workerIndex(0), ratingIndex(2)
{
    ui->setupUi(this);
    connect(ui->ratingBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SearchDialog::setRatingIndex);
    connect(ui->siteBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SearchDialog::setWorkerIndex);
}

void SearchDialog::setRatingIndex(const uint8_t &index)
{
    ratingIndex = index;
}

void SearchDialog::setWorkerIndex(const uint8_t &index)
{
    workerIndex = index;
}

SearchDialog::SearchOptions SearchDialog::getOptions()
{
    image::Image::Rating rating = image::Image::Rating::All;
    network::ImageWorker::WorkerType worker = network::ImageWorker::WorkerType { workerIndex };
    switch (ratingIndex)
    {
        case 0:
            break;
        case 1:
            rating = image::Image::Rating::Safe;
            break;
        case 2:
            rating = image::Image::Rating::Safe_Questionable;
            break;
        case 3:
            rating = image::Image::Rating::Questionable;
            break;
        case 4:
            rating = image::Image::Rating::Questionable_Explicit;
            break;
        case 5:
            rating = image::Image::Rating::Explicit;
            break;
    }
    return SearchOptions(ui->tagField->toPlainText().toStdString(), rating, worker);
}

void SearchDialog::setFilterText(const std::string &filterText)
{
    ui->tagField->setPlainText(QString::fromStdString(filterText));
}

SearchDialog::~SearchDialog() = default;
