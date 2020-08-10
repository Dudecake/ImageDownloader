#ifndef IMAGEWORKER_H
#define IMAGEWORKER_H

#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>

#include "image/image.h"

namespace network
{
    class ImageWorker
    {
        protected:
            ImageWorker(const std::function<void(const image::Image::image_s &)> &callback, const image::Image::Rating &ratingFilter, const std::string &upstreamName): upstreamName(upstreamName), callback(callback), ratingFilter(ratingFilter), running(true) { }
            const std::string upstreamName;
            const std::function<void(const image::Image::image_s &)> &callback;
            std::mutex mutex;
            std::condition_variable waitCon;
            const image::Image::Rating ratingFilter;
            std::string tags;
            std::thread worker;
            bool running;
            virtual void run() = 0;
            bool enqueue(const image::Image::image_s &image)
            {
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    waitCon.wait(lock, [&]{return (queueLen.load() < 50) || !running; });
                }
                if (!running)
                {
                    return false;
                }
                callback(image);
                queueLen++;
                return true;
            }
        public:
            enum class WorkerType : uint8_t { Konachan = 0, Yande, Danbooru, Gelbooru };
            std::atomic<int> queueLen = 0;
            void skip(const int &count = 5)
            {
                queueLen.fetch_sub(count);
            }
            void wakeOne()
            {
                waitCon.notify_one();
            }
            void wakeAll()
            {
                waitCon.notify_all();
            }
            void stop()
            {
                running = false;
                waitCon.notify_all();
            }
            virtual ~ImageWorker()
            {
                running = false;
                worker.join();
            }
    };
};

#endif // IMAGEWORKER_H
