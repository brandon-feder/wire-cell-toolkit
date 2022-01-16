// Provide some helper functions for working with ICluster

#ifndef WIRECELLAUX_CLUSTERJSONIFY
#define WIRECELLAUX_CLUSTERJSONIFY

#include "WireCellUtil/Configuration.h"
#include "WireCellIface/IFrame.h"
#include "WireCellIface/ISlice.h"
#include "WireCellIface/ICluster.h"

#include <string>

namespace WireCell::Aux {
    /// Return JSON representation of the cluster.
    Json::Value jsonify(const ICluster::pointer& cluster,
                         double drift_speed);

    /// Return name for the cluster in a canonical form suitable for
    /// use as a file name.  It only includes info related directly to
    /// the cluster (the ident).
    std::string name(const ICluster::pointer& cluster);

    /// Return the slices in the cluster.
    ISlice::vector find_slices(const ICluster::pointer& cluster);

    /// Return the frame of the first slice in the cluster.  Note, in
    /// principle, clusters can span frames.
    IFrame::pointer find_frame(const ICluster::pointer& cluster);

}

#endif

