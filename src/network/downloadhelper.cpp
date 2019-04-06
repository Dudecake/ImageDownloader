#include "network/downloadhelper.h"

#include <functional>

std::mutex network::DownloadHelper::mutex = std::mutex();

std::string network::DownloadHelper::download(const std::string &url)
{
    CURLcode res;
    CURL *localCurl = getCurl();
    std::string buffer;

    mutex.lock();
    curl_easy_setopt(localCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(localCurl, CURLOPT_WRITEFUNCTION, writeCallBack);
    curl_easy_setopt(localCurl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(localCurl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(localCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
    if (res = curl_easy_perform(localCurl); res != CURLcode::CURLE_OK) {
        LOG4CXX_WARN(getLogger(), "Failed to download " << url << ": " << url << "\n" << curl_easy_strerror(res));
    }
    mutex.unlock();

    return !buffer.empty() ? buffer : "[]";
}

std::vector<char> network::DownloadHelper::downloadImage(const std::string &url, const size_t &fileSize)
{
    CURLcode res;
    CURL *localCurl = getCurl();
    std::vector<char> buffer;
    buffer.reserve(fileSize);

    mutex.lock();
    curl_easy_setopt(localCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(localCurl, CURLOPT_WRITEFUNCTION, vectorCallBack);
    curl_easy_setopt(localCurl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(localCurl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(localCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
    if (res = curl_easy_perform(localCurl); res != CURLcode::CURLE_OK) {
        LOG4CXX_WARN(getLogger(), "Failed to download " << url << ": " << url << "\n" << curl_easy_strerror(res));
    }
    mutex.unlock();

    return buffer;
}

std::string network::DownloadHelper::escape(const std::string &url)
{
    char *encodedUrl = curl_easy_escape(getCurl(), url.c_str(), static_cast<int>(url.length()));
    const std::string res = std::string(encodedUrl);
    curl_free(encodedUrl);
    return res;
}

std::string network::DownloadHelper::unescape(const std::string &url)
{
    int len = 0;
    char *encodedUrl = curl_easy_unescape(getCurl(), url.c_str(), static_cast<int>(url.length()), &len);
    const std::string res = std::string(encodedUrl, static_cast<size_t>(len));
    curl_free(encodedUrl);
    return res;

}
