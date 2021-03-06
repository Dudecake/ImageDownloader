#include "network/konachanworker.h"
#include "network/downloadhelper.h"

network::KonachanWorker::KonachanWorker(const std::function<void(const image::Image::image_s &)> &callback, const image::Image::Rating &ratingFilter, std::string filter, const std::string &upstreamName)
    : ImageWorker(callback, ratingFilter, upstreamName)
{
    std::string ratingString;
    if (!filter.empty() && !endsWith(filter, "+"))
    {
        filter = filter + '+';
    }
    switch (ratingFilter)
    {
        case image::Image::Rating::All:
            break;
        case image::Image::Rating::Safe:
            ratingString = "rating%3Asafe";
            break;
        case image::Image::Rating::Safe_Questionable:
            ratingString = "-rating%3Aexplicit";
            break;
        case image::Image::Rating::Questionable:
            ratingString = "rating%3Aquestionable";
            break;
        case image::Image::Rating::Questionable_Explicit:
            ratingString = "-rating%3Asafe";
            break;
        case image::Image::Rating::Explicit:
            ratingString = "rating%3Aexplicit";
            break;
    }
    this->tags = ratingString + (filter.empty() ? "" : '+' + filter);
    this->worker = std::thread(std::function<void()>([&](){ KonachanWorker::run(); }));
}

void network::KonachanWorker::run()
{
    using namespace std::chrono_literals;
    int page = 1;
    simdjson::dom::parser parser;
    do
    {
        const std::string url = "https://" + upstreamName + "/post.json?limit=50&page=" + std::to_string(page++) + "&tags=" + tags;
        const simdjson::dom::element reply = parser.parse(DownloadHelper::download(url));
        if(!reply.is_array())
        {
            getLogger()->warn("Reply is not an array, is {}", reply.type());
            return;
        }
        int count = 0;
        for (const auto &item : reply.get_array())
        {
            if (!running)
            {
                break;
            }
            if (const std::optional<image::Image::image_s> image = image::Image::fromKonachan(item, upstreamName); image)
            {
                count++;
                if (!image::Image::isDownloaded(image::Image::image_download_s{ image->checksum, image->imageId, image->origin }))
                {
                    if (enqueue(*image))
                    {
                        std::this_thread::sleep_for(100ms);
                    }
                    else
                    {
                        return;
                    }
                }
            }
        }
        if (count == 0)
        {
            break;
        }
    }
    while (running);
    getLogger()->info("Completed search");
}
