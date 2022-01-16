/**
   Code here helps read in data which follows the Wire Cell Toolkit
   wire data schema.  See python module wirecell.util.wires.schema and
   sister submodules.

 */
#ifndef WIRECELLUTIL_WIREPLANES
#define WIRECELLUTIL_WIREPLANES

#include "WireCellUtil/Point.h"
#include "WireCellUtil/Exceptions.h"
#include "WireCellUtil/String.h"
#include "WireCellUtil/BoundingBox.h"
#include <memory>

namespace WireCell {

    namespace WireSchema {

        // IWire
        struct Wire {
            int ident;
            int channel;
            int segment;
            Point tail, head;  // end points, direction of signal to channel
        };

        struct Plane {
            int ident;
            std::vector<int> wires;
        };

        struct Face {
            int ident;
            std::vector<int> planes;
        };

        struct Anode {
            int ident;
            std::vector<int> faces;
        };

        struct Detector {
            int ident;
            std::vector<int> anodes;
        };

        struct StoreDB {
            std::vector<Detector> detectors;
            std::vector<Anode> anodes;
            std::vector<Face> faces;
            std::vector<Plane> planes;
            std::vector<Wire> wires;
        };

        // Access store via shared pointer to allow for caching of underlying data.
        typedef std::shared_ptr<const StoreDB> StoreDBPtr;

        // Bolt on some const functions to the underlying and shared store.
        class Store {
            StoreDBPtr m_db;

           public:
            Store();  // underlying store will be null!
            Store(StoreDBPtr db);
            Store(const Store& other);  // copy ctro
            Store& operator=(const Store& other);

            // Access underlying data store as shared pointer.
            StoreDBPtr db() const;

            const std::vector<Detector>& detectors() const;
            const std::vector<Anode>& anodes() const;
            const std::vector<Face>& faces() const;
            const std::vector<Plane>& planes() const;
            const std::vector<Wire>& wires() const;

            const Anode& anode(int ident) const;

            std::vector<Anode> anodes(const Detector& detector) const;
            std::vector<Face> faces(const Anode& anode) const;
            std::vector<Plane> planes(const Face& face) const;
            std::vector<Wire> wires(const Plane& plane) const;

            BoundingBox bounding_box(const Anode& anode) const;
            BoundingBox bounding_box(const Face& face) const;
            BoundingBox bounding_box(const Plane& plane) const;

            Ray wire_pitch(const Plane& plane) const;

            std::vector<int> channels(const Plane& plane) const;
        };

        Store load(const char* filename);
        // void dump(const char* filename, const Store& store);

    }  // namespace WireSchema

}  // namespace WireCell
#endif
