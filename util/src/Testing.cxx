#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Exceptions.h"
#include "WireCellUtil/Logging.h"

#include <sstream>

using namespace WireCell;

void boost::assertion_failed_msg(char const* expr, char const* msg, char const* function, char const* file, long line)
{
    spdlog::critical("{}:{}:{} {} {}", file, function, line, expr, msg or "");

    std::stringstream ss;
    ss << "WireCell::AssertionError: \"" << expr << "\" in " << function << " " << file << ":" << line;
    if (msg and msg[0]) {
        ss << "\n" << msg;
    }
    THROW(AssertionError() << errmsg{ss.str()});
}

void boost::assertion_failed(char const* expr, char const* function, char const* file, long line)
{
    boost::assertion_failed_msg(expr, "", function, file, line);
}

void Testing::log(const char* argv0)
{
    std::string name = argv0;
    name += ".log";
    Log::add_stderr(true, "trace");
    Log::add_file(name, "trace");
}
