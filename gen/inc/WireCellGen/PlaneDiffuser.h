/**
   A PlaneDiffuser will return a queue of diffusions once enough
   depositions have been collected such that the queue represents all
   diffusions "close enough" to each wire.  It implements a window
   across the wire pitch.
 */

#ifndef WIRECELL_PLANEDIFFUSER
#define WIRECELL_PLANEDIFFUSER

namespace WireCell {

    class PlaneDiffuser : public IDiffuser, public IConfigurable {
       public:
    };

}  // namespace WireCell

#endif
