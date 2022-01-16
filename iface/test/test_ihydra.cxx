#include "WireCellIface/IHydraNode.h"
#include "WireCellIface/IDepoSource.h"
#include "WireCellUtil/Type.h"
#include <iostream>
#include <tuple>

typedef std::tuple<WireCell::IDepo, WireCell::IDepo> in_tuple_t;
typedef std::tuple<WireCell::IDepo> out_tuple_t;

class MyDepoSrc : public WireCell::IDepoSource {
   public:
    virtual ~MyDepoSrc() {}
    virtual bool operator()(WireCell::IDepo::pointer& depo)
    {
        using namespace WireCell;

        std::cerr << "Running instance of " << demangle(signature()) << "\n";

        std::cerr << "input types:\n";
        for (auto tn : input_types()) {
            std::cerr << "\t" << demangle(tn) << "\n";
        }

        std::cerr << "output types:\n";
        for (auto tn : output_types()) {
            std::cerr << "\t" << demangle(tn) << "\n";
        }
        return true;
    }
};

class MyHydra : public WireCell::IHydraNode<in_tuple_t, out_tuple_t> {
   public:
    virtual ~MyHydra() {}

    virtual bool operator()(input_queues_type& inqs, output_queues_type& outqs)
    {
        using namespace WireCell;

        std::cerr << "Running instance of " << demangle(signature()) << "\n";
        std::cerr << "input_queues_type:\n\t" << demangle(typeid(input_queues_type).name()) << "\n";
        std::cerr << "output_queues_type:\n\t" << demangle(typeid(output_queues_type).name()) << "\n";

        std::cerr << "input types:\n";
        for (auto tn : input_types()) {
            std::cerr << "\t" << demangle(tn) << "\n";
        }

        std::cerr << "output types:\n";
        for (auto tn : output_types()) {
            std::cerr << "\t" << demangle(tn) << "\n";
        }

        return true;
    }

    virtual std::string signature() { return typeid(MyHydra).name(); }
};

int main()
{
    WireCell::IDepo::pointer out;
    MyDepoSrc mds;
    mds(out);

    MyHydra::input_queues_type inqs;
    MyHydra::output_queues_type outqs;

    MyHydra mh;
    mh(inqs, outqs);

    return 0;
}
