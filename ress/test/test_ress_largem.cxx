#include "WireCellRess/LassoModel.h"
#include "WireCellRess/ElasticNetModel.h"

#include <Eigen/Dense>
using namespace Eigen;

#include <iostream>
using namespace std;

void test_model(WireCell::LinearModel& m, MatrixXd& G, VectorXd& W);
void test_lasso(WireCell::LassoModel& m, MatrixXd& G, VectorXd& W);
void print_results(WireCell::LinearModel& m, VectorXd& C);

int main(int argc, char* argv[])
{
    // std::srand((unsigned int) time(0));

    const int N_CELL = 500;
    const int N_NZERO = 50;
    const int N_WIRE = int(N_CELL * 0.5);

    VectorXd C = VectorXd::Zero(N_CELL);
    for (int i = 0; i < N_NZERO; i++) {
        int index = int(N_CELL / 2 * (VectorXd::Random(1)(0) + 1));
        // cout << index << endl;
        C(index) = VectorXd::Random(1)(0) * 50 + 150;
    }

    // initialize G matrix: N_WIRE rows and N_CELL columns. (geometry matrix)
    MatrixXd G = MatrixXd::Random(N_WIRE, N_CELL);

    // W vector is the measured charge on wires.
    VectorXd W = G * C;

    // cout << "geometry matrix:" << endl;
    // cout << G << endl << endl;

    // cout << "measured charge on each wire:" << endl;
    // cout << W.transpose() << endl << endl;

    // cout << "true charge of each cell:" << endl;
    // cout << C.transpose() << endl << endl;
    double lambda = 1;

    // WireCell::ElasticNetModel m(lambda, 0.95, 100000, 1e-4);
    // test_model(m, G, W);
    // print_results(m, C);

    WireCell::LassoModel m2(lambda, 10000, 1e-3);
    test_lasso(m2, G, W);
    print_results(m2, C);

    return 0;
}

void test_model(WireCell::LinearModel& m, MatrixXd& G, VectorXd& W)
{
    m.SetData(G, W);
    m.Fit();
}

void test_lasso(WireCell::LassoModel& m, MatrixXd& G, VectorXd& W)
{
    m.SetData(G, W);

    // one can set the weight of each cell's regularization.
    // m.SetLambdaWeight(0, 10.);
    // m.SetLambdaWeight(2, 10.);
    // m.SetLambdaWeight(6, 10.);
    m.Fit();
}

void print_results(WireCell::LinearModel& m, VectorXd& C)
{
    VectorXd beta = m.Getbeta();

    // cout << "fitted charge of each cell: " << m.name << endl;
    // cout << beta.transpose() << endl << endl;

    // cout << "predicted charge on each wire: Lasso" << endl;
    // cout << m.Predict().transpose() << endl << endl;

    cout << "average residual charge difference per wire: " << m.name << ": " << m.MeanResidual() << endl;

    int nbeta = beta.size();

    int n_zero_true = 0;
    int n_zero_beta = 0;
    int n_zero_correct = 0;

    for (int i = 0; i < nbeta; i++) {
        if (fabs(C(i)) < 0.1) n_zero_true++;
        if (fabs(beta(i)) < 5) n_zero_beta++;
        if (fabs(C(i)) < 0.1 && (fabs(C(i) - beta(i)) < 10)) n_zero_correct++;
    }
    cout << "true zeros: " << n_zero_true << endl;
    cout << "fitted zeros: " << n_zero_beta << endl;
    cout << "correct fitted zeros: " << n_zero_correct << endl;

    cout << endl;
}