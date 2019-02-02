#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <log4cxx/logger.h>

#include "image/image.h"
#include "searchdialog.h"

namespace Ui
{
class MainWindow;
}

namespace image
{
    class Image;
}
namespace network
{
    class ImageWorker;
    class WorkerFactory;
}
class SearchDialog;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    std::deque<image::Image> images;
    void closeEvent(QCloseEvent*);
    explicit MainWindow(const std::vector<std::string> &args, QWidget *parent = nullptr);
    ~MainWindow();

    private:
        static log4cxx::LoggerPtr logger;
        int currentIndex;
        int depth;
        SearchDialog dialog;
        image::Image currentImage;
        std::unique_ptr<network::ImageWorker> worker;
        std::shared_ptr<network::WorkerFactory> workerFactory;
        std::unique_ptr<Ui::MainWindow> ui;
        void skipSingle();
        void skipMultiple(const int &count = 5);
        void startNewWorker();
        void addToQueue(const image::Image::image_s &);

private slots:
    void handleButton();
    void handleSelect(const int &);
    void setSearchOptions();
};

#endif // MAINWINDOW_H
