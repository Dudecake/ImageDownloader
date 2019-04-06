#ifndef DOWNLOADHELPER_H
#define DOWNLOADHELPER_H

#include <memory>
#include <vector>
#include <mutex>
#include <cstdio>
#include <curl/curl.h>
#include <log4cxx/logger.h>

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
            static log4cxx::LoggerPtr& getLogger()
            {
                static log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("DownloadHelper");
                return logger;
            }
            static CURL* getCurl()
            {
                static std::unique_ptr<CURL, void(*)(CURL*)> curl = std::unique_ptr<CURL, void(*)(CURL*)>(curl_easy_init(), &curl_easy_cleanup);
                return curl.get();
            }
        public:
            DownloadHelper() = delete;
            static std::string download(const std::string &url);
            static std::vector<char> downloadImage(const std::string &url, const size_t &fileSize = 524288);
            static std::string escape(const std::string &url);
            static std::string unescape(const std::string &url);
    };
}

#endif // DOWNLOADHELPER_H
