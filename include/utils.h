#ifndef IMAGEDOWNLOADER_UTILS_H
#define IMAGEDOWNLOADER_UTILS_H

#include <string>
#include <vector>
#include <cstdarg>
#include <algorithm>

inline bool startsWith(const std::string &value, const std::string &suffix)
{
    if (suffix.size() > value.size())
    {
        return false;
    }
    return std::equal(suffix.begin(), suffix.end(), value.begin());
}

inline bool endsWith(const std::string &value, const std::string &suffix)
{
    if (suffix.size() > value.size())
    {
        return false;
    }
    return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
}

inline std::string expandEnv(const std::string &env)
{
    const char *path = getenv(env.c_str());
    return path ? std::string(path) : std::string();
}

inline std::string format(const char * const zcFormat, ...)
{
    // initialize use of the variable argument array
    va_list vaArgs;
    va_start(vaArgs, zcFormat);

    // reliably acquire the size
    // from a copy of the variable argument array
    // and a functionally reliable call to mock the formatting
    va_list vaArgsCopy;
    va_copy(vaArgsCopy, vaArgs);
    const int iLen = std::vsnprintf(nullptr, 0, zcFormat, vaArgsCopy);
    va_end(vaArgsCopy);

    // return a formatted string without risking memory mismanagement
    // and without assuming any compiler or platform specific behavior
    std::vector<char> zc(iLen + 1);
    std::vsnprintf(zc.data(), zc.size(), zcFormat, vaArgs);
    va_end(vaArgs);
    return std::string(zc.data(), iLen);
}

inline std::string getHumanFileSize(double fileSize)
{
    static const std::vector<std::string> units = { "B", "KiB", "MiB", "GiB" };
    int i = 0;
    for (; fileSize > 1024 && i < units.size(); i++)
    {
        fileSize /= 1024;
    }
    return format("%.*f %s", fileSize, units[i].c_str());
}

inline std::string getDataDir()
{
    static std::string dataDir = []{
        std::string res = expandEnv("HOME");
        if (res.empty())
        {
            res = expandEnv("USERPROFILE");
            if (res.empty())
            {
                if (std::string hdrive = expandEnv("HOMEDRIVE"), hpath = expandEnv("HOMEPATH"); !hdrive.empty() && !hpath.empty())
                {
                    res = hdrive + hpath;
                }
            }
        }
#ifdef __linux__
        res += "/.local/share/ImageDownloader";
#elif _WIN32
        res += "/AppData/Local/ImageDownloader";
#elif __MACOSX__
        res += "/Library/Application Support/ImageDownloader";
#endif
        return res;
    }();
    return dataDir;
}

inline std::string getWallpaperBaseDir()
{
    static std::string wallpaperBaseDir = []{
        std::string res = expandEnv("HOME");
        if (res.empty())
        {
            res = expandEnv("USERPROFILE");
            if (res.empty())
            {
                if (std::string hdrive = expandEnv("HOMEDRIVE"), hpath = expandEnv("HOMEPATH"); !hdrive.empty() && !hpath.empty())
                {
                    res = hdrive + hpath;
                }
            }
        }
        return res + "/Downloads/Achtergronden";
    }();
    return wallpaperBaseDir;
}

inline std::string getTempDir()
{
    static std::string tempDir = []{
        std::string res;
#ifdef _WIN32
        std::string userName = expandEnv("USERNAME");
        res = "C:/Users/" + userName + "/AppData/Local/Temp/ImageDownloader/cache";
#else
        res = "/tmp/ImageDownloader/cache";
#endif
        return res;
    }();
    return tempDir;
}

inline std::vector<std::string> split(const std::string &string, const char &separator, const bool &includeEmpty = false)
{
    std::vector<std::string> res;
    int lastIndex = 0;
    int currentIndex = 0;
    while ((currentIndex = string.find_first_of(separator, lastIndex)) != std::string::npos)
    {
        std::string item = string.substr(lastIndex, currentIndex - lastIndex);
        if (!item.empty() || includeEmpty)
        {
            res.push_back(item);
        }
        lastIndex = currentIndex + 1;
    }
    res.push_back(string.substr(lastIndex, currentIndex));
    return res;
}


inline std::vector<std::string> split(const std::string &string, const std::string &separator, const bool &includeEmpty = false)
{
    std::vector<std::string> res;
    size_t lastIndex = 0;
    size_t currentIndex = 0;
    while ((currentIndex = string.find_first_of(separator, lastIndex)) != std::string::npos)
    {
        std::string item = string.substr(lastIndex, currentIndex - lastIndex);
        if (!item.empty() || includeEmpty)
        {
            res.push_back(item);
        }
        lastIndex = currentIndex + 1;
    }
    res.push_back(string.substr(lastIndex, currentIndex));
    return res;
}

inline size_t find_nth_of(const std::string &string, const char &character, const int &occurrences)
{
    if (std::count(string.begin(), string.end(), character) < occurrences)
    {
        return std::string::npos;
    }
    size_t lastIndex = 0;
    size_t currentIndex = 0;
    int count = 0;
    while ((currentIndex = string.find_first_of(character, lastIndex)) != std::string::npos)
    {
        if (++count == occurrences)
        {
            break;
        }
        lastIndex = currentIndex + 1;
    }
    if (count != occurrences)
    {
        currentIndex = std::string::npos;
    }
    return currentIndex;
}

#endif // IMAGEDOWNLOADER_UTILS_H
