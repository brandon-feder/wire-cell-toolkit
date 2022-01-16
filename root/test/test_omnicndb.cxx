#include "WireCellSigProc/OmniChannelNoiseDB.h"
#include "WireCellUtil/Persist.h"
#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/NamedFactory.h"
#include "WireCellIface/IAnodePlane.h"
#include "WireCellIface/IConfigurable.h"

#include "TCanvas.h"
#include "TGraph.h"
#include "TStyle.h"
#include "TH1F.h"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>

const std::string config_text = R"JSONNET(
// example OmniChannelNoiseDB configuration.

local wc = import "wirecell.jsonnet";

{
    anode: "AnodePlane:0",  
    tick: 0.5*wc.us,
    nsamples: 9600,

    // channel groups is a 2D list.  Each element is one group of
    // channels which should be considered together for coherent noise
    // filtering.  
    groups: [std.range(g*48, (g+1)*48-1) for g in std.range(0,171)],

    // Default channel info which is used if not overriden by one of
    // the channel_info objects (below) for a given channel.
    default_info : {
	nominal_baseline: 0.0,  // adc count
        gain_correction: 1.0,     // unitless
        response_offset: 79,      // ticks?
        pad_window_front: 20,     // ticks?
	pad_window_back: 10,      // ticks?
	min_rms_cut: 1.0,         // units???
	max_rms_cut: 5.0,         // units???

        // parameter used to make "rcrc" spectrum
        rcrc: 1.0*wc.millisecond,

        // parameters used to make "config" spectrum
        reconfig : {},

        // list to make "noise" spectrum mask
        freqmasks: [],

        // field response waveform to make "response" spectrum
        response: {},

    },

    // overide defaults for specific channels.  If an info is
    // mentioned for a particular channel in multiple objects, last
    // one wins.
    channel_info: [             

        {
            channels: 4,        // single channel
            nominal_baseline: 400,
        },
        {
            channels: [1,42,69],      // explicit list
	    nominal_baseline: 2048.0, // adc count
            reconfig : {
                from: {gain: 7.8*wc.mV/wc.fC, shaping: 1.0*wc.us},
                to: {gain: 14.0*wc.mV/wc.fC, shaping: 2.0*wc.us}, 
            }
        },
        {
            channels: { first: 1, last: 4 }, // inclusive range.
            // could also use Jsonnet's std.range(first,last):
            // channels: std.range(1,4),
	    nominal_baseline: 400.0,
        },
        {
            channels: {first: 0, last: 2047,},
            freqmasks: [            // masks in frequency domain.
                { value: 1.0, lobin: 0, hibin: $.nsamples-1 },
                { value: 0.0, lobin: 169, hibin: 173 },
                { value: 0.0, lobin: 513, hibin: 516 },
            ]
        },
        {
            // All channels in a given anode wire plane.  Note, if
            // used, the anode plane must be added to top level
            // configuration sequence.  The "channels" can be
            // specified by the other means.
            channels: { wpid: wc.WirePlaneId(wc.Ulayer) },
            response: {
                // the average response to use that of the wpid.
                wpid: wc.WirePlaneId(wc.Ulayer)
                // or as a raw waveform:
                // waveform: [time-domain samples assumed to be at tick sampling period]
                // waveformid: <uniquenumber>
            }
        }
    ],

}

)JSONNET";

#include "anode_loader.h"
using namespace WireCell;
using namespace std;

typedef std::vector<WireCell::IChannelNoiseDatabase::filter_t> filter_bag_t;

void plot_spec(const filter_bag_t& specs, const std::string& name)
{
    if (specs.empty()) {
        cerr << "No specs for \"" << name << "\"\n";
        return;
    }
    cerr << "plot spec \"" << name << "\" size=" << specs.size() << endl;

    std::vector<TGraph*> graphs;
    std::vector<float> tmp;
    for (const auto& spec : specs) {
        TGraph* graph = new TGraph();
        graphs.push_back(graph);
        for (size_t ind = 0; ind < spec.size(); ++ind) {
            double amp = std::abs(spec.at(ind));
            graph->SetPoint(ind, ind, amp);
            tmp.push_back(amp);
        }
    }
    auto mme = std::minmax_element(tmp.begin(), tmp.end());
    float ymin = *mme.first;
    float ymax = *mme.second;

    const int ncolors = 5;
    int colors[ncolors] = {1, 2, 4, 6, 8};
    for (size_t igraph = 0; igraph < graphs.size(); ++igraph) {
        TGraph* graph = graphs[igraph];
        graph->SetLineColor(colors[igraph % ncolors]);
        graph->SetLineWidth(2);

        if (!igraph) {
            auto frame = graph->GetHistogram();
            frame->SetTitle(name.c_str());
            frame->GetXaxis()->SetTitle("frequency bins");
            frame->GetYaxis()->SetTitle("amplitude");
            graph->Draw("AL");
            frame->SetMinimum(ymin);
            frame->SetMaximum(ymax);
            cerr << name << " ymin=" << ymin << ", ymax=" << ymax << endl;
        }
        else {
            graph->Draw("L");
        }
    }
}

