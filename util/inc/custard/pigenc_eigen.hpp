/**
   Serialize Eigen with pigenc
 */
#ifndef pigenc_eigen_hpp
#define pigenc_eigen_hpp

#include "pigenc.hpp"
#include "Eigen/Core"

namespace pigenc::eigen {


    /// Dump an array into a pigenc header object
    template<typename ArrayType>
    void dump(pigenc::Header& pighead, const ArrayType& array)
    {
        using Scalar = typename ArrayType::Scalar;
        std::vector<size_t> shape;
        shape.push_back(array.rows());
        shape.push_back(array.cols());
        const bool fortran_order = !(ArrayType::Flags & Eigen::RowMajorBit);
        pighead.set<Scalar>(shape, fortran_order);
    }

    /// Dump an array into a pigenc file object
    template<typename ArrayType>
    void dump(pigenc::File& pig, const ArrayType& array)
    {
        dump(pig.header(), array);
        auto& dat = pig.data();
        dat.clear();
        char* adat = (char*)array.data();
        dat.insert(dat.begin(), adat, adat+pig.header().data_size());
    }

    /// Write Eigen3 array as a pigenc stream (not custard!)
    template<typename ArrayType>
    std::ostream& write(std::ostream& so, const ArrayType& array)
    {
        pigenc::File pig;
        dump<ArrayType>(pig, array);
        pig.write(so);
        return so;
    }

    // Load data from pigenc file to array, return false on if
    // template type is a mismatch.
    template<typename ArrayType>
    bool load(const pigenc::File& pig, ArrayType& array)
    {
        const pigenc::Header& head = pig.header();

        using Scalar = typename ArrayType::Scalar;
        const Scalar* dat = pig.as_type<Scalar>();
        if (!dat) { return false; }

        const auto& shape = head.shape();
        if (head.fortran_order()) {
            using COLM = Eigen::Array<Scalar, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>;
            const COLM temp = Eigen::Map<const COLM>(dat,shape[0],shape[1]);
            array = temp;
        }
        else {
            using ROWM = Eigen::Array<Scalar, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
            const ROWM temp = Eigen::Map<const ROWM>(dat, shape[0], shape[1]);
            array = temp;
        }
        return true;
    }


    /// Read Eigen3 from pigenc stream.  Stream will fail if ArrayType
    /// does not match dtype.  For a type-discovery mechanism, use a
    /// pigenc::File and then load<T>() above.
    template<typename ArrayType>
    std::istream& read(std::istream& si, ArrayType& array)
    {
        pigenc::File pig;
        pig.read(si);
        if (!si) return si;

        if (load(pig, array)) {
            return si;
        }
        si.setstate(std::ios::failbit);
        return si;
    }
    
    
}

#endif
