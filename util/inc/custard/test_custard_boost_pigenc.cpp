// This test works in the same two modes as, eg test_custard_boost.
//
// pack: first arg is a tar file, remaining are .npy files
// unpack: one arg is a tar file, files will be unpacked.
//
// This test differs from test_custard_boost in that it will use
// pigenc twice for either mode.  Where .npy files are read or written
// then base pigenc is used.  Where a tar stream is used then
// custard_pigenc is used.
//

#include <boost/iostreams/filtering_stream.hpp>

#include "custard_boost.hpp"
#include "pigenc_eigen.hpp"
#include "pigenc_stl.hpp"

template<typename Scalar>
bool rt(const pigenc::File& inpig, pigenc::File& outpig)
{
    if (inpig.header().shape().size() == 2) {
        using ArrayType = Eigen::Array<Scalar, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
        ArrayType array;
        bool ok = pigenc::eigen::load<ArrayType>(inpig, array);
        if (!ok) {
            std::cerr << "fail to load array from " << inpig.header().dtype() << "\n";
            return false;
        }
        pigenc::eigen::dump<ArrayType>(outpig, array);
        return true;
    }
    // 1D, test with std::vector
    using VectorType = std::vector<Scalar>;
    VectorType vec;
    bool ok = pigenc::stl::load<VectorType>(inpig, vec);
    if (!ok) {
        std::cerr << "fail to load vector from " << inpig.header().dtype() << "\n";
        return false;
    }
    pigenc::stl::dump<VectorType>(outpig, vec);
    return true;
}

static
bool round_tripper(const pigenc::File& inpig, pigenc::File& outpig)
{
    std::string dtype = inpig.header().dtype();;
    std::cerr << "round trip with type " << dtype << std::endl;
    if (dtype == "c")   { return rt<char>(inpig, outpig); }
    if (dtype == "<i1") { return rt<int8_t>(inpig, outpig); }
    if (dtype == "<u1") { return rt<uint8_t>(inpig, outpig); }
    if (dtype == "<i2") { return rt<int16_t>(inpig, outpig); }
    if (dtype == "<u2") { return rt<uint16_t>(inpig, outpig); }
    if (dtype == "<i4") { return rt<int32_t>(inpig, outpig); }
    if (dtype == "<u4") { return rt<uint32_t>(inpig, outpig); }
    if (dtype == "<i8") { return rt<int64_t>(inpig, outpig); }
    if (dtype == "<u8") { return rt<uint64_t>(inpig, outpig); }
    if (dtype == "<f4") { return rt<float>(inpig, outpig); }
    if (dtype == "<f8") { return rt<double>(inpig, outpig); }

    std::cerr << "unsupported dtype: " << dtype << std::endl;
    return false;
}

static
int unpack(std::string archive, bool round_trip = true)
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
            std::cerr << "ERROR stream broken at start: " << strerror(errno) << std::endl;
            return -1;
        }

        std::string fname;
        size_t fsize;
        custard::read(si, fname, fsize);
        if (si.eof()) {
            std::cerr << "EOF on custard head read\n";
            break;
        }
        if (!si) {
            std::cerr << "ERROR unpacking tar header " << strerror(errno) << std::endl;
            return -1;
        }
        std::cerr << "Unpacking " << fname << " " << fsize << std::endl;
        
        pigenc::File inpig;
        inpig.read(si);
        if (!si) {
            std::cerr << "ERROR unpacking pig " << strerror(errno) << std::endl;
            return -1;
        }
        std::cerr << "Read in " << fname << std::endl;

        pigenc::File outpig;        
        if (round_trip) {
            bool ok = round_tripper(inpig, outpig);
            if (!ok) {
                std::cerr << "ERROR round trip failure\n";
                return -1;
            }
            std::cerr << "Round tripped " << fname << std::endl;
        }
        else {
            outpig = inpig;
        }
        
        std::ofstream so(fname);
        if (!so) {
            std::cerr << "ERROR open output file " << fname
                      << " " << strerror(errno) << std::endl;
            return -1;
        }
        std::cerr << "Open for writing: " << fname << std::endl;

        outpig.write(so);
        if (!so) {
            std::cerr << "ERROR writing pig " << strerror(errno) << std::endl;
            return -1;
        }
        std::cerr << "Wrote: " << fname << std::endl;

    }

    return 0;
    
}

static
int pack(std::string archive, size_t nmembers, char* member[], bool round_trip = true)
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
        std::cerr << "packing " << fname << std::endl;

        pigenc::File inpig, outpig;
        std::ifstream si(fname);
        if (!si) {
            std::cerr << "No such file: " << fname << std::endl;
            continue;
        }
        std::cerr << "reading " << fname << std::endl;
        inpig.read(si);
        if (!si) {
            std::cerr << "failed to open " << fname << " as pigenc\n";
            continue;
        }

        if (round_trip) {
            std::cerr << "round trip " << fname << std::endl;
            round_tripper(inpig, outpig);
        }
        else {
            outpig = inpig;
        }
        
        size_t fsize = outpig.header().file_size();
        custard::write(so, fname, fsize);
        if (!so) {
            std::cerr << "ERROR in writing tar header: " << strerror(errno) << std::endl;
            return -1;
        }

        std::cerr << "Writing body for " << fname << std::endl;
        outpig.write(so);
        if (!so) {
            std::cerr << "ERROR in writing tar header: " << strerror(errno) << std::endl;
            return -1;
        }

        so.flush();
    }
    so.pop();
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
      
