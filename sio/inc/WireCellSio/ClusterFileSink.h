#ifndef WIRECELLSIO_CLUSTERFILESINK
#define WIRECELLSIO_CLUSTERFILESINK

#include "WireCellIface/IClusterSink.h"
#include "WireCellIface/ITerminal.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellAux/Logger.h"

#include <boost/iostreams/filtering_stream.hpp>

#include <string>

namespace WireCell::Sio {

    /** Sink cluster related information to file.
     *
     * This component will save ICluster and referenced objects,
     * optionally including IFrame, to a stream which may terminate in
     * file or files or be forwarded over a network.
     */
    class ClusterFileSink : public Aux::Logger,
                            public IClusterSink, public ITerminal,
                            public IConfigurable
    {
    public:
        ClusterFileSink();
        virtual ~ClusterFileSink();

        virtual void finalize();

        virtual void configure(const WireCell::Configuration& cfg);
        virtual WireCell::Configuration default_configuration() const;
        
        virtual bool operator()(const ICluster::pointer& cluster);

    private:
        /// Configuration:
        ///
        /// Output stream name should be a file name with .tar .tar,
        /// .tar.bz2 or .tar.gz.
        ///
        /// In to it, individual files named following a fixed scheme
        /// will be streamed.
        ///
        /// Clusters are written as JSON files and frames as Numpy
        /// files.
        ///
        /// Future extension to writing to a directory of individual
        /// files or to a zeromq stream is expected and their outname
        /// formats are reserved.
        std::string m_outname{"cluster-file.tar.bz2"};

        /// If set to true, also output the referenced frames.
        // bool m_output_frame{"false"};

        /// Used to set X-axis blob point values based on drift time.
        double m_drift_speed;

        // The output stream
        boost::iostreams::filtering_ostream m_out;

        size_t m_count{0};
    };

}  // namespace WireCell
#endif
