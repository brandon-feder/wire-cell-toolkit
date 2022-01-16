#ifndef WIRECELLROOT_ROOTFILECREATION_H
#define WIRECELLROOT_ROOTFILECREATION_H

#include "WireCellIface/IDepoFilter.h"
#include "WireCellIface/IFrameFilter.h"
#include "WireCellIface/IConfigurable.h"

// class TFile;

namespace WireCell {
    namespace Root {

        class RootfileCreation_depos : public IDepoFilter, public IConfigurable {
           public:
            RootfileCreation_depos();
            virtual ~RootfileCreation_depos();

            virtual bool operator()(const WireCell::IDepo::pointer& indepo, WireCell::IDepo::pointer& outdepo);

            /* virtual bool operator()(const IFrame::pointer& in, IFrame::pointer& out); */

            /// IConfigurable
            virtual WireCell::Configuration default_configuration() const;
            virtual void configure(const WireCell::Configuration& config);

           private:
            Configuration m_cfg;
            void create_file();
        };

        class RootfileCreation_frames : public IFrameFilter, public IConfigurable {
           public:
            RootfileCreation_frames();
            virtual ~RootfileCreation_frames();

            virtual bool operator()(const IFrame::pointer& in, IFrame::pointer& out);

            /// IConfigurable
            virtual WireCell::Configuration default_configuration() const;
            virtual void configure(const WireCell::Configuration& config);

           private:
            Configuration m_cfg;
            void create_file();
        };

    }  // namespace Root
}  // namespace WireCell

#endif
