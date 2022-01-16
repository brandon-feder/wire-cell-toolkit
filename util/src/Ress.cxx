#include "WireCellUtil/Ress.h"

#include "WireCellRess/LassoModel.h"

using namespace WireCell;

Ress::vector_t Ress::solve(Ress::matrix_t matrix, Ress::vector_t measured, const Ress::Params& params,
                           Ress::vector_t initial, Ress::vector_t weights)
{
    // Provide a uniform interface to RESS solving models.  RESS
    // *almost* already provides this.

    if (params.model == Ress::lasso) {
        WireCell::LassoModel model(params.lambda, params.max_iter, params.tolerance, params.non_negative);
        if (initial.size()) {
            model.Setbeta(initial);
        }
        if (weights.size()) {
            model.SetLambdaWeight(weights);
        }
        model.SetData(matrix, measured);
        model.Fit();
        return model.Getbeta();
    }

    if (params.model == Ress::elnet) {
        WireCell::ElasticNetModel model(params.lambda, params.alpha, params.max_iter, params.tolerance,
                                        params.non_negative);
        if (initial.size()) {
            model.Setbeta(initial);
        }
        if (weights.size()) {
            model.SetLambdaWeight(weights);
        }
        model.SetData(matrix, measured);
        model.Fit();
        return model.Getbeta();
    }

    return Ress::vector_t();
}

double Ress::chi2_l1(vector_t measured, vector_t solved, double lambda)
{
    return 2 * lambda * solved.lpNorm<1>() * measured.size();
}

Ress::vector_t Ress::predict(matrix_t response, vector_t source) { return response * source; }

double Ress::chi2(vector_t measured, vector_t predicted) { return (measured - predicted).squaredNorm(); }

double Ress::mean_residual(vector_t measured, vector_t predicted)
{
    return (measured - predicted).norm() / measured.size();
}
