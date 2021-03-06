#include "network/danbooruworker.h"
#include "network/downloadhelper.h"

network::DanbooruWorker::DanbooruWorker(const std::function<void(const image::Image::image_s &)> &callback, const image::Image::Rating &ratingFilter, const std::string &filter, const std::string &tags, const std::string &upstreamName)
    : ImageWorker(callback, ratingFilter, upstreamName)
{
    const std::string fullTags = (filter.empty() ? "" : filter + '+') + tags;
    if (!fullTags.empty())
    {
        for (const std::string &tag : split(tags, '+'))
        {
            if (startsWith(tag, "width"))
            {
                // is query for 9:16 ratio image
                std::vector widthList = split(tag.substr(8), "..");
                this->minWidth = std::stoi(widthList.at(0));
            }
            else if (startsWith(tag, "height"))
            {
                if (tag.find("..") != std::string::npos)
                {
                    // skip beginning of string "height%3A1440..1600"
                    std::vector heightList = split(tag.substr(9), "..");
                    this->minHeight = std::stoi(heightList.at(0));
                    this->maxHeight = std::stoi(heightList.at(1));
                    if (this->maxHeight == 0)
                    {
                        this->maxHeight = std::numeric_limits<int>::max();
                    }
                }
                else
                {
                    this->minHeight = std::stoi(tag);
                }
            }
        }
        size_t index = find_nth_of(fullTags, '+', 2);
        if (index != std::string::npos)
        {
            this->tags = fullTags.substr(0, index);
            std::stringstream ss;
            for (const std::string &item : split(fullTags.substr(index + 1), '+'))
            {
                if (!startsWith(item, "width") && !startsWith(item, "height"))
                {
                    ss << item << '+';
                }
            }
            if (ss.tellp() != 0)
            {
                std::string filterString = ss.str();
                filterString.pop_back();
                ss.str(filterString);
            }
            this->filter = ss.str();
        }
        else
        {
            this->tags = fullTags;
        }
    }
    else
    {
        this->tags = fullTags;
    }
    this->worker = std::thread(std::function<void()>([&](){ DanbooruWorker::run(); }));
}

void network::DanbooruWorker::run()
{
    using namespace std::chrono_literals;
    int page = 1;
    simdjson::dom::parser parser;
    do
    {
        const std::string url = "https://" + upstreamName + "/posts.json?limit=50&page=" + std::to_string(page++) + "&tags=" + tags;
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
            count++;
            if (const std::optional<image::Image::image_s> image = image::Image::fromDanbooru(item); image)
            {
                const char rating = image->rating;
                if (rating == 's' && !(ratingFilter & image::Image::Rating::Safe))
                {
                    continue;
                }
                if (rating == 'q' && !(ratingFilter & image::Image::Rating::Questionable))
                {
                    continue;
                }
                if (rating == 'e' && !(ratingFilter & image::Image::Rating::Explicit))
                {
                    continue;
                }
                if (minWidth > 0)
                {
                    if (const int width = image->width; width < minWidth)
                    {
                        continue;
                    }
                }
                if (const int height = image->height; minHeight > 0)
                {
                    if (maxHeight > 0)
                    {
                        if (height < minHeight || height > maxHeight)
                        {
                            continue;
                        }
                    }
                    else
                    {
                        if (height != minHeight)
                        {
                            continue;
                        }
                    }
                }
                if (!filter.empty())
                {
                    bool shouldBreak = false;
                    const std::vector<std::string> tagList = split(image->imageTags, ' ');
                    for (const std::string &filterTag : split(filter, '+'))
                    {
                        if (!startsWith(filterTag, "-"))
                        {
                            if (std::find(tagList.begin(), tagList.end(), filterTag) == tagList.end())
                            {
                                shouldBreak = true;
                                break;
                            }
                        }
                        else
                        {
                            if (std::find(tagList.begin(), tagList.end(), filterTag.substr(1)) != tagList.end())
                            {
                                shouldBreak = true;
                                break;
                            }
                        }
                    }
                    if (shouldBreak)
                    {
                        continue;
                    }
                }
                if (image::Image::isDownloaded(image::Image::image_download_s{ image->checksum, image->imageId, image->origin }))
                {
                    continue;
                }
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
        if (count == 0)
        {
            getLogger()->info("Completed search");
            return;
        }
    }
    while (running);
}
