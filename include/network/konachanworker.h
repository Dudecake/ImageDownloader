#ifndef KONACHANWORKER_H
#define KONACHANWORKER_H

#include "network/imageworker.h"

namespace network
{
    class KonachanWorker : public ImageWorker
    {
        protected:
            void run() override;
        public:
            KonachanWorker(const std::function<void(const image::Image::image_s &)> &callback, const image::Image::Rating &ratingFilter, std::string filter, const std::string &upstreamName = "konachan.com");
            KonachanWorker(const std::function<void(const image::Image::image_s &)> &callback, const image::Image::Rating &ratingFilter, const std::string &filter, const int &width, const int &height, const int &maxheight = -1, const std::string &upstreamName = "konachan.com")
                : KonachanWorker(callback, ratingFilter, filter + (width == -1 ? "" : "+width%3A" + std::to_string(width)) + (height == -1 ? "" : "+height%3A" + std::to_string(height) + ".." + std::to_string(maxheight)), upstreamName) { }
            KonachanWorker(const std::function<void(const image::Image::image_s &)> &callback, const image::Image::Rating &ratingFilter, const std::string &filter, const std::string &tags, const int &width, const int &height, const int &maxheight = -1, const std::string &upstreamName = "konachan.com")
                : KonachanWorker(callback, ratingFilter, (filter.empty() ? "" : filter + '+') + tags + (width == -1 ? "" : "+width%3A" + std::to_string(width)) + (height == -1 ? "" : "+height%3A" + std::to_string(height) + ".." + std::to_string(maxheight)), upstreamName) { }
        private:
            static log4cxx::LoggerPtr &getLogger()
            {
                static log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("KonachanWorker");
                return logger;
            }
    };
};

#endif // KONACHANWORKER_H
