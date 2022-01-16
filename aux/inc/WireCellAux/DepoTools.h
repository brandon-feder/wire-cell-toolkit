/**
   Some helper functions operating on IDepo
 */
#ifndef WIRECELLAUX_DEPOTOOLS
#define WIRECELLAUX_DEPOTOOLS

#include "WireCellIface/IDepo.h"
#include "WireCellUtil/Array.h"

namespace WireCell::Aux {

     /** Fill data and info arrays from IDepoSet.
             
         The "data" array will be shaped (ndepos,7) and each
         7-tuple holds: (time, charge, x, y, z, long, tran).

         The "info" array is integer and shaped (nedpos, 4).  Each
         4-tuple holds: (id, pdg, gen, child).

         If "priors" is true, then run the depos through flatten() in
         order to save them and give non-zero gen and child.  O.w.,
         only those depos in pass directly in the vector are saved.
     */
     void fill(Array::array_xxf& data, Array::array_xxi& info, 
               const IDepo::vector& depos, bool priors = true);
         
}

#endif
