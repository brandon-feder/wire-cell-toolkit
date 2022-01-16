#include "WireCellIface/IDepo.h"

namespace WireCell {

    // A deposition that simply holds all the data it presents.  If
    // you have something that makes depositions immediately and needs
    // no "smarts" just use this to hold their info outright.
    class SimpleDepo : public WireCell::IDepo {
    public:
        SimpleDepo(double t, const WireCell::Point& pos,
                   double charge = 1.0,
                   IDepo::pointer prior = nullptr,
                   double extent_long = 0.0,
                   double extent_tran = 0.0,
                   int id = 0, int pdg = 0,
                   double energy = 1.0);

        virtual const WireCell::Point& pos() const;
        virtual double time() const;
        virtual double charge() const;
        virtual double energy() const;
        virtual int id() const;
        virtual int pdg() const;
        virtual WireCell::IDepo::pointer prior() const;
        virtual double extent_long() const;
        virtual double extent_tran() const;

    public:                     // not available via IDepo
        void set_prior(WireCell::IDepo::pointer p) { m_prior = p; }

    private:
        // bag o' data
        double m_time;
        WireCell::Point m_pos;
        int m_id;
        int m_pdg;
        double m_charge;
        double m_energy;
        IDepo::pointer m_prior;
        double m_long, m_tran;
    };

}  // namespace WireCell
