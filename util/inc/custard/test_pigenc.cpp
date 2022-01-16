#include "pigenc.hpp"

#include <fstream>
#include <iostream>

void test_write_low(std::string npyfile)
{
    std::vector<size_t> shape{2,2};
    auto head = pigenc::make_header("<i4", shape);
    std::cerr << head << std::endl;    
    
    std::ofstream out(npyfile);
    assert(out);

    int data[4] = {0,1,2,3};
    pigenc::write_data(out, data, {2,2});
    assert(out);
}

void test_read_low(std::string npyfile)
{

    std::ifstream fstr(npyfile);
    assert(fstr);
    auto head = pigenc::read_header(fstr);
    std::cerr << head.dump(4) << std::endl;
    assert(fstr);

    size_t size=1;
    for (auto s : head["shape"]) {
        int one = s;
        size *= one;
    }
    assert (size == 4);
    std::vector<int> dat(size,0);
    
    fstr.read((char*)dat.data(), dat.size()*sizeof(int));
    for (int ind=0; ind<4; ++ind) {
        std::cerr << ind << ": " << dat[ind] << std::endl;
    }
    assert(fstr);
}
    
void test_write_high(std::string npyfile)
{
    pigenc::Header head;

    head.set<int>({2,2});
    std::cerr << head.str() << std::endl;    
    
    std::ofstream out(npyfile);
    head.write(out);
    assert(out);
    int arr[4] = {0,1,2,3};
    out.write((char*)arr, sizeof(int) * 4);
    assert(out);
}

void test_read_high(std::string npyfile)
{
    pigenc::Header head;
    std::ifstream si(npyfile);
    assert(si);
    head.read(si);
    assert(si);
    std::vector<int> dat(head.array_size(), 0);
    si.read((char*)dat.data(), head.data_size());
    assert(si);
    for (int ind=0; ind<4; ++ind) {
        std::cerr << ind << ": " << dat[ind] << std::endl;
    }
}

int main(int argc, char* argv[])
{

    std::string npyfile = "test_pigenc.npy";
    if (argc > 1) {
        npyfile = argv[1];
    }

    assert(pigenc::dtype<int>() == "<i4");
    assert(pigenc::dtype<short>() == "<i2");
    assert(pigenc::dtype<int8_t>() == "<i1");
    assert(pigenc::dtype<char>() == "c");
    assert(pigenc::dtype<float>() == "<f4");
    assert(pigenc::dtype<double>() == "<f8");

    test_write_low(npyfile);
    test_read_low(npyfile);

    test_write_high(npyfile);
    test_read_high(npyfile);


    return 0;
}

