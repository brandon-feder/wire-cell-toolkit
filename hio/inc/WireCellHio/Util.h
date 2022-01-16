/** A wrapper for h5cpp calls
 */

#ifndef WIRECELLHIO_UTIL
#define WIRECELLHIO_UTIL

#include <mutex>
#include <thread>
#include <chrono>

#include <h5cpp/all>

namespace WireCell {
    namespace Hio {

        extern std::mutex g_h5cpp_mutex;

    };  // namespace Hio
}  // namespace WireCell

#endif  // WIRECELLHIO_UTIL