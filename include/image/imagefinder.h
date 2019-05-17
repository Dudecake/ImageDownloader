#ifndef IMAGEFINDER_H
#define IMAGEFINDER_H

#include <memory>
#include <vector>
#include <filesystem>
#include "logger.h"

namespace image
{
    class ImageFinder
    {
        public:
            explicit ImageFinder(const std::string &dir);
            void find();
            //bool rebuildChecksums();
            bool rebuildDB();
        private:
            static auto &getLogger()
            {
                static auto logger = logger::getSpdLogger("Image");
                return logger;
            }
            std::vector<std::filesystem::directory_iterator> iterators;
    };
};

#endif // IMAGEFINDER_H
