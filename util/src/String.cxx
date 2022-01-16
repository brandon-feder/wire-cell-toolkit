#include "WireCellUtil/String.h"

using namespace std;

std::vector<std::string> WireCell::String::split(const std::string& in, const std::string& delim)
{
    vector<string> chunks;
    if (in.empty()) {
        return chunks;
    }
    boost::split(chunks, in, boost::is_any_of(delim), boost::token_compress_on);
    return chunks;
}

std::pair<std::string, std::string> WireCell::String::parse_pair(const std::string& in, const std::string& delim)
{
    vector<string> chunks = split(in, delim);

    string first = chunks[0];
    string second = "";
    if (chunks.size() > 1) {
        second = chunks[1];
    }
    return make_pair(first, second);
}

// more:
// http://www.boost.org/doc/libs/1_60_0/doc/html/string_algo/quickref.html#idm45555128584624
