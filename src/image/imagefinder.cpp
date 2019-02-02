#include "image/imagefinder.h"
#include "image/imageinfo.h"
#include "image/fraction.h"
#include "network/downloadhelper.h"

#include <filesystem>

#include <QRegularExpression>

log4cxx::LoggerPtr image::ImageFinder::logger = log4cxx::Logger::getLogger("ImageFinder");

image::ImageFinder::ImageFinder(const std::string &dir)
{
    iterators.push_back(std::filesystem::directory_iterator(dir + "/4.3"));
    iterators.push_back(std::filesystem::directory_iterator(dir + "/5.3"));
    iterators.push_back(std::filesystem::directory_iterator(dir + "/5.4"));
    iterators.push_back(std::filesystem::directory_iterator(dir + "/9.16"));
    iterators.push_back(std::filesystem::directory_iterator(dir + "/16.9"));
    iterators.push_back(std::filesystem::directory_iterator(dir + "/16.10"));
    iterators.push_back(std::filesystem::directory_iterator(dir + "/32.9"));
    iterators.push_back(std::filesystem::directory_iterator(dir + "/Anders"));
}

void image::ImageFinder::find()
{
    namespace fs = std::filesystem;
    int counter = 0;
    for (fs::directory_iterator &it : iterators)
    {
        for (const fs::directory_entry &item : it)
        {
            if (item.is_regular_file())
            {
                std::string folderName = item.path().parent_path().filename().generic_string() + "/";
                counter++;
                if (folderName.compare("16.9/") == 0 || folderName.compare("16.10/") == 0)
                {
                    ImageInfo info(item.path());
                    std::string imageName = item.path().filename();
                    std::string link = getWallpaperBaseDir() + "/Slideshow/" + imageName;
                    std::string linkTarget = "../" + folderName + imageName;
                    try
                    {
                        fs::create_symlink(linkTarget, link);
                    }
                    catch (const fs::filesystem_error &ex)
                    {
                        LOG4CXX_ERROR(logger, "Failed to create link:\n" << ex.what());
                        throw;
                    }
                }
            }
        }
    }
    LOG4CXX_INFO(logger, "found "<< counter << " images");
}

//bool image::ImageFinder::rebuildChecksums()
//{
//    return false;
//    QUrl url;
//    QNetworkAccessManager manager;
//    QNetworkRequest request;
//    QNetworkReply* reply;
//    QEventLoop loop;
//    QXmlStreamReader reader;
//    //QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
//    int counter = 0;
//    QStringList tables;
//    tables << "blacklist" << "wallpapers";
//    for (QString &table : tables)
//    {
//        QSqlQuery query;
//        query.prepare("SELECT id FROM " + table);
//        if (query.exec())
//        {
//            int imageID;
//            QChar rating;
//            while (query.next())
//            {
//                counter++;
//                imageID = query.value("id").toInt();
//                url = QUrl(QStringLiteral("https://konachan.com/post.xml?limit=1&tags=id%3A") + QString::number(imageID));
//                request = QNetworkRequest(url);
//                reply = manager.get(request);
//                QObject::disconnect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
//                QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
//                loop.exec();
//                reader.setDevice(reply);
//                reader.readNextStartElement();
//                QString md5;
//                if (reader.name() == "posts")
//                {
//                    while (reader.readNextStartElement())
//                    {
//                        md5 = reader.attributes().value("md5").toString();
//                        rating = reader.attributes().value("rating").toString().at(0);
//                        if (table == "blacklist" && rating == 'e')
//                        {
////                            QSqlQuery deleteQuery;
////                            deleteQuery.prepare("DELETE from blacklist WHERE id=(:id)");
////                            deleteQuery.bindValue(":id", imageID);
////                            if (!deleteQuery.exec())
////                            {
////                                qWarning() << "Failed to exec query" << query.lastError();
////                            }
////                            else
////                            {
////                                printf("Deleted %d", std::to_string(imageID));
////                            }
//                            continue;
//                        }
////                        QSqlQuery updateQuery;
////                        updateQuery.prepare("UPDATE " + table + " set md5=(:md5) WHERE id=(:id)");
////                        updateQuery.bindValue(":md5", md5);
////                        updateQuery.bindValue(":id", imageID);
////                        if (!updateQuery.exec())
////                        {
////                            qWarning() << "Failed to exec query" << query.lastError();
////                        }
//                    }
//                }
//            }
//        }
//    }
//    printf("updated %d images", counter);
//    return true;
//}

