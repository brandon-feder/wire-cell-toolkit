/** Ress is an interface to wire-cell-ress
 */
#ifndef WIRECELL_RESS_HEADER_SEEN
#define WIRECELL_RESS_HEADER_SEEN

#include <Eigen/Dense>

namespace WireCell {

    namespace Ress {

        typedef Eigen::VectorXd vector_t;
        typedef Eigen::MatrixXd matrix_t;

        enum Model {
            unknown = 0,
            lasso,  // Lasso model
            elnet   // elastic net
        };

        struct Params {
            Model model = elnet;
            double lambda = 1.0;
            int max_iter = 100000;
            double tolerance = 1e-3;
            bool non_negative = true;
            double alpha = 1.0;
        };

        // Solve m = R*s for s, return s.
        vector_t solve(
            // matrix R
            matrix_t response,
            // measured vector m
            vector_t measured,
            // params
            const Params& params = Params(),
            // optional initial source s
            vector_t source = Eigen::VectorXd(),
            // optional initial measurement weights
            vector_t weights = Eigen::VectorXd());

        vector_t predict(matrix_t response, vector_t source);

        double chi2(vector_t measured, vector_t predicted);

        double mean_residual(vector_t measured, vector_t predicted);

        double chi2_l1(vector_t measured, vector_t solved, double lambda = 1.0);

    }  // namespace Ress

}  // namespace WireCell

#endif /* WIRECELL_RESS */
