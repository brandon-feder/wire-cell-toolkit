/**
   Convert ionization electrons into ADC.

   This class assumes fine-grained field responses calculated at
   impact positions along lines across the wire pitch for each plane.
 */

#ifndef WIRECELL_DETSIM
#define WIRECELL_DETSIM

#include "WireCellIface/IDetsim.h"
#include "WireCellIface/IConfigurable.h"

namespace WireCell {

    class Detsim : public IDetsim, public IConfigurable {
       public:
        Detsim(std::string wire_params,                    // name of the IWireParameters object
               std::string field_response_filename,        // name of a .json.gz file holding garfield field responses
               double time_origin = 0.0 * units::seconds,  // the absolute time from which depo times are interpreted.
               double tick = 0.5 * units::microseconds,    // digitization sample time
               double min_readout_time = 5 * units::milliseconds,  // will have at least this much time read out

               double binsize_l = 2.0 * units::millimeter, double time_offset = 0.0 * units::microsecond,
               double origin_l = 0.0 * units::microsecond, double DL = 5.3 * units::centimeter2 / units::second,
               double DT = 12.8 * units::centimeter2 / units::second,
               double drift_velocity = 1.6 * units::millimeter / units::microsecond,
               double max_sigma_l = 5 * units::microsecond, double nsigma = 3.0);

        // IQueuedoutNode.
        /// A frame will be returned once sufficient depos are collected (or EOS).
        virtual bool operator()(const input_pointer& depo, output_queue& outq_frames);
    };

}  // namespace WireCell

#endif