int main(int argc, char* argv[])
{
    /// User code should never do this.
    auto anode_tns = anode_loader("uboone");
    const std::string pcr_filename = "microboone-channel-responses-v1.json.bz2";

    {
        Configuration cfg;

        if (argc > 1) {
            cerr << "testing with " << argv[1] << endl;
            WireCell::Persist::externalvars_t extvar;
            extvar["detector"] = "uboone";
            cfg = Persist::load(argv[1], extvar);
            if (cfg.isArray()) {  // probably a full configuration
                for (auto jone : cfg) {
                    string the_type = jone["type"].asString();
                    if (the_type == "wclsChannelNoiseDB" || the_type == "OmniChannelNoiseDB") {
                        // cerr << "Found my config\n" << jone << "\n";
                        cfg = jone["data"];
                        break;
                    }
                }
            }
        }
        else {
            cerr << "testing with build in config text\n";
            cfg = Persist::loads(config_text);
        }
        cfg["anode"] = anode_tns[0];

        auto icfg = Factory::lookup_tn<IConfigurable>("OmniChannelNoiseDB");
        auto def = icfg->default_configuration();
        cfg = update(def, cfg);
        icfg->configure(cfg);
    }

    auto anode = Factory::find_tn<IAnodePlane>(anode_tns[0]);
    const int nchannels = anode->channels().size();

    auto idb = Factory::find_tn<IChannelNoiseDatabase>("OmniChannelNoiseDB");

    gStyle->SetOptStat(0);
    TCanvas canvas("canvas", "canvas", 500, 500);

    string pdfname = Form("%s.pdf", argv[0]);

    canvas.Print((pdfname + "[").c_str(), "pdf");
    canvas.SetGridx(1);
    canvas.SetGridy(1);
    double tick = idb->sample_time();
    cerr << "tick = " << tick / units::us << " us.\n";

    std::vector<std::string> scalar_names{"nominal baseline", "gain correction", "response offset", "pad window front",
                                          "pad window back",  "min rms cut",     "max rms cut",     "rcrc sum",
                                          "config sum",       "noise sum",       "response sum"};

    const int nscalars = 11;
    std::vector<TGraph*> scalars;
    for (int ind = 0; ind < 11; ++ind) {
        scalars.push_back(new TGraph);
    }
    for (int ch = 0; ch < nchannels; ++ch) {
        scalars[0]->SetPoint(ch, ch, idb->nominal_baseline(ch));
        scalars[1]->SetPoint(ch, ch, idb->gain_correction(ch));
        scalars[2]->SetPoint(ch, ch, idb->response_offset(ch));
        scalars[3]->SetPoint(ch, ch, idb->pad_window_front(ch));
        scalars[4]->SetPoint(ch, ch, idb->pad_window_back(ch));
        scalars[5]->SetPoint(ch, ch, idb->min_rms_cut(ch));
        scalars[6]->SetPoint(ch, ch, idb->max_rms_cut(ch));

        scalars[7]->SetPoint(ch, ch, std::abs(Waveform::sum(idb->rcrc(ch))));
        scalars[8]->SetPoint(ch, ch, std::abs(Waveform::sum(idb->config(ch))));
        scalars[9]->SetPoint(ch, ch, std::abs(Waveform::sum(idb->noise(ch))));
        scalars[10]->SetPoint(ch, ch, std::abs(Waveform::sum(idb->response(ch))));
    }

    for (size_t ind = 0; ind < nscalars; ++ind) {
        TGraph* graph = scalars[ind];
        graph->SetName(scalar_names[ind].c_str());
        graph->SetLineColor(2);
        graph->SetLineWidth(3);
        graph->Draw("AL");

        auto frame = graph->GetHistogram();
        frame->SetTitle(scalar_names[ind].c_str());
        frame->GetXaxis()->SetTitle("channels");
        canvas.Print(pdfname.c_str(), "pdf");
    }

    canvas.Print((pdfname + "]").c_str(), "pdf");

    return 0;
}
