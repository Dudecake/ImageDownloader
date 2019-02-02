#ifndef WORKERFACTORY_H
#define WORKERFACTORY_H

#include <memory>
#include <functional>

#include "network/imageworker.h"
#include "network/konachanworker.h"
#include "network/danbooruworker.h"
#include "network/gelbooruworker.h"
#include "image/image.h"

namespace network
{
    class WorkerFactory: public std::enable_shared_from_this<WorkerFactory>
    {
    private:
        friend class ImageWorker;
        friend class KonachanWorker;
        friend class DanbooruWorker;
        friend class GelbooruWorker;
        std::function<void(const image::Image::image_s &)> callback;
        ImageWorker::WorkerType worker;
        image::Image::Rating ratingFilter = image::Image::Rating::Safe_Questionable;
        std::string tags;
        std::string filter;
        int width = -1;
        int height = -1;
        int maxheight = -1;
        WorkerFactory() = default;
    public:
        static std::shared_ptr<WorkerFactory> getInstance()
        {
            std::shared_ptr<WorkerFactory> res = std::shared_ptr<WorkerFactory>(new WorkerFactory());
            return res;
        }

        std::shared_ptr<WorkerFactory> withCallback(const std::function<void(const image::Image::image_s &)> &callback)
        {
            this->callback = callback;
            return shared_from_this();
        }

        std::shared_ptr<WorkerFactory> withWorkerType(const ImageWorker::WorkerType &workerType)
        {
            worker = workerType;
            return shared_from_this();
        }

        std::shared_ptr<WorkerFactory> withRatingFilter(const image::Image::Rating &ratingFilter)
        {
            this->ratingFilter = ratingFilter;
            return shared_from_this();
        }

        std::shared_ptr<WorkerFactory> withTags(const std::string &tags)
        {
            this->tags = tags;
            return shared_from_this();
        }

        std::shared_ptr<WorkerFactory> withFilter(const std::string &filter)
        {
            this->filter = filter;
            return shared_from_this();
        }

        std::shared_ptr<WorkerFactory> withWidth(const int &width)
        {
            this->width = width;
            return shared_from_this();
        }

        std::shared_ptr<WorkerFactory> withHeight(const int &height)
        {
            this->height = height;
            return shared_from_this();
        }

        std::shared_ptr<WorkerFactory> withMaxHeight(const int &maxHeight)
        {
            this->maxheight = maxHeight;
            return shared_from_this();
        }

        std::unique_ptr<ImageWorker> build() const
        {
            std::unique_ptr<ImageWorker> res;
            switch (worker)
            {
                case ImageWorker::WorkerType::Konachan:
                    [[fallthrough]];
                case ImageWorker::WorkerType::Yande:
                    res = std::make_unique<KonachanWorker>(callback, ratingFilter, tags, filter, width, height, maxheight, worker == ImageWorker::WorkerType::Konachan ? "konachan.com" : "yande.re");
                    break;
                case ImageWorker::WorkerType::Danbooru:
                    res = std::make_unique<DanbooruWorker>(callback, ratingFilter, tags, filter, width, height, maxheight);
                    break;
                case ImageWorker::WorkerType::Gelbooru:
                    res = std::make_unique<GelbooruWorker>(callback, ratingFilter, tags, filter, width, height, maxheight);
            }
            return res;
        }
    };
};

#endif // WORKERFACTORY_H
