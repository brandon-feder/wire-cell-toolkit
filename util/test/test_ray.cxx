#include "WireCellUtil/Point.h"
#include "WireCellUtil/Testing.h"
#include "WireCellUtil/Logging.h"

using namespace WireCell;
using spdlog::info;

int main()
{
    const Ray r1(Point(-1, -1, -1), Point(1, 1, 1));
    const Ray r2(Point(0, 0, 0), Point(1, 1, 1));
    info("Some rays: {}, {}", r1, r2);

    const Ray& rr1 = r1;
    const Ray& rr2 = r2;
    info("Some ray refs: {}, {}", rr1, rr2);
}
