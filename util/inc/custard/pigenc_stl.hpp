#ifndef custard_pigenc_hpp
#define custard_pigenc_hpp

#include "pigenc.hpp"


namespace pigenc::stl {

    /// Dump a vector into a pigenc header object
    template<typename VectorType>
    void dump(pigenc::Header& pighead, const VectorType& vec)
    {
        using Scalar = typename VectorType::value_type;
        std::vector<size_t> shape;
        shape.push_back(vec.size());
        const bool fortran_order = false;
        pighead.set<Scalar>(shape, fortran_order);
    }

    /// Dump a vector into a pigenc file object
    template<typename VectorType>
    void dump(pigenc::File& pig, const VectorType& vec)
    {
        dump(pig.header(), vec);
        auto& dat = pig.data();
        dat.clear();
        char* adat = (char*)vec.data();
        dat.insert(dat.begin(), adat, adat+pig.header().data_size());
    }

    /// Write std::vector type as a pigenc stream (not custard!)
    template<typename VectorType>
    std::ostream& write(std::ostream& so, const VectorType& vec)
    {
        pigenc::File pig;
        dump<VectorType>(pig, vec);
        pig.write(so);
        return so;
    }


    // Load data from pigenc file to vector, return false on if
    // template type is a mismatch.
    template<typename VectorType>
    bool load(const pigenc::File& pig, VectorType& vec)
    {
        vec.clear();
        const pigenc::Header& head = pig.header();

        using Scalar = typename VectorType::value_type;
        const Scalar* dat = pig.as_type<Scalar>();
        if (!dat) { return false; }

        vec.insert(vec.begin(), dat, dat+head.array_size());
        return true;
    }


    /// Read std::vector from pigenc stream.  Stream will fail if
    /// VecType does not match dtype.  For a type-discovery mechanism,
    /// use a pigenc::File and then load<T>() above.
    template<typename VectorType>
    std::istream& read(std::istream& si, VectorType& vec)
    {
        pigenc::File pig;
        pig.read(si);
        if (!si) return si;

        if (load(pig, vec)) {
            return si;
        }
        si.setstate(std::ios::failbit);
        return si;
    }

    

}

#endif
