#include "WireCellUtil/IndexedSet.h"
#include "WireCellUtil/Interface.h"
#include "WireCellUtil/IFactory.h"

WireCell::Interface::~Interface() {}
WireCell::IFactory::~IFactory() {}
WireCell::INamedFactory::~INamedFactory() {}

void dummy() { WireCell::IndexedSet<int> xxx; }
