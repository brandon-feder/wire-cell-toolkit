#ifndef WIRECELLTEST_MULTIPDF
#define WIRECELLTEST_MULTIPDF

#include "TCanvas.h"

namespace WireCell {
    namespace Test {
        struct MultiPdf {
            TCanvas canvas;
            const char* name;
            MultiPdf(const char* name)
              : canvas("c", "canvas", 500, 500)
              , name(name)
            {
                canvas.Print(Form("%s.pdf[", name), "pdf");
            }
            ~MultiPdf() { close(); }
            void operator()()
            {
                canvas.Print(Form("%s.pdf", name), "pdf");
                canvas.Clear();
            }
            void close()
            {
                if (name) {
                    canvas.Print(Form("%s.pdf]", name), "pdf");
                    name = nullptr;
                }
            }
        };

    }  // namespace Test
}  // namespace WireCell

#endif
