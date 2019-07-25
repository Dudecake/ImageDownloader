#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "image/imagefinder.h"
#include "network/danbooruworker.h"
#include "network/imageworker.h"
#include "network/konachanworker.h"
#include "network/workerfactory.h"

#include "network/danbooruworker.h"
#include "network/imageworker.h"
#include "network/konachanworker.h"
#include "network/workerfactory.h"

#include <deque>

#include <QComboBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QString>

MainWindow::MainWindow(const std::vector<std::string> &args, QWidget *parent) :
    QMainWindow(parent), depth(0), images(51), workerFactory(network::WorkerFactory::getInstance()), ui(std::make_unique<Ui::MainWindow>())
{
    QSettings settings;
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    ui->setupUi(this);
    currentIndex = 0;
    image::Image::initializeDB(getDataDir() + "/imgs.db");
    connect(ui->saveButton, &QPushButton::pressed, this, &MainWindow::handleButton, Qt::QueuedConnection);
    connect(ui->blackListButton, &QPushButton::pressed, this, &MainWindow::handleButton, Qt::QueuedConnection);
    connect(ui->resBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::handleSelect, Qt::QueuedConnection);
    connect(ui->nextButton, &QPushButton::pressed, this, &MainWindow::handleButton, Qt::QueuedConnection);
    connect(ui->skipButton, &QPushButton::pressed, this, &MainWindow::handleButton, Qt::QueuedConnection);
    connect(ui->endButton, &QPushButton::pressed, this, &MainWindow::handleButton, Qt::QueuedConnection);
    connect(ui->optionsButton, &QPushButton::pressed, this, &MainWindow::handleButton, Qt::QueuedConnection);
    connect(&dialog, &SearchDialog::accepted, this, &MainWindow::setSearchOptions);
    restoreState(settings.value("mainWindowState").toByteArray());
    if (!args.empty())
    {
        dialog.setFilterText(args.front());
        workerFactory->withFilter(args.front());
    }
    workerFactory->withCallback([&](const image::Image::image_s &image){ addToQueue(image); });
//    image::Image::redownloadAll();
//    image::ImageFinder finder(wallpaperBaseDir);
//    finder.find();
    //finder.rebuildChecksums();
    //finder.rebuildDB();
}

void MainWindow::addToQueue(const image::Image::image_s &image)
{
    getLogger()->info("Found image with a resolution of {} x {} \"{}\"", image.width, image.height, image.imageUrl);
    images.emplace_back(image);
    ui->queueLabel->setText(QString::number(static_cast<int>(images.size())) + " items in queue");
}

void MainWindow::setSearchOptions()
{
    QObject *senderObj = sender();
    if (senderObj->inherits("SearchDialog"))
    {
        SearchDialog *search = qobject_cast<SearchDialog*>(senderObj);
        SearchDialog::SearchOptions options = search->getOptions();
        workerFactory->withRatingFilter(options.getRating())->withWorkerType(options.getWorker())->withFilter(options.getTags());
        startNewWorker();
    }
}

void MainWindow::skipSingle()
{
    if (!images.empty())
    {
        currentImage = images.front();
        images.pop_front();
        worker->skip(1);
        worker->wakeAll();
        depth++;
        ui->queueLabel->setText(QString::number(images.size()) + " items in queue");
        ui->nameLabel->setText(QString::fromStdString(currentImage.getName()));
        ui->depthLabel->setText(QString::fromStdString("Depth: " + std::to_string(depth)));
        ui->frame->setPixmap(currentImage.getSample());
    }
}

void MainWindow::skipMultiple(const int &count)
{
    for (int i = 0; i < count; i++)
    {
        if (!images.empty())
        {
            currentImage = images.front();
            images.pop_front();
            worker->skip(1);
            worker->wakeAll();
            depth++;
        }
    }
    if (!currentImage.nullOrEmpty() && !images.empty())
    {
        ui->queueLabel->setText(QString::number(images.size()) + " items in queue");
        ui->nameLabel->setText(QString::fromStdString(currentImage.getName()));
        ui->depthLabel->setText(QString::fromStdString("Depth: " + std::to_string(depth)));
        ui->frame->setPixmap(currentImage.getSample());
    }
}

