// A minimally complete example of a tbb dfp that only deals in boost::any.

#include "WireCellUtil/Testing.h"

#include <tbb/flow_graph.h>
#include <boost/any.hpp>

#include <string>
#include <deque>
#include <iostream>
using namespace std;

// mock INode categories
enum NodeCategory {
    unknownCat,
    sourceCat,
    sinkCat,
    functionCat,
};

class MockNode {
   public:
    virtual ~MockNode() {}
    virtual NodeCategory category() = 0;
    virtual int concurrency() { return 0; }
};

// mock an INode::pointer
typedef std::shared_ptr<MockNode> mock_node_pointer;

class MockSource : public MockNode {
    int m_count;
    const int m_maxcount;

   public:
    MockSource(int maxcount = 10)
      : m_count(0)
      , m_maxcount(maxcount)
    {
    }
    virtual ~MockSource() {}
    virtual NodeCategory category() { return sourceCat; }
    virtual bool extract(boost::any& out)
    {
        cerr << "Source: " << m_count << endl;
        if (m_count > m_maxcount) {
            cerr << "ModeSource drained\n";
            return false;
        }
        ++m_count;
        out = m_count;
        return true;
    }
};

class MockFunction : public MockNode {
    std::deque<int> m_numbers;

   public:
    virtual ~MockFunction() {}
    virtual NodeCategory category() { return functionCat; }
    virtual bool insert(boost::any& anyin)
    {
        int num = boost::any_cast<int>(anyin);
        m_numbers.push_back(num);
        return true;
    }
    virtual bool extract(boost::any& anyout)
    {
        if (m_numbers.empty()) {
            return false;
        }
        anyout = m_numbers.front();
        m_numbers.pop_front();
        return true;
    }
};

class MockSink : public MockNode {
   public:
    virtual ~MockSink() {}
    virtual NodeCategory category() { return sinkCat; }
    virtual bool insert(const boost::any& anyin)
    {
        int num = boost::any_cast<int>(anyin);
        cerr << "Sunk number " << num << endl;
        return true;
    }
};

// fixme: this fakes the factory until we clean up nodes to allow
// empty c'tor and use configuration.
mock_node_pointer get_node(const std::string& node_desc)
{
    if (node_desc == "source") {  // note actual desc should be class or class:inst
        return mock_node_pointer(new MockSource);
    }
    if (node_desc == "drift") {  // note actual desc should be class or class:inst
        return mock_node_pointer(new MockFunction);
    }
    if (node_desc == "sink") {  // note actual desc should be class or class:inst
        return mock_node_pointer(new MockSink);
    }
    return nullptr;
}

typedef tbb::flow::sender<boost::any> sender_type;
typedef tbb::flow::receiver<boost::any> receiver_type;
typedef std::shared_ptr<sender_type> sender_port_pointer;
typedef std::shared_ptr<receiver_type> receiver_port_pointer;
typedef std::vector<sender_port_pointer> sender_port_vector;
typedef std::vector<receiver_port_pointer> receiver_port_vector;

typedef tbb::flow::input_node<boost::any> input_node;
typedef tbb::flow::function_node<boost::any> sink_node;

// base facade, expose sender/receiver ports and provide initialize hook
class TbbNodeWrapper {
   public:
    virtual ~TbbNodeWrapper() {}

    virtual sender_port_vector sender_ports() { return sender_port_vector(); }
    virtual receiver_port_vector receiver_ports() { return receiver_port_vector(); }

    // call before running graph
    virtual void initialize() {}
};

// expose wrappers only as a shared pointer
typedef std::shared_ptr<TbbNodeWrapper> TbbNode;

//
// SOURCE
//

// adapter to convert from WC source node to TBB source node body.
class TbbSourceBody {
   public:
    TbbSourceBody(mock_node_pointer wcnode)
    {
        m_wcnode = std::dynamic_pointer_cast<MockSource>(wcnode);
        Assert(m_wcnode);
    }
    TbbSourceBody(const TbbSourceBody& other)
    {
        cerr << "TbbSourceBody copied\n";
        m_wcnode = other.m_wcnode;
    }
    ~TbbSourceBody() {}

    // assignment - should this in general clone the underlying WC node to allow for proper concurrency?
    void operator=(const TbbSourceBody& other)
    {
        cerr << "TbbSourceBody assigned\n";
        m_wcnode = other.m_wcnode;
    }

