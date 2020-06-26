#ifndef BLACKLISTIMAGE_H
#define BLACKLISTIMAGE_H

#include <string>

namespace image {
    class Blacklist {
    public:
        long id;
        int imageId;
        std::string dimensions;
        std::string ratio;
        std::string md5;
        std::string source;
    };
}; // namespace image

#endif // BLACKLISTIMAGE_H
