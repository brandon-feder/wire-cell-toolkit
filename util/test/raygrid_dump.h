
#include <json/json.h>

class JsonEvent {
    const WireCell::RayGrid::Coordinates& coords;
    Json::Value blobs, points;

   public:
    JsonEvent(const WireCell::RayGrid::Coordinates& coords)
      : coords(coords)
    {
    }

    void operator()(const Point& pt, double charge = 1.0)
    {
        Json::Value jpt;
        jpt[0] = pt.x();
        jpt[1] = pt.y();
        jpt[2] = pt.z();
        Json::Value jpv;
        jpv["point"] = jpt;
        jpv["values"]["charge"] = charge;
        points.append(jpv);
    }

    Json::Value convert(const WireCell::RayGrid::crossing_t& corner, double x)
    {
        Json::Value jcorner = Json::arrayValue;
        auto pt = coords.ray_crossing(corner.first, corner.second);
        jcorner.append(x);
        jcorner.append(pt.y());
        jcorner.append(pt.z());
        return jcorner;
    }

    Json::Value convert(const WireCell::RayGrid::crossings_t& corners, double x)
    {
        Json::Value jcorners = Json::arrayValue;
        for (const auto& corner : corners) {
            jcorners.append(convert(corner, x));
        }
        return jcorners;
    }

    void operator()(const WireCell::RayGrid::Blob& blob, double x, double charge = 1.0, int slice = 1, int number = 0)
    {
        Json::Value jblob;
        jblob["points"] = convert(blob.corners(), x);
        jblob["values"]["charge"] = charge;
        jblob["values"]["slice"] = slice;
        jblob["values"]["number"] = number;
        blobs.append(jblob);
    }

    void dump(const std::string& filename)
    {
        Json::Value top;
        top["points"] = points;
        top["blobs"] = blobs;
        ofstream fstr(filename);
        fstr << top;
    }
};
