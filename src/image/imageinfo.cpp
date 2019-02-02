#include "image/imageinfo.h"

void image::ImageInfo::processStream(std::fstream &fstream)
{
    char buf[2];
    memset(&buf, 0 , sizeof buf);
    int c1; fstream.read(buf, 1); c1 = static_cast<unsigned char>(buf[0]);
    int c2; fstream.read(buf, 1); c2 = static_cast<unsigned char>(buf[0]);
    int c3; fstream.read(buf, 1); c3 = static_cast<unsigned char>(buf[0]);

    mimeType = "";
    width = height = -1;

    if (c1 == 'G' && c2 == 'I' && c3 == 'F')
    { // GIF
        fstream.seekg(3, std::fstream::cur);
        width = readInt(fstream, 2, false);
        height = readInt(fstream, 2, false);
        mimeType = "image/gif";
    }
    else if (c1 == 0xFF && c2 == 0xD8)
    { // JPG
        while (c3 == 255)
        {
            fstream.read(buf, 1);
            int marker; marker = static_cast<unsigned char>(buf[0]);
            int len = readInt(fstream, 2, true);
            if (marker == 192 || marker == 193 || marker == 194)
            {
                fstream.seekg(1, std::fstream::cur);
                height = readInt(fstream, 2, true);
                width = readInt(fstream, 2, true);
                mimeType = "image/jpeg";
                break;
            }
            fstream.seekg(len - 2, std::fstream::cur);
            fstream.read(buf, 1);
            c3 = static_cast<unsigned char>(buf[0]);
        }
    }
    else if (c1 == 137 && c2 == 80 && c3 == 78)
    { // PNG
        fstream.seekg(15, std::fstream::cur);
        width = readInt(fstream, 2, true);
        fstream.seekg(2, std::fstream::cur);
        height = readInt(fstream, 2, true);
        mimeType = "image/png";
    }
    else if (c1 == 66 && c2 == 77)
    { // BMP
        fstream.seekg(15, std::fstream::cur);
        width = readInt(fstream, 2, false);
        fstream.seekg(2, std::fstream::cur);
        height = readInt(fstream, 2, false);
        mimeType = "image/bmp";
    }
    else
    {
        fstream.read(buf, 1);
        int c4 = static_cast<unsigned char>(buf[0]);
        if ((c1 == 'M' && c2 == 'M' && c3 == 0 && c4 == 42) || (c1 == 'I' && c2 == 'I' && c3 == 42 && c4 == 0))
        { // TIFF
            bool bigEndian = c1 == 'M';
            int ifd = 0;
            int entries;
            ifd = readInt(fstream, 4, bigEndian);
            fstream.seekg(ifd - 8, std::fstream::cur);
            entries = readInt(fstream, 2, bigEndian);
            for (int i = 1; i <= entries; i++)
            {
                int tag = readInt(fstream, 2, bigEndian);
                int fieldType = readInt(fstream, 2, bigEndian);
                // long count = readInt(fstream, 4, bigEndian);
                int valOffset;
                if ((fieldType == 3 || fieldType == 8))
                {
                    valOffset = readInt(fstream, 2, bigEndian);
                    fstream.seekg(2, std::fstream::cur);
                }
                else
                {
                    valOffset = readInt(fstream, 4, bigEndian);
                }
                if (tag == 256)
                {
                    width = valOffset;
                }
                else if (tag == 257)
                {
                    height = valOffset;
                }
                if (width != -1 && height != -1)
                {
                    mimeType = "image/tiff";
                    break;
                }
            }
        }
    }
    if (mimeType.empty())
    {
        fprintf(stderr, "Unsupported image type\n");
    }
}

int image::ImageInfo::readInt(std::fstream &fstream, const int &noOfBytes, const bool &bigEndian)
{
    int ret = 0;
    int sv = bigEndian ? ((noOfBytes - 1) * 8) : 0;
    int cnt = bigEndian ? -8 : 8;
    char buf[2];
    for (int i = 0; i < noOfBytes; i++)
    {
        memset(&buf, 0, sizeof buf);
        int temp;
        fstream.read(buf, 1);
        temp = static_cast<unsigned char>(buf[0]);
        ret |= (temp << sv);
        sv += cnt;
    }
    return ret;
}
