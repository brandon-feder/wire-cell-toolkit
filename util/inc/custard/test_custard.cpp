// test custard.hpp
//
// Beware, the code here would NOT be ideal for a real app 

#include "custard.hpp"
#include <fstream>
#include <string>



int unpack(std::string archive)
{
    std::cerr << "unpacking: " << archive << std::endl;
    std::ifstream fi(archive);
    while (fi) {
        custard::Header head;
        fi.read(head.as_bytes(), 512);
        if (fi.eof()) {
            return 0;
        }
        assert (fi);

        if (! head.size()) {
            std::cerr << "skipping empty\n";
            continue;
        }

        std::cerr << head.name() << "\n"
                  << "stored check sum: " << head.chksum() << "\n"
                  << "  calculated sum: " << head.checksum() << "\n"
                  << "       file size: " << head.size() 
                  << "\n";

        assert(head.chksum() == head.checksum());

        std::string path = head.name();
        std::cerr << archive << " -> " << path << std::endl;
        while (path[0] == '/') {
            // At leat pretend to be a little secure
            path.erase(path.begin());
        }
        std::ofstream fo(path);
        assert (fo);

        // This is NOT smart on large files!
        std::string buf(head.size(), 0);
        fi.read((char*)buf.data(), buf.size());
        assert (fi);
        fo.write(buf.data(), buf.size());
        assert (fo);

        // get past padding
        size_t npad = head.padding();
        std::cerr << head.name() << " skipping " << npad << " after " << head.size() << std::endl;
        fi.seekg(npad, fi.cur);
        assert (fi);

    }
    return 0;
}

int pack(std::string archive, int nmembers, char* member[])
{
    std::ofstream fo(archive);
    for (int ind = 0; ind<nmembers; ++ind) {
        std::string path(member[ind]);

        std::ifstream fi(path, std::ifstream::ate | std::ifstream::binary);        
        assert (fi);

        auto siz = fi.tellg();
        fi.seekg(0);
        assert (fi);

        // note, real tar preserves mtime, uid, etc.
        custard::Header head(path, siz);

        fo.write(head.as_bytes(), 512);
        assert (fo);

        std::string buf(head.size(), 0);
        fi.read((char*)buf.data(), buf.size());
        assert (fi);

        fo.write(buf.data(), buf.size());
        assert (fo);

        size_t npad = 512 - head.size() % 512;
        std::string pad(npad, 0);
        fo.write(pad.data(), pad.size());
        assert (fo);
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " archive.tar [file ...]" << std::endl;
        std::cerr << " with no files, extract archive.tar, otherwise produce it\n";
        return -1;
    }

    if (argc == 2) {
        return unpack(argv[1]);
    }

    return pack(argv[1], argc-2, argv+2);
}

