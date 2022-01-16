#ifndef WIRECELLIFACE_IFRAME
#define WIRECELLIFACE_IFRAME

#include "WireCellIface/ITrace.h"
#include "WireCellIface/IWirePlane.h"
#include "WireCellIface/WirePlaneId.h"
#include "WireCellUtil/Waveform.h"

#include <string>
#include <vector>

namespace WireCell {

    /** Interface to a sequence of traces.
     *
     */
    class IFrame : public IData<IFrame> {
       public:
        virtual ~IFrame();

        // Tag gives some semantic meaning to that which it is
        // associated.  The frame as a whole may have a number of tags
        // as can a subset of traces.  Although they are free-form
        // they are best kept limited in number, single words of lower
        // case letters.  Like all data they are immutable after
        // creation.
        typedef std::string tag_t;

        // A frame and a subset of traces may have more than one tag.
        // Such tags are grouped into a tag list.  The order of the
        // tag list may or may not have meaning.
        typedef std::vector<tag_t> tag_list_t;

        // A list of indices into the traces vector.
        typedef std::vector<size_t> trace_list_t;

        // A numerical summary value defined over a number of traces.
        typedef std::vector<double> trace_summary_t;

        // The list of tags applied to this frame as a whole.
        virtual const tag_list_t& frame_tags() const = 0;

        // The union of all tags applied to all traces of this frame.
        virtual const tag_list_t& trace_tags() const = 0;

        // Return traces (as a list of their indices) with the given
        // tag applied.
        virtual const trace_list_t& tagged_traces(const tag_t& tag) const = 0;

        // Return a trace summary defined over the tagged subset of
        // traces.  The values of the summary correspond to the
        // tagged trace list as returned by `tagged_traces(tag)`.
        virtual const trace_summary_t& trace_summary(const tag_t& tag) const = 0;

        /// Return a vector of all traces ignoring any potential tag.
        virtual ITrace::shared_vector traces() const = 0;

        /// Return all masks associated with this frame
        // fixme: this should be its own interface
        virtual Waveform::ChannelMaskMap masks() const
        {
            return Waveform::ChannelMaskMap();  // default is empty
        }

        /// Return an identifying number of this frame.
        virtual int ident() const = 0;

        /// Return the reference time of the frame
        /// w.r.t. some global time.  Note, each trace has a "tbin"
        /// counting number of ticks w.r.t. to this time where the
        /// trace starts.  In general, tbin is not zero.
        virtual double time() const = 0;

        /// Return the digitization sample period.
        virtual double tick() const = 0;
    };

}  // namespace WireCell

#endif
