#ifndef WALLPAPERIMAGE_H
#define WALLPAPERIMAGE_H

#include <string>

namespace image {
    class Wallpaper
    {
        public:
            long id;
            int imageId;
            std::string image;
            std::string dimensions;
            std::string ratio;
            std::string rating;
            std::string imageTags;
            std::string md5;
            std::string source;
    };
};

#endif // WALLPAPERIMAGE_H
