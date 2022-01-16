/**
   Gen::Random is an IRandom which is implemented with standard C++ <random>.
 */

#ifndef WIRECELLGEN_RANDOM
#define WIRECELLGEN_RANDOM

#include "WireCellIface/IRandom.h"
#include "WireCellIface/IConfigurable.h"

namespace WireCell {
    namespace Gen {

        class Random : public IRandom, public IConfigurable {
           public:
            Random(const std::string& generator = "default", const std::vector<unsigned int> seeds = {0, 0, 0, 0, 0});
            virtual ~Random() {}

            // IConfigurable interface
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;

            /// Sample a binomial distribution
            virtual int binomial(int max, double prob);

            /// Sample a Poisson distribution.
            virtual int poisson(double mean);

            /// Sample a normal distribution.
            virtual double normal(double mean, double sigma);

            /// Sample a uniform distribution
            virtual double uniform(double begin, double end);

            /// Sample an exponential distribution
            virtual double exponential(double mean);

            /// Sample a uniform integer range.
            virtual int range(int first, int last);

           private:
            std::string m_generator;
            std::vector<unsigned int> m_seeds;
            IRandom* m_pimpl;
        };

    }  // namespace Gen
}  // namespace WireCell
#endif
