#ifndef IMAGE_H
#define IMAGE_H

#include <QPixmap>
#include <shared_mutex>
#include <sqlite_orm/sqlite_orm.h>
#include <optional>

#include "utils.h"
#include "logger.h"

#include "image/blacklistimage.h"
#include "image/fraction.h"
#include "image/wallpaperimage.h"
#include "network/downloadhelper.h"
#include "simdjson/simdjson.h"

namespace image
{
    class Image
    {
    public:
        struct image_s
        {
            int64_t imageId = -1;
            int width = -1;
            int height  = -1;
            char rating = 's';
            std::string checksum;
            std::string origin;
            std::string sampleUrl;
            std::string imageUrl;
            std::string imageTags;
            size_t fileSize = -1;
            size_t sampleFileSize = -1;
        };
        struct image_db_s
        {
            int64_t imageId;
            std::string imageName;
            int width;
            int height;
            char rating;
            std::string imageTags;
            std::string checksum;
            std::string origin;
        };
        struct image_download_s
        {
            std::string md5;
            int64_t imageId;
            std::string source = "konachan.com";
            bool useBlacklist = true;
        };
        enum Rating : uint8_t { All = 0x7, Safe = 0x1, Safe_Questionable = 0x03, Questionable = 0x2, Questionable_Explicit = 0x6, Explicit = 0x4 };
        static void initializeDB(const std::string &dbName) { Image::dbName = dbName; }
        static std::optional<image_s> fromKonachan(const simdjson::dom::object &image, const std::string &upstreamName = "konachan.com")
        {
            std::optional<Image::image_s> res;
            auto idField = image["id"];
            if (const int imageId = idField.is_int64() ? static_cast<int>(idField.get_int64()) : -1; imageId != -1) {
                const std::string checksum = image["md5"].get_c_str().value();
                const std::string sampleUrl = network::DownloadHelper::unescape(image["sample_url"].get_c_str().value());
                const std::string url = network::DownloadHelper::unescape(image["file_url"].get_c_str().value());
                const int height = image["height"].get_int64();
                const int width = image["width"].get_int64();
                const char rating = image["rating"].get_c_str().value()[0];
                const std::string imageTags = image["tags"].get_c_str().value();
                const size_t fileSize = image["file_size"].get_int64();
                const size_t sampleFileSize = image["sample_file_size"];
                res = image_s{ imageId, width, height, rating, checksum, upstreamName, sampleUrl, url, imageTags, fileSize, sampleFileSize };
            }
            return res;
        }
        static std::optional<image_s> fromGelbooru(const simdjson::dom::object &image, const std::string &upstreamName = "gelbooru.com")
        {
            std::optional<Image::image_s> res;
            auto idField = image["id"];
            if (const int imageId = idField.is_int64() ? static_cast<int>(idField.get_int64()) : -1; imageId != -1) {
                const std::string checksum = image["hash"].get_c_str().value();
                const std::string sampleUrl = network::DownloadHelper::unescape(image["file_url"].get_c_str().value());
                const int height = image["height"].get_int64();
                const int width = image["width"].get_int64();
                const char rating = image["rating"].get_c_str().value()[0];
                const std::string imageTags = image["tags"].get_c_str().value();
//                const size_t fileSize = image["file_size"].get_int64();
//                const size_t sampleFileSize = image["sample_file_size"].get_int64();
                res = image_s{ imageId, width, height, rating, checksum, upstreamName, sampleUrl, sampleUrl, imageTags, 0, 0 };
            }
            return res;
        }
        static std::optional<image_s> fromDanbooru(const simdjson::dom::object &image, const std::string &upstreamName = "danbooru.donmai.us")
        {
            std::optional<Image::image_s> res;
            auto idField = image["id"];
            if (const int imageId = idField.is_int64() ? static_cast<int>(idField.get_int64()) : -1; imageId != -1) {
                const std::string checksum = image["md5"].get_c_str().value();
                const std::string sampleUrl = image["large_file_url"].get_c_str().value();
                std::string url = image["file_url"].get_c_str().value();
                const int height = image["image_height"].get_int64();
                const int width = image["image_width"].get_int64();
                const char rating = image["rating"].get_c_str().value()[0];
                const std::string imageTags = image["tag_string"].get_c_str().value();
                std::vector<std::string> charactersVec = split(image["tag_string_character"].get_c_str().value(), ' ');
                std::stringstream ss;
                if (!charactersVec.empty()) {
                    ss << charactersVec[0];
                }
                for (int i = 1; i < static_cast<int>(std::min(charactersVec.size(), 6ul)); i++) {
                    ss << ' ' << charactersVec[i];
                }
                std::string characters = ss.str();
                characters.erase(std::remove_if(characters.begin(), characters.end(), &forbiddenFunc), characters.end());
                std::replace_if(characters.begin(), characters.end(), [](char c) { return std::isspace(c); }, '_');
                std::string artist = image["tag_string_artist"].get_c_str().value();
                artist.erase(std::remove_if(artist.begin(), artist.end(), &forbiddenFunc), artist.end());
                std::replace_if(artist.begin(), artist.end(), [](char c) { return std::isspace(c); }, '_');
                if (endsWith(url, "webm") || endsWith(url, "mp4")) {
                    return res;
                }
                if (!startsWith(url, "http")) {
                    url = "https://danbooru.donmai.us" + url;
                }
                if (url.substr(url.find_last_of('/')+1).back() != '_') {
                    url.insert(url.find_last_of('/') + 1, QString("__%1_drawn_by_%2__").arg(QString::fromStdString(characters)).arg(QString::fromStdString(artist)).toStdString());
                }
                const size_t fileSize = image["file_size"];
                const size_t sampleFileSize = fileSize;
                res = image_s{ imageId, width, height, rating, checksum, upstreamName, sampleUrl, url, imageTags, fileSize, sampleFileSize };
            }
            return res;
        }
        static auto& getDBInstance() {
            using sqlite_orm::make_storage;
            using sqlite_orm::make_table;
            using sqlite_orm::make_column;
            using sqlite_orm::primary_key;
            using sqlite_orm::unique;
            static auto storage = make_storage(dbName,
                                             make_table("blacklist",
                                                             make_column("id", &Blacklist::id, primary_key()),
                                                             make_column("imageId", &Blacklist::imageId),
                                                             make_column("dimensions", &Blacklist::dimensions),
                                                             make_column("ratio", &Blacklist::ratio),
                                                             make_column("md5", &Blacklist::md5, unique()),
                                                             make_column("source", &Blacklist::source)),
                                             make_table("wallpapers",
                                                             make_column("id", &Wallpaper::id, primary_key()),
                                                             make_column("imageId", &Wallpaper::imageId),
                                                             make_column("image", &Wallpaper::image),
                                                             make_column("dimensions", &Wallpaper::dimensions),
                                                             make_column("ratio", &Wallpaper::ratio),
                                                             make_column("rating", &Wallpaper::rating),
                                                             make_column("imagetags", &Wallpaper::imageTags),
                                                             make_column("md5", &Wallpaper::md5, unique()),
                                                             make_column("source", &Wallpaper::source)));
            return storage;
        }
        static void addDBEntry(const image_db_s &image);
        static bool isDownloaded(const image_download_s &image);
        static void redownloadAll();
        Image() { }
        explicit Image(const image_s &image): imageId(image.imageId), checksum(image.checksum), origin(image.origin), sampleUrl(image.sampleUrl), imageUrl(image.imageUrl), imageTags(image.imageTags), width(image.width), height(image.height), rating(image.rating), fileSize(image.fileSize), sampleFileSize(image.sampleFileSize) { }
        [[nodiscard]] QPixmap getSample();
        [[nodiscard]] std::vector<char> getBytes();
        std::string getName() const { return imageUrl.substr(imageUrl.find_last_of('/')+1); }
        std::string getImageTags() const { return imageTags; }
        constexpr char getRating() const { return rating; }
        constexpr int getWidth() const { return width; }
        constexpr int getHeight() const { return height; }
        std::string getUrl() const { return imageUrl; }
        bool isDownloaded(const bool &useBlacklist = true) { return isDownloaded(image_download_s{ checksum, imageId, origin, useBlacklist }); }
        constexpr bool nullOrEmpty() const { return imageId < 0; }
        void blacklist();
        bool save(const bool &insert = true);
    private:
        static std::string dbName;
        static std::shared_mutex readWriteLock;
        static auto &getLogger()
        {
            static auto logger = logger::getSpdLogger("Image");
            return logger;
        }
        static bool forbiddenFunc(const char &c) {
            static std::string forbiddenChars = "()\\<>";
            return forbiddenChars.find_first_of(c) != std::string::npos;
        }
        std::optional<std::vector<char>> sampleImage;
        std::optional<std::vector<char>> image;
        int64_t imageId{-1};
        std::string checksum;
        std::string origin;
        std::string sampleUrl;
        std::string imageUrl;
        std::string imageTags;
        int width{-1};
        int height{-1};
        char rating{'s'};
        size_t fileSize{0L};
        size_t sampleFileSize{0L};
        std::string getFolderName(const Fraction &) const;
    };
};  // namespace image

#endif // IMAGE_H
