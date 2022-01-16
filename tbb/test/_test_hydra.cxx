#include <tbb/flow_graph.hpp>
#include <boost/any.hpp>

typedef std::deque<boost::any> AnyQueue;
typedef std::vector<AnyQueue> AnyPorts;

class Hydra {
   public:
    virtual ~Hydra() {}
    bool insert(AnyPorts& input) = 0;
    bool extract(AnyPorts& output) = 0;
};

template <typename OutputType>
class Source : public Hydra {
   public:
    virtual ~Source() {}

    typedef std::shared_ptr<const OutputType> output_pointer;

    virtual bool insert(AnyPorts& input) { return false; }
    virtual bool extract(AnyPorts& output)
    {
        output_pointer out;
        bool ok = this->extract(out);
        if (!ok) return false;
        output[0].push_back(out);
        return true;
    }

    virtual bool extract(output_pointer& out) = 0;
};

template <typename InputType>
class Sink : public Hydra {
   public:
    virtual ~Sink() {}

    typedef std::shared_ptr<const InputType> input_pointer;

    virtual bool extract(AnyPorts& output) { return false; }
    virtual bool insert(AnyPorts& input)
    {
        for (auto anyin : input[0]) {
            input_pointer in = any_cast<input_pointer>(anyin);
            bool ok = this->insert(in);
            if (!ok) return false;
        }
        return true;
    }

    virtual bool insert(const input_pointer& in) = 0;
};

class IntSource : public Source<int> {
    int m_count;
    const int m_max;

   public:
    IntSource(int maxcount = 10)
      : m_count(0)
      , m_max(maxcount)
    {
    }
    virtual ~IntSource() {}

    virtual bool extract(output_pointer& out)
    {
        ++m_count;
        if (m_count > m_max) {
            return false;
        }
        out = new int(m_count);
        return true;
    }
};

class IntSink : public Sink<int> {
   public:
    virtual ~IntSink() {}
    virtual bool insert(const input_pointer& in) { return true; }
};

int main() {}
