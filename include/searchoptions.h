#ifndef SEARCHOPTIONS_H
#define SEARCHOPTIONS_H

#include "image/image.h"
#include "network/imageworker.h"

class SearchOptions {
private:
    std::string tags;
    image::Image::Rating rating;
    network::ImageWorker::WorkerType worker;

public:
    explicit SearchOptions(const std::string &tags, const image::Image::Rating &rating, const network::ImageWorker::WorkerType &worker)
    {
        this->tags = tags;
        this->rating = rating;
        this->worker = worker;
    }
    std::string getTags()
    {
        if (tags.find_first_of(' ') != std::string::npos)
            std::replace(tags.begin(), tags.end(), ' ', '+');
        return tags;
    }
    image::Image::Rating getRating() const { return rating; }
    network::ImageWorker::WorkerType getWorker() const { return worker; }
};

#endif // SEARCHOPTIONS_H
