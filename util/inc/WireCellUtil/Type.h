#ifndef WIRECELL_TYPE
#define WIRECELL_TYPE

// Some type utilities

#include <string>
#include <typeinfo>

namespace WireCell {

    // Warning! do not rely on the actual format of demangle() and
    // type() to be consistent across compilers!  It's only for adding
    // pretty-printing niceties for those that happen to be supported.

    // http://stackoverflow.com/a/4541470

    std::string demangle(const std::string& name);

    template <class T>
    std::string type(const T& t)
    {
        return demangle(typeid(t).name());
    }

}  // namespace WireCell

#endif
