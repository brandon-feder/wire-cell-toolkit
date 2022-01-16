#include "WireCellIface/IDepo.h"
#include "WireCellUtil/IComponent.h"
#include "WireCellUtil/Testing.h"

#include <tbb/flow_graph.h>

using namespace WireCell;

class IDepoSource : public WireCell::IComponent<IDepoSource> {
   public:
    virtual ~IDepoSource() {}
    virtual bool operator()(WireCell::IDepo::pointer& out) = 0;
};
class IDepoSink : public WireCell::IComponent<IDepoSink> {
   public:
    virtual ~IDepoSink() {}
    virtual bool operator()(const WireCell::IDepo::pointer& in) = 0;
};

class MyDepoSource : public IDepoSource {
   public:
    virtual ~MyDepoSource() {}
    bool operator()(WireCell::IDepo::pointer& out)
    {
        out = nullptr;
        return true;
    }
};

class MyDepoSink : public IDepoSink {
   public:
    virtual ~MyDepoSink() {}
    bool operator()(const WireCell::IDepo::pointer& in) { return true; }
};

class INode : public WireCell::IComponent<INode> {
   public:
    virtual ~INode() {}

    // connect output of tail to our input
    virtual bool connect_tail(INode& tail) = 0;

    // connect input of tail to our output
    virtual bool connect_head(INode& head) = 0;
};

class ITbbNode : public INode {
   public:
    virtual ~ITbbNode() {}

    virtual tbb::flow::graph_node* tbb_node(tbb::flow::graph& graph) = 0;
};

// register with a NamedFactory under IDepoSource class name and instance name TBB
class TbbDepoSource : public ITbbNode {
    IDepoSource::pointer m_src;

   public:
    TbbDepoSource(IDepoSource::pointer src)
      : m_src(src)
    {
    }
    TbbDepoSource(const TbbDepoSource& other)
      : m_src(other.m_src)
    {
    }
    virtual ~TbbDepoSource() {}
    void operator=(const TbbDepoSource& other) { m_src = other.m_src; }

    bool operator()(IDepo::pointer& out) { return (*m_src)(out); }

    tbb::flow::graph_node* tbb_node(tbb::flow::graph& graph)
    {
        return new tbb::flow::source_node<IDepo::pointer>(graph, *this, false);
    }
    virtual void connect(tbb::flow::graph_node& head, tbb::flow::graph_node& tail)
    {
        tbb::flow::sender<IDepo::pointer>* myhead = dynamic_cast<tbb::flow::sender<IDepo::pointer>*>(&head);
        Assert(myhead);
        tbb::flow::receiver<IDepo::pointer>* mytail = dynamic_cast<tbb::flow::receiver<IDepo::pointer>*>(&tail);
        Assert(mytail);
        make_edge(*myhead, *mytail);
    }
};

class TbbDepoSink : public ITbbNode {
    IDepoSink::pointer m_snk;

   public:
    TbbDepoSink(IDepoSink::pointer snk)
      : m_snk(snk)
    {
    }
    TbbDepoSink(const TbbDepoSink& other)
      : m_snk(other.m_snk)
    {
    }
    virtual ~TbbDepoSink() {}
    void operator=(const TbbDepoSink& other) { m_snk = other.m_snk; }

    bool operator()(const IDepo::pointer& out) { return (*m_snk)(out); }

    tbb::flow::graph_node* make_node(tbb::flow::graph& graph, int concurency = 1)
    {
        return new tbb::flow::function_node<IDepo::pointer>(graph, concurency, *this);
    }
    virtual void connect(tbb::flow::graph_node& head, tbb::flow::graph_node& tail)
    {
        tbb::flow::sender<IDepo::pointer>* myhead = dynamic_cast<tbb::flow::sender<IDepo::pointer>*>(&head);
        Assert(myhead);
        tbb::flow::receiver<IDepo::pointer>* mytail = dynamic_cast<tbb::flow::receiver<IDepo::pointer>*>(&tail);
        Assert(mytail);
        make_edge(*myhead, *mytail);
    }
};

int main()
{
    // emulate named factory
    IDepoSource::pointer dsrc(new MyDepoSource);
    IDepoSink::pointer dsnk(new MyDepoSink);

    tbb::flow::graph graph;

    // emulate lookup of tbb wrapper
    ITbbNode::pointer tbbsrc(new TbbDepoSource(dsrc));
    ITbbNode::pointer tbbsnk(new TbbDepoSink(dsnk));

    tbb::flow::graph_node* gnsrc = tbbsrc->make_node(graph);
    tbb::flow::graph_node* gnsnk = tbbsnk->make_node(graph);
    tbbsrc->connect(*gnsrc, *gnsnk);

    graph.wait_for_all();

    return 0;
}
