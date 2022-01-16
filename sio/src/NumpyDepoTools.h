#ifndef WIRECELLSIO_NUMPYDEPOTOOLS
#define WIRECELLSIO_NUMPYDEPOTOOLS

#include "WireCellIface/IDepo.h"

#include <string>

namespace WireCell::Sio::NumpyDepoTools {

    bool load(const std::string& fname, int count,
              WireCell::IDepo::vector &depos);
}

#endif