    boost::any operator()(tbb::flow_control& fc)
    {
        cerr << "Extracting from " << m_wcnode << endl;
        if (boost::any out; m_wcnode->extract(out)) {
            return out;
        }
        fc.stop();
        return {};
    }

   private:
    std::shared_ptr<MockSource> m_wcnode;
};

// implement facade to access ports for source nodes
class TbbSourceNodeWrapper : public TbbNodeWrapper {
   public:
    TbbSourceNodeWrapper(tbb::flow::graph& graph, mock_node_pointer wcnode)
      : m_tbbnode(new input_node(graph, TbbSourceBody(wcnode)))
    {
    }

    virtual void initialize()
    {
        cerr << "Activating source node\n";
        m_tbbnode->activate();
    }

    virtual sender_port_vector sender_ports()
    {
        auto ptr = dynamic_pointer_cast<sender_type>(m_tbbnode);
        Assert(ptr);
        return sender_port_vector{ptr};
    }

   private:
    std::shared_ptr<input_node> m_tbbnode;
};

//
// SINK
//

// adapter to convert from WC sink node to TBB sink node body.
class TbbSinkBody {
   public:
    TbbSinkBody(mock_node_pointer wcnode)
    {
        m_wcnode = std::dynamic_pointer_cast<MockSink>(wcnode);
        Assert(m_wcnode);
    }
    TbbSinkBody(const TbbSinkBody& other)
    {
        cerr << "TbbSinkBody copied\n";
        m_wcnode = other.m_wcnode;
    }
    ~TbbSinkBody() {}

    // assignment - should this in general clone the underlying WC node to allow for proper concurrency?
    void operator=(const TbbSinkBody& other)
    {
        cerr << "TbbSinkBody assigned\n";
        m_wcnode = other.m_wcnode;
    }

    boost::any operator()(const boost::any& in)
    {
        cerr << "Inserting to " << m_wcnode << endl;
        m_wcnode->insert(in);
        return in;
    }

   private:
    std::shared_ptr<MockSink> m_wcnode;
};

// implement facade to access ports for sink nodes
class TbbSinkNodeWrapper : public TbbNodeWrapper {
   public:
    TbbSinkNodeWrapper(tbb::flow::graph& graph, mock_node_pointer wcnode)
      : m_tbbnode(new sink_node(graph, wcnode->concurrency(), TbbSinkBody(wcnode)))
    {
    }

    virtual receiver_port_vector receiver_ports()
    {
        auto ptr = dynamic_pointer_cast<receiver_type>(m_tbbnode);
        Assert(ptr);
        return receiver_port_vector{ptr};
    }

   private:
    std::shared_ptr<sink_node> m_tbbnode;
};

TbbNode make_node(tbb::flow::graph& graph, const std::string& node_desc)
{
    mock_node_pointer wcnode = get_node(node_desc);
    if (!wcnode) {
        cerr << "Failed to get node for " << node_desc << endl;
        return nullptr;
    }

    cerr << "Getting node from category: " << wcnode->category() << endl;
    switch (wcnode->category()) {
    case sourceCat:
        return TbbNode(new TbbSourceNodeWrapper(graph, wcnode));
    case sinkCat:
        return TbbNode(new TbbSinkNodeWrapper(graph, wcnode));
    // case functionCat:
    // 	return TbbNode(new TbbFunctionNodeWrapper(garph, wcnode));
    default:
        return nullptr;
    }
    return nullptr;
}

bool connect(TbbNode sender, TbbNode receiver, size_t sport = 0, size_t rport = 0);
bool connect(TbbNode sender, TbbNode receiver, size_t sport, size_t rport)
{
    Assert(sender);
    Assert(receiver);
    auto sports = sender->sender_ports();
    auto rports = receiver->receiver_ports();

    Assert(sports.size() > sport);
    Assert(rports.size() > rport);

    sender_type* s = sports[sport].get();
    receiver_type* r = rports[rport].get();
    Assert(s);
    Assert(r);

    cerr << "Connecting " << s << " and " << r << endl;
    make_edge(*s, *r);
    return true;
}

int main()
{
    tbb::flow::graph graph;
    TbbNode source = make_node(graph, "source");
    Assert(source);
    TbbNode sink = make_node(graph, "sink");
    Assert(sink);

    Assert(connect(source, sink));

    sink->initialize();
    source->initialize();

    graph.wait_for_all();

    return 0;
}
