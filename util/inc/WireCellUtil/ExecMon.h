#ifndef WIRECELLUTIL_EXECMON
#define WIRECELLUTIL_EXECMON

#include "WireCellUtil/TimeKeeper.h"
#include "WireCellUtil/MemUsage.h"

namespace WireCell {

    /** A helper class combining a TimeKeeper and a MemUsage
     *
     * Use like
     *
     *   ExecMon em("starting");
     *   ...
     *   em("starting long calculation....");
     *   do_long_calculation();
     *   em("...done");
     *   ...
     *   info(em.summary());
     */
    class ExecMon {
       public:
        ExecMon(const std::string& msg = "start",
                TimeKeeper::ptime starting_time = boost::posix_time::microsec_clock::local_time());
        ~ExecMon();

        /// Record an event.
        std::string operator()(std::string msg = "<tick>",
                               TimeKeeper::ptime now = boost::posix_time::microsec_clock::local_time(),
                               MemUsage::memusage mu = MemUsage::memusage(-1, -1));

        /// Return summary up to now.
        std::string summary() const;

        TimeKeeper tk;
        MemUsage mu;
    };
}  // namespace WireCell
#endif
