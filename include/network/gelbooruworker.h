#ifndef GELBOORUWORKER_H
#define GELBOORUWORKER_H

#include "network/imageworker.h"

namespace network
{
    class GelbooruWorker : public ImageWorker
    {
        protected:
            void run() override;
        public:
            GelbooruWorker(const std::function<void(const image::Image::image_s &)> &callback, const image::Image::Rating &ratingFilter, std::string filter, const std::string &upstreamName = "gelbooru.com");
            GelbooruWorker(const std::function<void(const image::Image::image_s &)> &callback, const image::Image::Rating &ratingFilter, const std::string &filter, const int &width, const int &height, const int &maxheight = -1, const std::string &upstreamName = "gelbooru.com")
                : GelbooruWorker(callback, ratingFilter, filter + (width == -1 ? "" : "+width%3A" + std::to_string(width)) + (height == -1 ? "" : "+height%3A" + std::to_string(height) + ".." + std::to_string(maxheight)), upstreamName) { }
            GelbooruWorker(const std::function<void(const image::Image::image_s &)> &callback, const image::Image::Rating &ratingFilter, const std::string &filter, const std::string &tags, const int &width, const int &height, const int &maxheight = -1, const std::string &upstreamName = "gelbooru.com")
                : GelbooruWorker(callback, ratingFilter, (filter.empty() ? filter : filter + '+') + tags + (width == -1 ? "" : "+width%3A" + std::to_string(width)) + (height == -1 ? "" : "+height%3A" + std::to_string(height) + ".." + std::to_string(maxheight)), upstreamName) { }
        private:
            static auto &getLogger()
            {
                static auto logger = logger::getSpdLogger("GelbooruWorker");
                return logger;
            }
    };
};

#endif // GELBOORUWORKER_H
