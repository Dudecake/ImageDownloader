#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include <fstream>

#include "image/image.h"

namespace image
{
    class ImageInfo
    {
        public:
            ImageInfo(const std::string &fileName)
            {
                std::fstream fstream(fileName, std::fstream::in);
                processStream(fstream);
            }
            std::string toString() const
            {
                return "MIME Type : " + mimeType + '\t' + " Width : " + std::to_string(width) + '\t' + " Height : " + std::to_string(height);
            }

            constexpr int getHeight() const
            {
                return height;
            }

            constexpr int getWidth() const
            {
                return width;
            }

            std::string getMimeType() const
            {
                return mimeType;
            }
        private:
            int height, width;
            std::string mimeType;
            void processStream(std::fstream &fstream);
            int readInt(std::fstream &fstream, const int &noOfBytes, const bool &bigEndian);
    };
};

#endif // IMAGEINFO_H
