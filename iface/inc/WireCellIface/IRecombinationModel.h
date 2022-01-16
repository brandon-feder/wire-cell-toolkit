#ifndef WIRECELL_IRECOMBINATIONMODEL
#define WIRECELL_IRECOMBINATIONMODEL

#include "WireCellUtil/IComponent.h"

namespace WireCell {

    class IRecombinationModel : virtual public IComponent<IRecombinationModel> {
       public:
        virtual ~IRecombinationModel();

        // Convert a point or step to ionized charge
        virtual double operator()(double dE, double dX = 0.0) = 0;
    };

}  // namespace WireCell

#endif
