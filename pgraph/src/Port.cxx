#include "WireCellPgraph/Port.h"

#include <iostream>

using namespace std;
using namespace WireCell::Pgraph;

Port::Port(Node* node, Type type, std::string signature, std::string name)
  : m_node(node)
  , m_type(type)
  , m_name(name)
  , m_sig(signature)
  , m_edge(nullptr)
{
}

bool Port::isinput() { return m_type == Port::input; }
bool Port::isoutput() { return m_type == Port::output; }

Edge Port::edge() { return m_edge; }

// Connect an edge, returning any previous one.
Edge Port::plug(Edge edge)
{
    Edge ret = m_edge;
    m_edge = edge;
    return ret;
}

// return edge queue size or 0 if no edge has been plugged
size_t Port::size()
{
    if (!m_edge) {
        return 0;
    }
    return m_edge->size();
}

// Return true if queue is empty or no edge has been plugged.
bool Port::empty()
{
    if (!m_edge or m_edge->empty()) {
        return true;
    }
    return false;
}

// Get the next data.  By default this pops the data off
// the queue.  To "peek" at the data, pas false.
Data Port::get(bool pop)
{
    if (isoutput()) {
        THROW(RuntimeError() << errmsg{"can not get from output port"});
    }
    if (!m_edge) {
        THROW(RuntimeError() << errmsg{"port has no edge"});
    }
    if (m_edge->empty()) {
        THROW(RuntimeError() << errmsg{"edge is empty"});
    }
    Data ret = m_edge->front();
    if (pop) {
        m_edge->pop_front();
    }
    return ret;
}

// Put the data onto the queue.
void Port::put(Data& data)
{
    if (isinput()) {
        THROW(RuntimeError() << errmsg{"can not put to input port"});
    }
    if (!m_edge) {
        THROW(RuntimeError() << errmsg{"port has no edge"});
    }
    m_edge->push_back(data);
}

const std::string& Port::name() { return m_name; }
const std::string& Port::signature() { return m_sig; }
