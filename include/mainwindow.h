#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <future>
#include <memory>
#include <deque>

#include "image/image.h"
#include "logger.h"
#include "searchdialog.h"
#include <deque>

namespace Ui {
    class MainWindow;
}

namespace network {
    class ImageWorker;
    class WorkerFactory;
} // namespace network
class SearchDialog;
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    void closeEvent(QCloseEvent *event) override;
    explicit MainWindow(const std::vector<std::string> &args, QWidget *parent = nullptr);
    MainWindow(const MainWindow &) = delete;
    MainWindow(MainWindow &&) = delete;
    MainWindow operator=(const MainWindow &) = delete;
    MainWindow operator=(const MainWindow &&) = delete;
    ~MainWindow() override;

private:
    static auto &getLogger()
    {
        static auto logger = logger::getSpdLogger("MainWindow");
        return logger;
    }
    int currentIndex;
    int depth;
    SearchDialog dialog;
    image::Image currentImage;
    std::unique_ptr<network::ImageWorker> worker;
    std::shared_ptr<network::WorkerFactory> workerFactory;
    std::unique_ptr<Ui::MainWindow> ui;
    std::deque<image::Image> images;
    std::future<void> downloadFuture;
    std::function<void(const image::Image::image_s &)> const callback;
    void skipSingle();
    void skipMultiple(const int &count = 5);
    void startNewWorker();
    void addToQueue(const image::Image::image_s &image);

private slots:
    void handleButton();
    void handleSelect(const int &);
    void setSearchOptions();
};

#endif // MAINWINDOW_H
