#include "WireCellAux/ClusterHelpers.h"

#include "WireCellIface/IChannel.h"
#include "WireCellIface/IAnodeFace.h"
#include "WireCellIface/IWire.h"
#include "WireCellIface/IBlob.h"
#include "WireCellIface/ISlice.h"
#include "WireCellIface/IFrame.h"

#include "WireCellUtil/Units.h"
#include "WireCellUtil/String.h"

using namespace WireCell;

static Json::Value size_stringer(const cluster_node_t& n) { return Json::nullValue; }

static Json::Value jpoint(const Point& p)
{
    Json::Value ret;
    for (int ind = 0; ind < 3; ++ind) {
        ret[ind] = p[ind];
    }
    return ret;
}

static Json::Value channel_jsoner(const cluster_node_t& n)
{
    IChannel::pointer ich = std::get<typename IChannel::pointer>(n.ptr);
    Json::Value ret = Json::objectValue;
    ret["ident"] = ich->ident();
    ret["index"] = ich->index();
    ret["wpid"] = ich->planeid().ident();
    return ret;
}

static Json::Value wire_jsoner(const cluster_node_t& n)
{
    IWire::pointer iwire = std::get<typename IWire::pointer>(n.ptr);
    Json::Value ret = Json::objectValue;
    ret["ident"] = iwire->ident();
    ret["index"] = iwire->index();
    ret["wpid"] = iwire->planeid().ident();
    ret["chid"] = iwire->channel();
    ret["seg"] = iwire->segment();
    auto r = iwire->ray();
    ret["tail"] = jpoint(r.first);
    ret["head"] = jpoint(r.second);
    return ret;
}

// this one needs extra info
struct blob_jsoner {
    double drift_speed;

    blob_jsoner(double ds)
      : drift_speed(ds)
    {
    }

    Json::Value operator()(const cluster_node_t& n)
    {
        IBlob::pointer iblob = std::get<typename IBlob::pointer>(n.ptr);
        IAnodeFace::pointer iface = iblob->face();
        IWirePlane::vector iplanes = iface->planes();
        IWire::pointer iwire = iplanes[2]->wires()[0];
        const double xplane = iwire->center().x();

        // fixme: this is not a particularly portable way to go for all wire geometries.
        const double xsign = iface->ident() == 0 ? 1.0 : -1.0;

        ISlice::pointer islice = iblob->slice();
        // set X based on time with knowledge of local drift
        // direction as given by the face.
        double x0 = xplane + xsign * (islice->start() - 250.0 * units::us) * drift_speed;  // FIXME: -250us hardcoded

        const auto& coords = iface->raygrid();

        Json::Value ret = Json::objectValue;
        ret["span"] = islice->span() * drift_speed;
        ret["ident"] = iblob->ident();
        ret["value"] = iblob->value();
        ret["error"] = iblob->uncertainty();
        ret["faceid"] = iblob->face()->ident();
        ret["sliceid"] = iblob->slice()->ident();
        Json::Value jcorners = Json::arrayValue;
        const auto& blob = iblob->shape();
        for (const auto& c : blob.corners()) {
            Json::Value j = jpoint(coords.ray_crossing(c.first, c.second));
            j[0] = x0;
            jcorners.append(j);
        }
        ret["corners"] = jcorners;

        //-- strip info is redunant with edges connected to wire vertices.
        // Json::Value jstrips = Json::arrayValue;
        // for (const auto& strip : blob.strips()) {
        //     int plane_ind = strip.layer - 2; // fixme
        //     if (plane_ind <0) { continue; }
        //     Json ::Value j = Json::objectValue;
        //     j["wpid"] = iplanes[plane_ind]->planeid().ident();
        //     j["planeind"] = plane_ind;
        //     j["wip1"] = strip.bounds.first;
        //     j["wip2"] = strip.bounds.second;
        //     Json::Value jwires = Json::arrayValue;
        //     const auto& wires = iplanes[plane_ind]->wires();
        //     for (auto wip = strip.bounds.first; wip < strip.bounds.second; ++wip) {
        //         jwires.append(wires[wip]->ident());
        //     }
        //     j["wids"] = jwires;
        //     jstrips.append(j);
        // }
        // ret["strips"] = jstrips;

        return ret;
    }
};

