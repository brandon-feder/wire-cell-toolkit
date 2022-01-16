#include "WireCellPytorch/Util.h"

using namespace WireCell;

int main()
{
    auto ten = torch::ones({1, 3, 800, 600}, torch::dtype(torch::kFloat32).device(torch::kCUDA, 0));
    std::cout << Pytorch::dump(ten) << "\n";

    return 0;
}