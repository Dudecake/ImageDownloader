#ifndef IMAGEFINDER_H
#define IMAGEFINDER_H

#include <memory>
#include <vector>
#include <filesystem>
#include <log4cxx/logger.h>

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
            static log4cxx::LoggerPtr logger;
            std::vector<std::filesystem::directory_iterator> iterators;
    };
};

#endif // IMAGEFINDER_H
