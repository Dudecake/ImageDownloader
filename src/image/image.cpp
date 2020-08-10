#include "image/image.h"

#include <filesystem>
#include <fstream>

std::string image::Image::dbName = std::string();
std::shared_mutex image::Image::readWriteLock;

using namespace sqlite_orm;

bool image::Image::isDownloaded(const image_download_s &image)
{
    bool res = false;
    auto storage = getDBInstance();
    std::shared_lock<std::shared_mutex> lock(readWriteLock);
    if (auto result = storage.select(count(&Wallpaper::imageId), where(eq(&Wallpaper::md5, image.md5))); result.front() > 0)
    {
        //contains image
        res = true;
    }
    else if (auto result = storage.select(count(&Wallpaper::imageId), where(eq(&Wallpaper::imageId, image.imageId) and eq(&Wallpaper::source, image.source))); result.front() > 0)
    {
        //contains image
        res = true;
    }
    else if (image.useBlacklist)
    {
        if (auto result = storage.select(count(&Blacklist::imageId), where(eq(&Blacklist::md5, image.md5))); result.front() > 0)
        {
            //contains image
            res = true;
        }
        else if (auto result = storage.select(count(&Blacklist::imageId), where(eq(&Blacklist::imageId, image.imageId) and eq(&Blacklist::source, image.source))); result.front() > 0)
        {
            //contains image
            res = true;
        }
    }
    return res;
}

void image::Image::redownloadAll()
{
    namespace fs = std::filesystem;
    int count = 0;
    auto storage = getDBInstance();
    for (const Wallpaper &wallpaper : storage.iterate<Wallpaper>())
    {
        fs::path path = getWallpaperBaseDir() + '/' + wallpaper.image;
        if (!fs::exists(path))
        {
            std::vector<std::string> dimensions = split(wallpaper.dimensions, 'x');
            std::string imageUrl;
            if (wallpaper.source == "konachan.com" || wallpaper.source == "yande.re")
            {
                imageUrl = wallpaper.source + "/image/" + wallpaper.md5 + "/" + path.filename().string();
            }
            else if (wallpaper.source == "danbooru.donmai.us")
            {
                imageUrl = wallpaper.source + "/data/" + path.filename().string();
            }
            else if (wallpaper.source == "gelbooru.com")
            {
                imageUrl = "https://simg3.gelbooru.com/images/" + wallpaper.md5.substr(0, 2) + '/' + wallpaper.md5.substr(2, 4) + '/' + path.filename().string();
            }
            if (Image image(Image(image_s{wallpaper.imageId, std::stoi(dimensions[0]), std::stoi(dimensions[1]), wallpaper.rating[0], wallpaper.md5, wallpaper.source, "", imageUrl, wallpaper.imageTags})); !image.save(false)) {
                getLogger()->warn("Could not redownload image");
                continue;
            }
            count++;
        }
    }
    getLogger()->info("Redownloaded {} images", count);
}

[[nodiscard]] QPixmap image::Image::getSample()
{
    if (!sampleImage)
    {
        namespace fs = std::filesystem;
        std::string tempDir = getTempDir();
        std::string fileName = getTempDir() + '/' + checksum;
        sampleImage = network::DownloadHelper::downloadImage(sampleUrl, sampleFileSize);
        bool created = true;
        if (!fs::exists(tempDir))
        {
            created = fs::create_directories(tempDir);
        }
        if (created && !fs::exists(fileName))
        {
            std::fstream ostream(fileName, std::ios_base::out | std::ios_base::binary);
            ostream.write(sampleImage->data(), static_cast<int64_t>(sampleImage->size()));
        }
    }
    QPixmap res;
    res.loadFromData(reinterpret_cast<const uchar *>(sampleImage->data()), static_cast<unsigned int>(sampleImage->size()));
    return res.isNull() ? res : res.scaled(QSize(1280, 720), Qt::KeepAspectRatio);
}

[[nodiscard]] std::vector<char> image::Image::getBytes()
{
    if (!image)
    {
        image = network::DownloadHelper::downloadImage(imageUrl, fileSize);
    }
    return *image;
}

