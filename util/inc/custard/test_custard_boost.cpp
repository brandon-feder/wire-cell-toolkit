#include "custard_boost.hpp"

int unpack(std::string archive)
{
    boost::iostreams::filtering_istream si;
    custard::input_filters(si, archive);

    if (!si) {
        std::cerr << "stream open error: " << strerror(errno) << std::endl;
        return -1;
    }

    assert(si.size() > 1);      // must have sink plus at least one filter

    std::cerr << "filtering istream for " << archive
              << " has " << si.size()-1 << " filters " << std::endl;

    while (true) {
        if (si.eof()) {
            std::cerr << "premature end of archive\n";
            return -1;
        }
        if (!si) {
            std::cerr << "ERROR stream broken: " << strerror(errno) << std::endl;
            return -1;
        }

        std::string fname{""};
        size_t fsize{0};
        std::cerr << "READ header\n";
        custard::read(si, fname, fsize);
        if (si.eof()) {
            std::cerr << "EOF in header, stream done!\n";
            break;
        }
        if (!si) {
            std::cerr << "ERROR reading header: " << strerror(errno) << std::endl;
            std::cerr << "FILE name:|"<<fname<<"| size:|"<<fsize<<"|\n";
            return -1;
        }

        std::cerr << "FILE: |" << fname << "|, size:|" << fsize << "|\n";

        std::string buf(fsize, 0);
        si.read(&buf[0], fsize);
        if (!si) {
            if (si.eof()) {
                std::cerr << "EOF unexpected in body: " << strerror(errno) << std::endl;
            }
            else{
                std::cerr << "ERROR read in body: " << strerror(errno) << std::endl;
            }
            return -1;
        }

        std::ofstream one(fname);
        one.write((char*)buf.data(), buf.size());
        one.flush();
    }

    return 0;
}


int pack(std::string archive, size_t nmembers, char* member[])
{
    boost::iostreams::filtering_ostream so;
    custard::output_filters(so, archive);

    if (!so) {
        std::cerr << "stream open error: " << strerror(errno) << std::endl;
        return -1;
    }
    assert(so.size() > 1);  // must have sink plus at least one filter

    std::cerr << "filtering ostream for " << archive
              << " has " << so.size()-1 << " filters " << std::endl;

    if (!so) {
        std::cerr << "stream open error: " << strerror(errno) << std::endl;
        return -1;
    }
    for (size_t ind = 0; ind<nmembers; ++ind) {
        std::string fname(member[ind]);

        std::ifstream si(fname, std::ifstream::ate | std::ifstream::binary);
        if (!si) {
            std::cerr << "No such file: " << fname << std::endl;
            continue;
        }
        auto siz = si.tellg();
        std::string buf(siz, 0);
        si.seekg(0);
        assert(si);
        si.read(&buf[0], siz);
        si.close();

        std::cerr << "HEAD: |" << fname << "| -> " << archive << " " << siz << std::endl;

        custard::write(so, fname, buf.size());
        if (!so) {
            std::cerr << "header write error: " << strerror(errno) << std::endl;
            return -1;
        }
        //std::cerr << "FLUSH\n";
        //so.flush();

        std::cerr << "FILE: |" << fname << "| -> " << archive << " " << siz << std::endl;

        so.write((char*)buf.data(), buf.size());
        if (!so) {
            std::cerr << "file write error: " << strerror(errno) << std::endl;
            return -1;
        }
        // std::cerr << "FLUSH\n";
        // so.flush();
    }

    so.pop();
    //out.pop();                  // remove the file to close.

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

