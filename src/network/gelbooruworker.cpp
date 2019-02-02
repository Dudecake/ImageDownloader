#include "network/gelbooruworker.h"
#include "network/downloadhelper.h"

log4cxx::LoggerPtr network::GelbooruWorker::logger = log4cxx::Logger::getLogger("GelbooruWorker");

network::GelbooruWorker::GelbooruWorker(const std::function<void(const image::Image::image_s &)> &callback, const image::Image::Rating &ratingFilter, std::string filter, const std::string &upstreamName)
    : ImageWorker(callback, ratingFilter, upstreamName)
{
    std::string ratingString = "";
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
    this->worker = std::thread(std::function<void()>([&](){ GelbooruWorker::run(); }));
}

void network::GelbooruWorker::run()
{
    using namespace std::chrono_literals;
    int page = 1;
    do
    {
        const std::string url = "https://" + upstreamName + "/index.php?page=dapi&s=post&json=1&q=index&limit=50&pid=" + std::to_string(page++) + "&tags=" + tags;
        const nlohmann::json reply = nlohmann::json::parse(DownloadHelper::download(url));
        if(!reply.is_array())
        {
            LOG4CXX_WARN(logger, "Reply is not an array, is " << reply.type_name());
            return;
        }
        int count = 0;
        for (const auto &item : reply)
        {
            if (!running)
                break;
            if (const std::optional<image::Image::image_s> image = image::Image::fromGelbooru(item, upstreamName); image)
            {
                count++;
                if (!image::Image::isDownloaded(image::Image::image_download_s{ image->checksum, image->imageID, image->origin }))
                {
                    if (enqueue(*image))
                        std::this_thread::sleep_for(100ms);
                    else
                        return;
                }
            }
        }
        if (count == 0)
            break;
    }
    while (running);
    LOG4CXX_INFO(logger, "Completed search");
}
