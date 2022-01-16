#include "WireCellUtil/Type.h"

#include <boost/core/demangle.hpp>

std::string WireCell::demangle(const std::string& name)
{
    std::string ret = boost::core::demangle(name.c_str());
    return ret;
}
