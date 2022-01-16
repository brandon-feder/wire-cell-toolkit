#include "WireCellUtil/WireSchema.h"
#include "WireCellUtil/Persist.h"
#include "WireCellUtil/Configuration.h"

using namespace WireCell;
using namespace WireCell::WireSchema;

static std::map<std::string, StoreDBPtr> gStoreCache;

Store WireCell::WireSchema::load(const char* filename)
{
    // turn into absolute real path
    std::string realpath = WireCell::Persist::resolve(filename);
    auto maybe = gStoreCache.find(realpath);
    if (maybe != gStoreCache.end()) {
        return Store(maybe->second);
    }

    Json::Value jtop = WireCell::Persist::load(filename);
    Json::Value jstore = jtop["Store"];

    StoreDB* store = new StoreDB;

    std::vector<Point> points;
    {
        Json::Value jpoints = jstore["points"];
        const int npoints = jpoints.size();
        points.resize(npoints);
        for (int ipoint = 0; ipoint < npoints; ++ipoint) {
            Json::Value jp = jpoints[ipoint]["Point"];
            points[ipoint].set(get<double>(jp, "x"), get<double>(jp, "y"), get<double>(jp, "z"));
        }
    }

    {  // wires
        Json::Value jwires = jstore["wires"];
        const int nwires = jwires.size();
        std::vector<Wire>& wires = store->wires;
        wires.resize(nwires);
        for (int iwire = 0; iwire < nwires; ++iwire) {
            Json::Value jwire = jwires[iwire]["Wire"];
            Wire& wire = wires[iwire];
            wire.ident = get<int>(jwire, "ident");
            wire.channel = get<int>(jwire, "channel");
            wire.segment = get<int>(jwire, "segment");

            int itail = get<int>(jwire, "tail");
            int ihead = get<int>(jwire, "head");
            wire.tail = points[itail];
            wire.head = points[ihead];
        }
    }

    {  // planes
        Json::Value jplanes = jstore["planes"];
        const int nplanes = jplanes.size();
        std::vector<Plane>& planes = store->planes;
        planes.resize(nplanes);
        for (int iplane = 0; iplane < nplanes; ++iplane) {
            Json::Value jplane = jplanes[iplane]["Plane"];
            Plane& plane = planes[iplane];
            plane.ident = get<int>(jplane, "ident");
            Json::Value jwires = jplane["wires"];
            const int nwires = jwires.size();
            plane.wires.resize(nwires);
            for (int iwire = 0; iwire < nwires; ++iwire) {
                plane.wires[iwire] = convert<int>(jwires[iwire]);
            }
        }
    }

    {  // faces
        Json::Value jfaces = jstore["faces"];
        const int nfaces = jfaces.size();
        std::vector<Face>& faces = store->faces;
        faces.resize(nfaces);
        for (int iface = 0; iface < nfaces; ++iface) {
            Json::Value jface = jfaces[iface]["Face"];
            Face& face = faces[iface];
            face.ident = get<int>(jface, "ident");
            Json::Value jplanes = jface["planes"];
            const int nplanes = jplanes.size();
            face.planes.resize(nplanes);
            for (int iplane = 0; iplane < nplanes; ++iplane) {
                face.planes[iplane] = convert<int>(jplanes[iplane]);
            }
        }
    }

    {  // anodes
        Json::Value janodes = jstore["anodes"];
        const int nanodes = janodes.size();
        std::vector<Anode>& anodes = store->anodes;
        anodes.resize(nanodes);
        for (int ianode = 0; ianode < nanodes; ++ianode) {
            Json::Value janode = janodes[ianode]["Anode"];
            Anode& anode = anodes[ianode];
            anode.ident = get<int>(janode, "ident");
            Json::Value jfaces = janode["faces"];
            const int nfaces = jfaces.size();
            anode.faces.resize(nfaces);
            for (int iface = 0; iface < nfaces; ++iface) {
                anode.faces[iface] = convert<int>(jfaces[iface]);
            }
        }
    }

    {  // detectors
        Json::Value jdets = jstore["detectors"];
        const int ndets = jdets.size();
        std::vector<Detector>& dets = store->detectors;
        dets.resize(ndets);
        for (int idet = 0; idet < ndets; ++idet) {
            Json::Value jdet = jdets[idet]["Detector"];
            Detector& det = dets[idet];
            det.ident = get<int>(jdet, "ident");
            Json::Value janodes = jdet["anodes"];
            const int nanodes = janodes.size();
            det.anodes.resize(nanodes);
            for (int ianode = 0; ianode < nanodes; ++ianode) {
                det.anodes[ianode] = convert<int>(janodes[ianode]);
            }
        }
    }
    gStoreCache[realpath] = StoreDBPtr(store);
    return Store(gStoreCache[realpath]);
}

