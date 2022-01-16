#ifndef WIRECELL_IDIFFUSION
#define WIRECELL_IDIFFUSION

#include "WireCellIface/IData.h"
#include "WireCellIface/IDepo.h"

namespace WireCell {

    /** An interface to information about charge which has diffused in
     * longitudinal and transverse directions.
     *
     * The diffusion is a rectangular patch of (charge) values on a
     * regular grid defined in longitudinal vs. transverse space.
     * Bins are typically tick vs. wire pitch or vs. impact position.
     *
     * See also WireCell::IDiffuser which is one interface that
     * produces these objects.
     */
    class IDiffusion : public IData<IDiffusion> {
       public:
        virtual ~IDiffusion();

        /// Return the deposition that led to this diffusion.
        virtual IDepo::pointer depo() const = 0;

        /// Get value at bin.
        virtual double get(int lind, int tind) const = 0;

        /// Helper method to give the array size in longitudinal dimension.
        virtual int lsize() const = 0;

        /// Helper method to give the array size in transverse dimension.
        virtual int tsize() const = 0;

        // Longitudinal position at index with extra offset (0.5 is bin center).
        virtual double lpos(int ind, double offset = 0.0) const = 0;

        // Transverse position at index with extra offset (0.5 is bin center).
        virtual double tpos(int ind, double offset = 0.0) const = 0;

        /// Return begin of diffusion patch in longitudinal direction.
        double lbegin() const { return lpos(0); }
        /// Return begin of diffusion patch in transverse direction.
        double tbegin() const { return tpos(0); }
        /// Return end of diffusion patch in longitudinal direction.
        double lend() const { return lpos(lsize()); }
        /// Return end of diffusion patch in transverse direction.
        double tend() const { return tpos(tsize()); }
        /// Return bins size in longitudinal direction.
        double lbin() const { return lpos(1) - lpos(0); }
        /// Return bins size in transverse direction.
        double tbin() const { return tpos(1) - tpos(0); }
    };

    // A set ordered by beginning of patch
    struct IDiffusionCompareLbegin {
        bool operator()(const IDiffusion::pointer& lhs, const IDiffusion::pointer& rhs) const
        {
            if (lhs->lbegin() == rhs->lbegin()) {
                return lhs.get() < rhs.get();  // break tie with pointer
            }
            return lhs->lbegin() < rhs->lbegin();
        }
    };
    typedef std::set<IDiffusion::pointer, IDiffusionCompareLbegin> IDiffusionSet;

}  // namespace WireCell
#endif
