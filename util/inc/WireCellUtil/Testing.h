#ifndef WIRECELLUTIL_TESTING
#define WIRECELLUTIL_TESTING

#define BOOST_ENABLE_ASSERT_HANDLER 1
#include <boost/assert.hpp>

#define Assert BOOST_ASSERT
#define AssertMsg BOOST_ASSERT_MSG

namespace boost {
    void assertion_failed(char const* expr, char const* function, char const* file, long line);
    void assertion_failed_msg(char const* expr, char const* msg, char const* function, char const* file, long line);
}  // namespace boost

namespace WireCell {
    namespace Testing {
        void log(const char* argv0);
    }
}  // namespace WireCell

#endif
