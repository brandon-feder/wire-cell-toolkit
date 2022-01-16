#include "WireCellAux/FrameTools.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Units.h"
#include "WireCellIface/SimpleFrame.h"
#include "WireCellIface/SimpleTrace.h"

#include <iostream>

using namespace std;
using namespace WireCell;

int main()
{
    const double tick = 0.5 * units::us;
    const double readout = 5 * units::ms;
    const int nsamples = round(readout / tick);

    // "Signal"
    const int signal_frame_ident = 100;
    const double signal_start_time = 6 * units::ms;
    vector<float> signal{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 0.0};

    ITrace::vector signal_traces{make_shared<SimpleTrace>(1, 10, signal), make_shared<SimpleTrace>(1, 20, signal),
                                 make_shared<SimpleTrace>(2, 30, signal), make_shared<SimpleTrace>(2, 33, signal)};
    auto f1 = make_shared<SimpleFrame>(signal_frame_ident, signal_start_time, signal_traces, tick);

    // "Noise"
    const int noise_frame_ident = 0;
    const double noise_start_time = 5 * units::ms;
    vector<float> noise(nsamples, -0.5);
    ITrace::vector noise_traces{make_shared<SimpleTrace>(0, 0, noise), make_shared<SimpleTrace>(1, 0, noise),
                                make_shared<SimpleTrace>(2, 0, noise), make_shared<SimpleTrace>(3, 0, noise)};
    auto f2 = make_shared<SimpleFrame>(noise_frame_ident, noise_start_time, noise_traces, tick);

    const int summed_ident = 1;
    auto f3 = Aux::sum({f1, f2}, summed_ident);
    Assert(f3);
    Assert(f3->ident() == summed_ident);
    Assert(f3->time() == noise_start_time);
    Assert(f3->tick() == tick);
    auto traces = f3->traces();
    Assert(traces->size() == 4);
    cerr << "Frame #" << f3->ident() << " time=" << f3->time() / units::ms << "ms\n";
    for (auto trace : *traces) {
        const auto& charge = trace->charge();
        cerr << "ch=" << trace->channel() << " tbin=" << trace->tbin() << " nsamples=" << charge.size() << endl;
        for (size_t ind = 0; ind < charge.size(); ++ind) {
            const float q = charge[ind];
            if (q < 0) {
                continue;
            }
            cerr << "\tq=" << q << " tbin=" << ind << " t=" << (ind * tick) / units::ms << "ms\n";
        }
        cerr << endl;
    }

    return 0;
}
