/**  This component "taps" into a stream of clusters to save them as JSON files.
 *
 *  The JSON can can then be converted for use with various viewers (paraview, bee).
 *
 * The schema of JSON file is one that reflects the graph nature of an ICluster.
 * It has two top level attributes:
 *
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

#ifndef WIRECELLIMG_JSONCLUSTERTAP
#define WIRECELLIMG_JSONCLUSTERTAP

#include "WireCellIface/IClusterFilter.h"
#include "WireCellIface/IConfigurable.h"

#include "WireCellUtil/Logging.h"

#include <string>

namespace WireCell {

    namespace Img {

        class JsonClusterTap : public IClusterFilter, public IConfigurable {
           public:
            JsonClusterTap();
            virtual ~JsonClusterTap();

            virtual void configure(const WireCell::Configuration& cfg);
            virtual WireCell::Configuration default_configuration() const;

            virtual bool operator()(const input_pointer& in, output_pointer& out);

           private:
            std::string m_filename;
            double m_drift_speed;

            Log::logptr_t log;
        };
    }  // namespace Img
}  // namespace WireCell

#endif
