#ifndef WIRECELL_DIFFUSER
#define WIRECELL_DIFFUSER

#include "WireCellIface/IDiffuser.h"
#include "WireCellIface/IDiffusion.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IDepo.h"

#include <vector>
#include <deque>

namespace WireCell {

    /** Model longitudinal and transverse diffusion of drifted charge.
     *
     * WireCell::IDepo objects are inserted and WireCell::IDiffusion
     * objects are extracted.  An internal buffer is kept in order to
     * assure the output is time ordered by the leading edge of the
     * diffusion "patch".  Depositions are diffused in place so are
     * assumed to be drifted to whatever location they are wanted.
     * The IDepo::prior() method is used to find the total drift
     * distance.
     *
     * FIXME: this action is really a compound as it does three things:
     *
     * 1) Projects a deposition position onto wire pitch.
     * 2) Applies arbitrary time offset.
     * 3) Diffuses the deposition.
     */
    class Diffuser : public IDiffuser, public IConfigurable {
       public:
        typedef std::pair<double, double> bounds_type;

        /** Create a diffuser.
         *
         * \param pitch gives the wire coordinate system.
         * \param lbinsize defines the grid spacing in the longitudinal direction.
         * \param tbinsize defines the grid spacing in the transverse direction.
         * \param lorigin defines a grid line in the longitudinal direction.
         * \param torigin defines a grid line in the transverse direction.
         * \param DL is the longitudinal diffusion coefficient.
         * \param DT is the transverse diffusion coefficient.
         * \param drift_velocity is what it sounds like.
         * \param max_sigma_l is the largest possible longitudinal diffusion.
         * \param nsigma defines the number of sigma at which to truncate the Gaussian.
         */
        Diffuser(const Ray& pitch = Ray(Point(0.0, 0.0, 0.0), Point(0.0, 0.0, 5 * units::millimeter)),
                 double binsize_l = 2.0 * units::millimeter, double time_offset = 0.0 * units::microsecond,
                 double origin_l = 0.0 * units::microsecond, double DL = 5.3 * units::centimeter2 / units::second,
                 double DT = 12.8 * units::centimeter2 / units::second,
                 double drift_velocity = 1.6 * units::millimeter / units::microsecond,
                 double max_sigma_l = 5 * units::microsecond, double nsigma = 3.0);

        virtual ~Diffuser();

        virtual void configure(const WireCell::Configuration& config);
        virtual WireCell::Configuration default_configuration() const;

        virtual void reset();

        // IQueuedoutNode.
        virtual bool operator()(const input_pointer& depo, output_queue& outq);

        /** Diffuse a point charge.
         *
         * \param mean_l is the position in the longitudinal direction.
         * \param mean_t is the position in the transverse direction.
         * \param sigma_l is the Gaussian sigma in the longitudinal direction.
         * \param sigma_t is the Gaussian sigma in the transverse direction.
         * \param weight is the normalization (eg, amount of charge)
         *
         * This is a mostly internal method.  It does not directly
         * fill or drain any internal buffers so can be used to test
         * diffusion directly.
         */
        IDiffusion::pointer diffuse(double mean_l, double mean_t, double sigma_l, double sigma_t, double weight = 1.0,
                                    IDepo::pointer depo = nullptr);

        /* Internal function, return the +/- nsigma*sigma bounds
         * around mean enlarged to fall on bin edges.
         */
        bounds_type bounds(double mean, double sigma, double binsize, double origin = 0.0);

        /* Internal function, return a 1D distribution of mean/sigma
         * binned with zeroth bin at origin and given binsize.
         */
        std::vector<double> oned(double mean, double sigma, double binsize, const Diffuser::bounds_type& bounds);

       private:
        Point m_pitch_origin;
        Vector m_pitch_direction;
        double m_time_offset;
        double m_origin_l;
        double m_origin_t;
        double m_binsize_l;
        double m_binsize_t;
        double m_DL, m_DT;
        double m_drift_velocity;
        double m_max_sigma_l;
        double m_nsigma;

        IDiffusionSet m_input;  // to allow for proper output ordering

        bool m_eos;

        void dump(const std::string& msg);
    };

}  // namespace WireCell

#endif
