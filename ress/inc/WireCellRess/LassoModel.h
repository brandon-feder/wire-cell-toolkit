#ifndef WIRECELLRESS_LASSOMODEL_H
#define WIRECELLRESS_LASSOMODEL_H

#include "WireCellRess/ElasticNetModel.h"

namespace WireCell {

    class LassoModel : public ElasticNetModel {
       public:
        LassoModel(double lambda = 1., int max_iter = 100000, double TOL = 1e-3, bool non_negtive = true);
        virtual ~LassoModel();

        // Perform the fit and return indices of variables below threshold.
        // These can be ignored or the fit may be retried with these variables removed.
        virtual std::vector<size_t> Fit();
        void Set_init_values(std::vector<double> values);

        double chi2_l1();
    };

}  // namespace WireCell

#endif
