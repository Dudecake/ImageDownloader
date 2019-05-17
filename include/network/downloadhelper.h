#ifndef DOWNLOADHELPER_H
#define DOWNLOADHELPER_H

#include <memory>
#include <vector>
#include <mutex>
#include <cstdio>
#include <curl/curl.h>

#include "logger.h"

namespace network
{
    class DownloadHelper {
        private:
            static std::mutex mutex;
            static size_t writeCallBack(void *contents, size_t size, size_t nmemb, void *userp)
            {
                (static_cast<std::string* const>(userp))->append(static_cast<char*>(contents), size * nmemb);
                return size * nmemb;
            }
            static size_t vectorCallBack(void *contents, size_t size, size_t nmemb, void *userp)
            {
                char *data = static_cast<char*>(contents);
                std::vector<char> * const target = static_cast<std::vector<char>*>(userp);
                target->insert(target->end(), data, data + (size * nmemb));
                return size * nmemb;
            }
            static auto &getLogger()
            {
                static auto logger = logger::getSpdLogger("DownloadHelper");
                return logger;
            }
            static CURL* getCurl()
            {
                static std::unique_ptr<CURL, void(*)(CURL*)> curl = std::unique_ptr<CURL, void(*)(CURL*)>(curl_easy_init(), &curl_easy_cleanup);
                return curl.get();
            }
            static CURLM* getMultiCurl(){
                static std::unique_ptr<CURLM, CURLMcode(*)(CURLM*)> multiCurl = []{
                    auto res = std::unique_ptr<CURLM, CURLMcode(*)(CURLM*)>(curl_multi_init(), &curl_multi_cleanup);
                    curl_multi_perform(res.get(), &runningHandles);
                    return res;
                }();
                return multiCurl.get();
            }
            static std::unique_ptr<CURL, void(*)(CURL*)> getMultiHandle(){
                return std::unique_ptr<CURL, void(*)(CURL*)>(curl_easy_init(), &curl_easy_cleanup);
            }
        public:
            DownloadHelper() = delete;
            static int runningHandles;
            static std::string download(const std::string &url);
            static std::vector<char> downloadImage(const std::string &url, const size_t &fileSize = 524288);
            static std::string escape(const std::string &url);
            static std::string unescape(const std::string &url);
    };
} // namespace network

#endif // DOWNLOADHELPER_H
