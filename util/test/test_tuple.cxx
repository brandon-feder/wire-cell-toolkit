#include "WireCellUtil/TupleHelpers.h"
#include "WireCellUtil/Testing.h"

#include <iostream>

using namespace WireCell;

int main()
{
    typedef std::tuple<int, float, double, char, std::string> IFDCS;
    typedef tuple_helper<IFDCS> IFDCS_helper;
    typedef shared_queued<IFDCS> IFDCS_shqed;
    typedef IFDCS_shqed::shared_queued_tuple_type IFDCS_queues;

    IFDCS_helper ifdcs_helper;
    IFDCS_shqed ifdcs_shqed;

    std::vector<std::string> typenames = ifdcs_helper.type_names();
    for (auto tn : typenames) {
        std::cerr << tn << std::endl;
    }

    IFDCS ifdcs{1, 2.2, 3.0e-9, 'a', "foo"};
    // std::vector<boost::any> anyvec = as_any(ifdcs);
    std::vector<boost::any> anyvec = ifdcs_helper.as_any(ifdcs);
    std::cerr << boost::any_cast<int>(anyvec[0]) << std::endl;
    std::cerr << boost::any_cast<std::string>(anyvec[4]) << std::endl;

    auto ifdcs2 = ifdcs_helper.from_any(anyvec);

    // typedef typename tuple_helper<IFDCS>::Wrapped<std::deque>::type ifdcs_queues;
    IFDCS_queues qs;
    std::get<0>(qs).push_back(std::make_shared<int>(1));
    std::get<1>(qs).push_back(std::make_shared<float>(2.2f));
    std::get<2>(qs).push_back(std::make_shared<double>(3.0e-9));
    std::get<3>(qs).push_back(std::make_shared<char>('a'));
    std::get<4>(qs).push_back(std::make_shared<std::string>("foo"));

    auto any_q = ifdcs_shqed.as_any_queue(qs);
    Assert(any_q.size() == 5);
    for (auto q : any_q) {
        Assert(q.size() == 1);
    }

    auto qs2 = ifdcs_shqed.from_any_queue(any_q);

    using boost::any_cast;

    std::cerr << "First element from each queue:\n";
    std::cerr << *std::get<0>(qs2)[0] << std::endl;
    std::cerr << *std::get<1>(qs2)[0] << std::endl;
    std::cerr << *std::get<2>(qs2)[0] << std::endl;
    std::cerr << *std::get<3>(qs2)[0] << std::endl;
    std::cerr << *std::get<4>(qs2)[0] << std::endl;

    // for (auto a: as_any(std::get<0>(qs))) {
    // 	std::cerr << boost::any_cast<int>(a) << std::endl;
    // }
    typedef typename IFDCS_helper::Wrapped<std::vector>::type ifdcs_vectors;
    ifdcs_vectors vs;
    std::get<0>(vs).push_back(1);
    std::get<1>(vs).push_back(2.2);
    std::get<2>(vs).push_back(3.0e-9);
    std::get<3>(vs).push_back('a');
    std::get<4>(vs).push_back(std::string("foo"));

    type_repeater<3, std::string>::type ahahah("one", "two", "three");
    std::cerr << std::get<0>(ahahah) << " " << std::get<1>(ahahah) << " " << std::get<2>(ahahah) << "\n";

    type_repeater<16, std::string>::type t16("0","1","2","3","4","5","6","7","8","9","a","b","c","d","e","f");
    std::cerr << std::get<0>(t16) << " " << std::get<15>(t16) << "\n";
                                             
    //type_repeater<128, std::string>::type mongo;

}
