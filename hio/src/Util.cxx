#include "WireCellHio/Util.h"

#include <mutex>
#include <thread>

using namespace WireCell;

namespace WireCell {
    namespace Hio {
        std::mutex g_h5cpp_mutex;
    }
}  // namespace WireCell