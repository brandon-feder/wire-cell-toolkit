#include "WireCellAux/TensorTools.h"
#include "WireCellAux/SimpleTensor.h"


#include <complex>
#include <vector>

using real_t = float;
using RV = std::vector<real_t>;
using complex_t = std::complex<real_t>;
using CV = std::vector<complex_t>;
using RT = WireCell::Aux::SimpleTensor<real_t>;
using CT = WireCell::Aux::SimpleTensor<complex_t>;

// test fodder
const RV real_vector{0,1,2,3,4,5};
const RV real_vector_cw{0,3,1,4,2,5};
const CV complex_vector{{0,0},{1,1},{2,2},{3,3},{4,4},{5,5}};
const WireCell::ITensor::shape_t shape{2,3};

using namespace WireCell;

void test_is_type()
{
    auto rt = std::make_shared<RT>(shape, real_vector.data());
    assert (Aux::is_type<real_t>(rt));
    assert (!Aux::is_type<complex_t>(rt));
}

void test_is_row_major()
{
    // ST actually does not let us do anything but C-order/row-major
    auto rm = std::make_shared<RT>(shape, real_vector.data());
    assert(Aux::is_row_major(rm));
}

template<typename VectorType>
void assert_equal(const VectorType& v1, const VectorType& v2)
{
    assert(v1.size() == v2.size());
    for (size_t ind=0; ind<v1.size(); ++ind) {
        assert(v1[ind] == v2[ind]);
    }
}

// asvec 1) match type, 2) type mismatch
void test_asvec()
{
    auto rt = std::make_shared<RT>(shape, real_vector.data());
    auto ct = std::make_shared<CT>(shape, complex_vector.data());
    auto got_rt = Aux::asvec<real_t>(rt);
    auto got_ct = Aux::asvec<complex_t>(ct);
    assert_equal(real_vector, got_rt);
    assert_equal(complex_vector, got_ct);

    try {
        auto oops = Aux::asvec<complex_t>(rt);
    }
    catch (ValueError& err) {
    }
}

void test_asarray()
{
    // as array 2x2: (1d,2d) x (rw,cw)

    // make mutable copy to test that TT returns a copy
    RV my_vec(real_vector.begin(), real_vector.end());

    // test 2d
    auto rt = std::make_shared<RT>(shape, my_vec.data());
    auto ra = Aux::asarray<real_t>(rt);
    auto shape = rt->shape();
    for (size_t irow = 0; irow < shape[0]; ++irow) {
        for (size_t icol = 0; icol < shape[1]; ++icol) {        
            assert(ra(irow, icol) == my_vec[irow*shape[1] + icol]);
        }
    }

    // test 1d
    const WireCell::ITensor::shape_t shape1d{6,};
    auto rt1d = std::make_shared<RT>(shape1d, my_vec.data());
    auto ra1d = Aux::asarray<real_t>(rt1d);
    for (size_t ind = 0; ind < shape[0]; ++ind) {        
        assert(ra1d(ind) == my_vec[ind]);
    }

    // Assure the internal use of Eigen::Map leads to a copy on return
    my_vec[0] = 42;
    assert(ra(0,0) == 0);
    assert(ra1d(0) == 0);
}

int main()
{
    test_is_type();
    test_is_row_major();
    test_asvec();
    test_asarray();

    return 0;
}