// void WireCell::WireSchema::dump(const char* filename, const Store& store)
// {

// }

Store::Store()
  : m_db(nullptr)
{
}

Store::Store(StoreDBPtr db)
  : m_db(db)
{
}

Store::Store(const Store& other)
  : m_db(other.db())
{
}
Store& Store::operator=(const Store& other)
{
    m_db = other.db();
    return *this;
}

StoreDBPtr Store::db() const { return m_db; }

const std::vector<Detector>& Store::detectors() const { return m_db->detectors; }
const std::vector<Anode>& Store::anodes() const { return m_db->anodes; }
const std::vector<Face>& Store::faces() const { return m_db->faces; }
const std::vector<Plane>& Store::planes() const { return m_db->planes; }
const std::vector<Wire>& Store::wires() const { return m_db->wires; }

const Anode& Store::anode(int ident) const
{
    for (auto& a : m_db->anodes) {
        if (a.ident == ident) {
            return a;
        }
    }
    THROW(KeyError() << errmsg{String::format("Unknown anode: %d", ident)});
}

std::vector<Anode> Store::anodes(const Detector& detector) const
{
    std::vector<Anode> ret;
    for (auto ind : detector.anodes) {
        ret.push_back(m_db->anodes[ind]);
    }
    return ret;
}

std::vector<Face> Store::faces(const Anode& anode) const
{
    std::vector<Face> ret;
    for (auto ind : anode.faces) {
        ret.push_back(m_db->faces[ind]);
    }
    return ret;
}

std::vector<Plane> Store::planes(const Face& face) const
{
    std::vector<Plane> ret;
    for (auto ind : face.planes) {
        ret.push_back(m_db->planes[ind]);
    }
    return ret;
}

std::vector<Wire> Store::wires(const Plane& plane) const
{
    std::vector<Wire> ret;
    for (auto ind : plane.wires) {
        ret.push_back(m_db->wires[ind]);
    }
    return ret;
}

BoundingBox Store::bounding_box(const Anode& anode) const
{
    BoundingBox bb;
    for (const auto& face : faces(anode)) {
        bb(bounding_box(face).bounds());
    }
    return bb;
}
BoundingBox Store::bounding_box(const Face& face) const
{
    BoundingBox bb;
    for (const auto& plane : planes(face)) {
        bb(bounding_box(plane).bounds());
    }
    return bb;
}
BoundingBox Store::bounding_box(const Plane& plane) const
{
    BoundingBox bb;
    for (const auto& wire : wires(plane)) {
        Ray ray(wire.tail, wire.head);
        bb(ray);
    }
    return bb;
}

Ray Store::wire_pitch(const Plane& plane) const
{
    Vector wtot;
    for (const auto& wire : wires(plane)) {
        Ray ray(wire.tail, wire.head);
        wtot += ray_vector(ray);
    }
    wtot = wtot.norm();

    const Wire& w1 = m_db->wires[plane.wires.front()];
    const Wire& w2 = m_db->wires[plane.wires.back()];

    const Vector c1 = 0.5 * (w1.tail + w1.head);
    const Vector c2 = 0.5 * (w2.tail + w2.head);

    // approximate pitch, in the plane
    Vector pit = ray_vector(Ray(c1, c2));
    Vector ecks = wtot.cross(pit);  // X-axis
    pit = ecks.cross(wtot);
    return Ray(wtot, pit.norm());
}

std::vector<int> Store::channels(const Plane& plane) const
{
    std::vector<int> ret;
    for (const auto& wire : wires(plane)) {
        ret.push_back(wire.channel);
    }
    return ret;
}