static
Json::Value slice_jsoner(const cluster_node_t& n)
{
    ISlice::pointer islice = std::get<typename ISlice::pointer>(n.ptr);
    Json::Value ret = Json::objectValue;

    ret["ident"] = islice->ident();
    ret["frameid"] = islice->frame()->ident();
    ret["start"] = islice->start();
    ret["span"] = islice->span();
    Json::Value jact = Json::objectValue;
    for (const auto& it : islice->activity()) {
        jact[String::format("%d", it.first->ident())] = it.second;
    }
    ret["activity"] = jact;
    return ret;
}

static
Json::Value measurement_jsoner(const cluster_node_t& n)
{
    IChannel::shared_vector ichv = std::get<IChannel::shared_vector>(n.ptr);
    Json::Value ret = Json::objectValue;
    Json::Value j = Json::arrayValue;
    for (auto ich : *ichv) {
        j.append(ich->ident());
        ret["wpid"] = ich->planeid().ident();  // last one wins, but they should all be the same
    }
    ret["chids"] = j;
    return ret;
}

Json::Value WireCell::Aux::jsonify(const ICluster::pointer& cluster,
                                   double drift_speed)
{
    std::vector<std::function<Json::Value(const cluster_node_t& ptr)> > jsoners{
        size_stringer, channel_jsoner, wire_jsoner, blob_jsoner(drift_speed), slice_jsoner, measurement_jsoner};
    auto asjson = [&](const cluster_node_t& n) { return jsoners[n.ptr.index()](n); };

    /*
     * - vertices :: a list of graph vertices
     * - edges :: a list of graph edges
     *
     * A vertex is represented as a JSON object with the following attributes
     * - ident :: an indexable ID number for the node, and referred to in "edges"
     * - type :: the letter "code" used in ICluster: one in "sbcwm"
     * - data :: an object holding information about the corresponding vertex object
     *
     * An edge is a pair of vertex ident numbers.
     *
     */
    const auto& gr = cluster->graph();

    Json::Value jvertices = Json::arrayValue;
    for (auto vtx : boost::make_iterator_range(boost::vertices(gr))) {
        const auto& vobj = gr[vtx];
        if (!vobj.ptr.index()) {
            // warn?
            continue;
        }
        Json::Value jvtx = Json::objectValue;
        jvtx["ident"] = (int) vtx;
        jvtx["type"] = String::format("%c", vobj.code());
        jvtx["data"] = asjson(vobj);

        jvertices.append(jvtx);
    }

    Json::Value jedges = Json::arrayValue;
    for (auto eit : boost::make_iterator_range(boost::edges(gr))) {
        Json::Value jedge = Json::arrayValue;
        jedge[0] = (int) boost::source(eit, gr);
        jedge[1] = (int) boost::target(eit, gr);
        jedges.append(jedge);
    }

    Json::Value top = Json::objectValue;
    top["vertices"] = jvertices;
    top["edges"] = jedges;

    return top;
}

// maybe useful to export
ISlice::vector WireCell::Aux::find_slices(const ICluster::pointer& cluster)
{
    ISlice::vector ret;
    const auto& gr = cluster->graph();
    for (auto vtx : boost::make_iterator_range(boost::vertices(gr))) {
        const auto& vobj = gr[vtx];
        if (vobj.ptr.index() != 4) {
            continue;
        }

        ISlice::pointer islice = std::get<typename ISlice::pointer>(vobj.ptr);
        ret.push_back(islice);
    }
    return ret;
}

IFrame::pointer WireCell::Aux::find_frame(const ICluster::pointer& cluster)
{
    const auto& gr = cluster->graph();
    for (auto vtx : boost::make_iterator_range(boost::vertices(gr))) {
        const auto& vobj = gr[vtx];
        if (vobj.ptr.index() != 4) {
            continue;
        }

        ISlice::pointer islice = std::get<typename ISlice::pointer>(vobj.ptr);
        return islice->frame();
    }
    return nullptr;
}

std::string WireCell::Aux::name(const ICluster::pointer& cluster)
{
    std::stringstream ss;
    ss << "cluster_" << cluster->ident();
    return ss.str();
}
