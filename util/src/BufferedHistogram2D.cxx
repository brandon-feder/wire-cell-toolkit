#include "WireCellUtil/BufferedHistogram2D.h"

using namespace WireCell;
using namespace std;

BufferedHistogram2D::BufferedHistogram2D(double xbinsize, double ybinsize, double xmin, double ymin)
  : m_xmin(xmin)
  , m_ymin(ymin)
  , m_xbinsize(xbinsize)
  , m_ybinsize(ybinsize)
{
}

double BufferedHistogram2D::xmax() const { return xmin() + m_xbinsize * m_xbindeque.size(); }
double BufferedHistogram2D::ymax() const { return ymin() + m_ybinsize * ysize(); }
int BufferedHistogram2D::ysize() const
{
    int imax = 0;
    for (auto v : m_xbindeque) {
        imax = std::max((int) v.size(), imax);
    }
    return imax;
}

bool BufferedHistogram2D::fill(double x, double y, double v)
{
    int ybin = (y - m_ymin) / m_ybinsize;
    if (ybin < 0) {
        return false;
    }
    int xbin = (x - m_xmin) / m_xbinsize;
    if (xbin < 0) {
        return false;
    }

    if ((int) m_xbindeque.size() < xbin + 1) {
        m_xbindeque.resize(xbin + 1);
    }
    XBin& vec = m_xbindeque[xbin];
    if ((int) vec.size() < ybin + 1) {
        vec.resize(ybin + 1, 0.0);
    }
    vec[ybin] += v;

    return true;
}

std::vector<double> BufferedHistogram2D::popx()
{
    std::vector<double> ret;

    if (!m_xbindeque.size()) {
        return ret;
    }

    ret = m_xbindeque.front();
    m_xbindeque.pop_front();
    m_xmin += m_xbinsize;
    return ret;
}