void MainWindow::startNewWorker()
{
    static std::function callback = [&](const image::Image::image_s &image){ addToQueue(image); };
    static int const fullHdWidth = 1920;
    static int const fullHdHeight = 1080;
    static int const wuxgaHeight = 1200;
    static int const qhdWidth = 2560;
    static int const qhdHeight = 1440;
    static int const wqxgaHeight = 1600;
    static int const uhdWidth = 3840;
    static int const uhdHeight = 2160;
    static int const wquxgaHeight = 2400;
    switch (currentIndex)
    {
        case 0:
            workerFactory->withWidth(-1);
            break;
        case 1:
            workerFactory->withWidth(fullHdWidth)->withHeight(fullHdHeight)->withMaxHeight(wuxgaHeight)->withTags(std::string());
            break;
        case 2:
            workerFactory->withWidth(qhdWidth)->withHeight(qhdHeight)->withMaxHeight(wqxgaHeight)->withTags(std::string());
            break;
        case 3:
            workerFactory->withWidth(uhdWidth)->withHeight(uhdHeight)->withMaxHeight(wquxgaHeight)->withTags(std::string());
            break;
        case 4:
            workerFactory->withWidth(fullHdHeight)->withHeight(fullHdWidth)->withMaxHeight(-1)->withTags(std::string());
            break;
        case 5:
            workerFactory->withWidth(wqxgaHeight)->withHeight(qhdWidth)->withMaxHeight(-1)->withTags(std::string());
            break;
        case 6:
            workerFactory->withWidth(-1)->withHeight(-1)->withMaxHeight(-1)->withTags("ratio%3A9%3A16+width%3A1080..");
            break;
        case 7:
            workerFactory->withWidth(-1)->withHeight(-1)->withMaxHeight(-1)->withTags("ratio%3A9%3A16+width%3A1440..");
            break;
        case 8:
            workerFactory->withWidth(-1)->withHeight(-1)->withMaxHeight(-1)->withTags("width%3A1920..+height%3A1080..");
            break;
        case 9:
            workerFactory->withWidth(-1)->withHeight(-1)->withMaxHeight(-1)->withTags("width%3A2560..+height%3A1440..");
            break;
        case 10:
            workerFactory->withWidth(-1)->withHeight(-1)->withMaxHeight(-1)->withTags("width%3A3840..+height%3A2160..");
            break;
    }
    if (worker)
    {
        worker->wakeAll();
        worker->stop();
        currentImage = image::Image();
    }
    worker = workerFactory->build();
    if (depth > 0)
    {
        getLogger()->info("Starting new search after reaching a depth of {}", depth);
    }
    depth = 0;
    images.clear();
    ui->queueLabel->setText(QString::number(images.size()) + " items in queue");
    ui->depthLabel->setText(QString::fromStdString("Depth: " + std::to_string(depth)));
}

void MainWindow::handleButton()
{
    QString senderName = sender()->objectName();
    if (senderName == "optionsButton")
    {
        if (dialog.isHidden())
        {
            dialog.show();
        }
    }
    else if (senderName == "saveButton")
    {
        if (!currentImage.nullOrEmpty())
        {
            downloadFuture = std::async(std::launch::async, [=]{
                currentImage.save();
                depth--;
                skipSingle();
            });
        }
    }
    else if (senderName == "nextButton")
    {
        downloadFuture = std::async(std::launch::async, [=]{
            skipSingle();
        });
    }
    else if (senderName == "skipButton")
    {
        downloadFuture = std::async(std::launch::async, [=]{
            skipMultiple(5);
        });
    }
    else if (senderName == "endButton")
    {
        downloadFuture = std::async(std::launch::async, [=]{
            skipMultiple(static_cast<int>(images.size()) - 1);
        });
    }
    else if (senderName == "blackListButton")
    {
        if (!currentImage.nullOrEmpty())
        {
            currentImage.blacklist();
            depth--;
        }
        downloadFuture = std::async(std::launch::async, [=]{
            skipSingle();
        });
    }
}

void MainWindow::handleSelect(const int &index)
{
    QString senderName = sender()->objectName();
    if (index > -1)
    {
        currentIndex = index;
    }
    if (senderName == ("resBox"))
    {
        startNewWorker();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    static_cast<void>(event);
    QSettings settings;
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());
}

MainWindow::~MainWindow()
{
    if (worker)
    {
        worker->wakeAll();
        worker->stop();
    }
}