bool image::Image::save(const bool &insert)
{
    namespace fs = std::filesystem;
    const std::string folderName = getFolderName(Fraction{width, height});
    const std::string imageName = imageUrl.substr(imageUrl.find_last_of('/') + 1);
    const std::string fileName = getWallpaperBaseDir() + folderName + imageName;

    std::string ratingString(1, rating);

    if (fileName.length() >= 255){
        getLogger()->warn("Skipping file {} with length: {}", fileName, fileName.length());
        return false;
    }

    if (!fs::exists(fileName))
    {
        std::fstream ostream(fileName, std::fstream::out | std::fstream::binary);
        std::vector<char> arr = getBytes();
        ostream.write(arr.data(), static_cast<int64_t>(arr.size()));
        if (ostream.fail() || ostream.bad()) {
            getLogger()->error("Failed to write file: {} \n {}", fileName);
            return false;
        }
        getLogger()->info("written \"{}\", size: {} bytes", fileName, ostream.tellp());
        if (const Fraction ratio(width, height); width >= 2560 && (ratio.equals(Fraction(16, 9)) || ratio.equals(Fraction(16, 10))))
        {
            const std::string link = getWallpaperBaseDir() + "/Slideshow/" + imageName;
            const std::string linkTarget = ".." + folderName + imageName;
            std::error_code err;
            fs::create_symlink(linkTarget, link, err);
            if (err) {
                getLogger()->error("Failed to create link:\n{}", err.message());
                return false;
            }
            getLogger()->info("Linked \"{}\" -> \"{}\"", link, linkTarget);
        }
    }
    for (std::string tagString : split(imageTags, ' '))
    {
        std::replace(tagString.begin(), tagString.end(), '/', '_');
        if (const std::string linkDir = getWallpaperBaseDir() + "/by-tag/" + tagString, link = linkDir + '/' + imageName; !fs::exists(link))
        {
            std::error_code err;
            if (!fs::exists(linkDir))
            {
                fs::create_directories(linkDir, err);
                if (err) {
                    getLogger()->error("Failed to create directory:\n{}", err.message());
                    return false;
                }
            }
            const std::string linkTarget = "../.." + folderName + imageName;
            fs::create_symlink(linkTarget, link, err);
            if (err) {
                getLogger()->error("Failed to create link:\n{}", err.message());
                return false;
            }
            getLogger()->info("Linked \"{}\" -> \"{}\"", link, linkTarget);
        }
    }
    if (insert)
    {
        const std::unique_lock<std::shared_mutex> lock(readWriteLock);
        getDBInstance().insert(Wallpaper{-1, static_cast<int>(imageId), (folderName.substr(1) + imageName),
                                         std::to_string(width) + 'x' + std::to_string(height), Fraction(width, height).toString(),
                                         ratingString, imageTags, checksum, origin});
    }
    return true;
}

void image::Image::addDBEntry(const image_db_s &image)
{
    std::string ratingString(1, image.rating);
    const std::unique_lock<std::shared_mutex> lock(readWriteLock);
    getDBInstance().insert(Wallpaper{-1, static_cast<int>(image.imageId), image.imageName,
                                     std::to_string(image.width) + 'x' + std::to_string(image.height), Fraction(image.width, image.height).toString(),
                                     ratingString, image.imageTags, image.checksum, image.origin});
}

void image::Image::blacklist()
{
    getLogger()->info("blacklisted \"{}\"", imageUrl.substr(imageUrl.find_last_of('/') + 1));
    const std::unique_lock<std::shared_mutex> lock(readWriteLock);
    getDBInstance().insert(Blacklist{-1, static_cast<int>(imageId), std::to_string(width) + 'x' + std::to_string(height),
                                     Fraction(width, height).toString(), checksum, origin});
}

std::string image::Image::getFolderName(const Fraction &fraction) const
{
    std::string res = "/Anders/";
    if (fraction.equals(Fraction(9, 16)))
    {
        res = "/9.16/";
    }
    else if (fraction.equals(Fraction(5, 4)))
    {
        res = "/5.4/";
    }
    else if (fraction.equals(Fraction(4, 3)))
    {
        res = "/4.3/";
    }
    else if (fraction.equals(Fraction(16, 10)))
    {
        res = "/16.10/";
    }
    else if (fraction.equals(Fraction(5, 3)))
    {
        res = "/5.3/";
    }
    else if (fraction.equals(Fraction(16, 9)))
    {
        res = "/16.9/";
    }
    else if (fraction.equals(Fraction(32, 9)))
    {
        res = "/32.9/";
    }
    return res;
}
