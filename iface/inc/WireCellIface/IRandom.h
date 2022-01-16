#ifndef WIRECELL_IRANDOM
#define WIRECELL_IRANDOM

#include "WireCellUtil/IComponent.h"
#include <functional>

namespace WireCell {

    class IRandom : public IComponent<IRandom> {
       public:
        virtual ~IRandom();

        /// Sample a binomial distribution
        virtual int binomial(int max, double prob) = 0;

        /// Sample a Poisson distribution.
        virtual int poisson(double mean) = 0;

        /// Sample a normal distribution.
        virtual double normal(double mean, double sigma) = 0;

        /// Sample a uniform distribution
        virtual double uniform(double begin, double end) = 0;

        /// Sample an exponential distribution
        virtual double exponential(double mean) = 0;

        /// Sample a uniform integer range.
        virtual int range(int first, int last) = 0;
    };

}  // namespace WireCell

#endif
