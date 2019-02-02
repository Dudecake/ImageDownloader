#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDebug>
#include <QDialog>
#include <QPlainTextEdit>
#include <QComboBox>

#include "image/image.h"
#include "network/imageworker.h"

namespace Ui {
    class SearchDialog;
}

class SearchDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit SearchDialog(QWidget *parent = nullptr);
        ~SearchDialog();
        class SearchOptions
        {
            private:
                std::string tags;
                image::Image::Rating rating;
                network::ImageWorker::WorkerType worker;
            public:
                explicit SearchOptions(const std::string &tags, const image::Image::Rating &rating, const network::ImageWorker::WorkerType &worker)
                {
                    this->tags = tags;
                    this->rating = rating;
                    this->worker = worker;
                }
                std::string getTags()
                {
                    if (tags.find_first_of(' ') != std::string::npos)
                        std::replace(tags.begin(), tags.end(), ' ', '+');
                    return tags;
                }
                image::Image::Rating getRating() const { return rating; }
                network::ImageWorker::WorkerType getWorker() const { return worker; }
        };
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
