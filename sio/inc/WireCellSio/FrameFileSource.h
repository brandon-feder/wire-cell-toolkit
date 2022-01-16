#ifndef WIRECELLSIO_FRAMEFILESOURCE
#define WIRECELLSIO_FRAMEFILESOURCE

#include "WireCellIface/IFrameSource.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellAux/Logger.h"

#include <boost/iostreams/filtering_stream.hpp>

#include <string>
#include <vector>

namespace WireCell::Sio {

    class FrameFileSource : public Aux::Logger, public IFrameSource, public IConfigurable {
    public:
        FrameFileSource();
        virtual ~FrameFileSource();

        virtual void configure(const WireCell::Configuration& cfg);
        virtual WireCell::Configuration default_configuration() const;
        
        virtual bool operator()(IFrame::pointer& frame);

    private:

        /// Configuration:
        /// 
        /// Input stream name should be a file name with .tar .tar,
        /// .tar.bz2 or .tar.gz.
        ///
        /// From it, individual files named following a fixed scheme
        /// will be streamed.
        ///
        /// Frames are read from the tar stream as Numpy .npy files.
        std::string m_inname;

        /// A set of tags to match against the .npy file names.  If
        /// the set is empty or a tag "*" is given then all tags are
        /// matched.
        std::vector<std::string> m_tags;

        // The output stream
        boost::iostreams::filtering_istream m_in;

        IFrame::pointer next();
        bool matches(const std::string& tag);

        size_t m_count{0};
        bool m_eos_sent{false};
    };        
     
}

#endif