bool image::ImageFinder::rebuildDB()
{
    namespace fs = std::filesystem;
    std::string url;
    int counter = 0;
    for (fs::directory_iterator &it : iterators)
    {
        for (const fs::directory_entry &item : it)
        {
            std::string folderName = item.path().parent_path().filename().generic_string() + "/";
            std::string imageName = item.path().filename();
            counter++;
            ImageInfo info(item.path());
            int width = info.getWidth();
            int height = info.getHeight();
            if (folderName.compare("16.9/") == 0 || folderName.compare("16.10/") == 0)
            {
                if (width >= 2560)
                {
                    std::string link(getWallpaperBaseDir() + "/Slideshow/" + imageName);
                    if (!fs::exists(link))
                    {
                        std::string linkTarget = "../" + folderName + imageName;
                        try
                        {
                            fs::create_symlink(linkTarget, link);
                            LOG4CXX_INFO(logger, "Linked \"" << link << "\" -> \"" << linkTarget << "\"");
                        }
                        catch (const fs::filesystem_error &ex)
                        {
                            LOG4CXX_ERROR(logger, "Failed to create link:\n" << ex.what());
                            throw;
                        }
                    }
                }
            }
            std::optional<image::Image::image_s> image;
            if (startsWith(imageName,"Konachan"))
            {
                const int imageID = std::stoi(split(imageName, ' ').at(2));
                url = "https://konachan.com/post.xml?limit=1&tags=id%3A" + std::to_string(imageID);
                const nlohmann::json response = nlohmann::json::parse(network::DownloadHelper::download(url));
                image = image::Image::fromKonachan(response[0]);
                for (std::string tag : split(image->imageTags, ' '))
                {
                    std::replace(tag.begin(), tag.end(), '/', '_');
                    if (const std::string linkDir = getWallpaperBaseDir() + "/by-tag/" + tag + '/', link = linkDir + '/' + imageName; !fs::exists(link))
                    {
                        try
                        {
                            if (!fs::exists(linkDir))
                                fs::create_directories(linkDir);
                            const std::string linkTarget = "../.." + folderName + imageName;
                            fs::create_symlink(linkTarget, link);
                            LOG4CXX_INFO(logger, "Linked \"" << link << "\" -> \"" << linkTarget << "\"");
                        }
                        catch (const fs::filesystem_error &ex)
                        {
                            LOG4CXX_ERROR(logger, "Failed to create link:\n" << ex.what());
                            throw;
                        }
                    }
                }
            }
            else
            {
                QRegularExpression searchExpr("[a-f0-9]{32}");
                QRegularExpressionMatch match = searchExpr.match(QString::fromStdString(imageName));
                if (match.hasMatch())
                {
                    // is danbooru image
                    std::string md5 = imageName.substr(imageName.length() - 36, 32);
                    url = "https://danbooru.donmai.us/posts.xml?limit=1&tags=md5%3A" + md5;
                    const nlohmann::json response = nlohmann::json::parse(network::DownloadHelper::download(url));
                    image = image::Image::fromDanbooru(response[0]);
                }
                else
                {
                    continue;
                }
            }
            if (!Image::isDownloaded(Image::image_download_s{ image->checksum, image->imageID, startsWith(imageName, "Konachan") ? "konachan.com" : "danbooru.donmai.us", false }))
            {
                Image::addDBEntry(Image::image_db_s{ image->imageID, folderName + imageName, width, height, image->rating, image->imageTags, image->checksum, startsWith(imageName, "Konachan") ? "konachan.com" : "danbooru.donmai.us" });
            }
            else
            {

            }
        }
    }
    LOG4CXX_INFO(logger, "found "<< counter << " images");
    return true;
}
