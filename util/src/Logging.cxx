#include "WireCellUtil/Logging.h"


#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/null_sink.h"

#include <vector>

using namespace WireCell;

static Log::logptr_t wct_base_logger()
{
    const std::string name = "wct";
    static Log::logptr_t base_logger = nullptr;
    if (base_logger) {
        return base_logger;
    }
    base_logger = spdlog::get(name);
    if (base_logger) {
        return base_logger;
    }

    std::vector<spdlog::sink_ptr> sv;
    base_logger = std::make_shared<spdlog::logger>(name, sv.begin(), sv.end());
    spdlog::register_logger(base_logger);
    base_logger->debug("create default logger \"wct\"");
    spdlog::set_default_logger(base_logger);

    return base_logger;
}

using sink_ptr = std::shared_ptr<spdlog::sinks::sink>;
using sink_maker_t = std::function<sink_ptr()>;
using sink_makers_t = std::vector<sink_maker_t>;
template<typename sinktype>
struct file_sink {
    std::string level;
    std::string fname;
    sink_ptr operator()() {
        auto s = std::make_shared<sinktype>(fname);
        if (!level.empty()) {
            s->set_level(spdlog::level::from_str(level));
        }
        return s;
    }
};
template<typename sinktype>
struct typed_sink {
    std::string level;
    sink_ptr operator()() {
        auto s = std::make_shared<sinktype>();
        if (!level.empty()) {
            s->set_level(spdlog::level::from_str(level));
        }
        return s;
    }
};
static sink_makers_t common_sink_makers;


// void Log::add_sink(Log::sinkptr_t sink, std::string level)
// {
//     if (!level.empty()) {
//         sink->set_level(spdlog::level::from_str(level));
//     }
//     wct_base_logger()->sinks().push_back(sink);
// }
void Log::add_file(std::string filename, std::string level)
{
    file_sink<spdlog::sinks::basic_file_sink_mt> m{level, filename};
    wct_base_logger()->sinks().push_back(m());
    common_sink_makers.push_back(m);
}

void Log::add_stdout(bool color, std::string level)
{
    if (color) {
        typed_sink<spdlog::sinks::stdout_color_sink_mt> m;
        wct_base_logger()->sinks().push_back(m());
        common_sink_makers.push_back(m);
        // auto s = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        // Log::add_sink(s, level);
    }
    else {
        typed_sink<spdlog::sinks::stdout_sink_mt> m;
        wct_base_logger()->sinks().push_back(m());
        common_sink_makers.push_back(m);
        // auto s = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        // Log::add_sink(s, level);
    }
}
void Log::add_stderr(bool color, std::string level)
{
    if (color) {
        typed_sink<spdlog::sinks::stderr_color_sink_mt> m;
        wct_base_logger()->sinks().push_back(m());
        common_sink_makers.push_back(m);
        // auto s = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        // Log::add_sink(s, level);
    }
    else {
        typed_sink<spdlog::sinks::stderr_sink_mt> m;
        wct_base_logger()->sinks().push_back(m());
        common_sink_makers.push_back(m);
        // auto s = std::make_shared<spdlog::sinks::stderr_sink_mt>();
        // Log::add_sink(s, level);
    }
}

Log::logptr_t Log::logger(std::string name, bool shared_sinks)
{
    wct_base_logger();  // make sure base logger is installed.
    auto l = spdlog::get(name);
    if (l) {
        return l;
    }

    // l = spdlog::default_logger()->clone(name);
    // return l;

    if (shared_sinks) {
        auto& sinks = wct_base_logger()->sinks();
        l = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
    }
    else {
        l = std::make_shared<spdlog::logger>(name);
        for (auto& csm : common_sink_makers) {
            auto s = csm();
            l->sinks().push_back(s);
        }
        // auto& sinks = make_sinks();
        // l = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
    }

    // peak under the hood of spdlog.  We want shared loggers to
    // get configured with the default level.
    try {
        spdlog::details::registry::instance().initialize_logger(l);
        spdlog::register_logger(l);
    }
    catch (spdlog::spdlog_ex& err) {
        return spdlog::get(name);
    }

    return l;
}

void Log::set_level(std::string level, std::string which)
{
    auto lvl = spdlog::level::from_str(level);

    if (which.empty()) {
        spdlog::set_level(lvl);
        return;
    }
    logger(which)->set_level(lvl);
}

void Log::set_pattern(std::string pattern, std::string which)
{
    if (which.empty()) {
        spdlog::set_pattern(pattern);
        return;
    }
    logger(which)->set_pattern(pattern);
}
