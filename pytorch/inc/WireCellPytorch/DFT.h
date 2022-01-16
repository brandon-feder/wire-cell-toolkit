/**
   TorchDFT provides a libtorch based implementation of IDFT.

   The libtorch API is documented at:

   https://pytorch.org/cppdocs/api/namespace_torch__fft.html
 */

#ifndef WIRECELL_PYTORCH_DFT
#define WIRECELL_PYTORCH_DFT

#include "WireCellIface/IDFT.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellPytorch/TorchContext.h"

namespace WireCell::Pytorch {
    class DFT : public IDFT,
                public IConfigurable
    {
      public:
        DFT();
        virtual ~DFT();

        // IConfigurable interface
        virtual void configure(const WireCell::Configuration& config);
        virtual WireCell::Configuration default_configuration() const;

        // 1d 

        virtual 
        void fwd1d(const complex_t* in, complex_t* out,
                   int size) const;

        virtual 
        void inv1d(const complex_t* in, complex_t* out,
                   int size) const;

        // batched 1D ("1b") - rely on base implementation
        virtual 
        void fwd1b(const complex_t* in, complex_t* out,
                   int nrows, int ncols, int axis) const;
        virtual 
        void inv1b(const complex_t* in, complex_t* out,
                   int nrows, int ncols, int axis) const;
        
        // 2d

        virtual 
        void fwd2d(const complex_t* in, complex_t* out,
                   int nrows, int ncols) const;
        virtual 
        void inv2d(const complex_t* in, complex_t* out,
                   int nrows, int ncols) const;
          
      private:
        TorchContext m_ctx;

    };

}

#endif
