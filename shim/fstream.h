#pragma once
#include <fstream>
#include <string>

struct ios { enum { in = 1, binary = 2 }; };

// Old iostream-style fstream: DOS paths ("c:\\bitmap.bmp") map onto the virtual FS.
class fstream : public std::fstream
{
public:
    void open(const char* path, int)
    {
        std::string p(path);
        if (p.size() > 1 && p[1] == ':')
            p.erase(0, 2);
        for (auto& c : p)
            if (c == '\\')
                c = '/';
        std::fstream::open(p, std::ios::in | std::ios::binary);
    }
};
