#ifndef DANBOORUWORKER_H
#define DANBOORUWORKER_H

#include "network/imageworker.h"

namespace network
{
    class DanbooruWorker : public ImageWorker
    {
        protected:
            void run() override;
        public:
            DanbooruWorker(const std::function<void(const image::Image::image_s &)> &callback, const image::Image::Rating &ratingFilter, const std::string &filter, const std::string &tags = "", const std::string &upstreamName = "danbooru.donmai.us");
            DanbooruWorker(const std::function<void(const image::Image::image_s &)> &callback, const image::Image::Rating &ratingFilter, const std::string &filter, const std::string &tags, const int &width, const int &height, const int &maxheight = -1, const std::string &upstreamName = "danbooru.donmai.us")
                : DanbooruWorker(callback, ratingFilter, filter, tags + (width == -1 ? "" : "+width%3A" + std::to_string(width)) + (height == -1 ? "" : "+height%3A" + std::to_string(height) + ".." + std::to_string(maxheight)), upstreamName) { }
        private:
            static auto &getLogger()
            {
                static auto logger = logger::getSpdLogger("DanbooruWorker");
                return logger;
            }
            std::string filter;
            int minWidth;
            int minHeight;
            int maxHeight;
    };
};

#endif // DANBOORUWORKER_H
