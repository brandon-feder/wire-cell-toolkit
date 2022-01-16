#include "WireCellUtil/TagRules.h"

/* nb:
        typedef std::string tag_t;
        typedef std::string match_t;
        typedef std::unordered_set<tag_t> tagset_t;
        typedef std::pair<std::regex, tagset_t> rule_t;
        typedef std::vector<rule_t> ruleset_t;
 */

#include <regex>

using namespace WireCell;

tagrules::tagset_t tagrules::match(const tag_t& tag, const rule_t& rule)
{
    if (std::regex_match(tag, rule.first)) {
        return rule.second;
    }
    return tagset_t{};
}

bool tagrules::match(const tagrules::tag_t& tag, const tagrules::ruleset_t& rs, tagrules::tagset_t& ret, bool all)
{
    bool found = false;
    for (const auto& rule : rs) {
        auto ts = tagrules::match(tag, rule);
        if (ts.empty()) {
            continue;
        }
        ret.insert(ts.begin(), ts.end());
        if (all) {
            found = true;
            continue;
        }
        return true;
    }
    return found;
}

tagrules::tagset_t tagrules::transform(const tagrules::tagset_t& ts, const tagrules::ruleset_t& rs, bool all)
{
    tagrules::tagset_t ret;
    for (auto tag : ts) {
        tagrules::tagset_t one;
        if (match(tag, rs, one, all)) {
            ret.insert(one.begin(), one.end());
        }
    }
    return ret;
}

void tagrules::Context::configure(const Configuration& jcfg)
{
    if (jcfg.empty() or !jcfg.isArray()) {
        return;
    }

    const int nrss = jcfg.size();

    // Note: JSON is in name-major order, C++ index-major.
    for (int ind = 0; ind < nrss; ++ind) {
        auto jone = jcfg[ind];
        for (auto name : jone.getMemberNames()) {
            auto& rsv = m_rulesets[name];  // eg, "frame" or "trace"
            rsv.resize(nrss);
            rsv[ind] = convert<tagrules::ruleset_t>(jone[name]);
        }
    }
}

tagrules::tagset_t tagrules::Context::transform(size_t ind, const std::string& name, const tagrules::tag_t& tag)
{
    const auto& rsv = m_rulesets[name];
    if (rsv.empty() or ind >= rsv.size()) {
        return tagrules::tagset_t{};
    }
    const auto& rs = rsv[ind];
    tagrules::tagset_t ts;
    ts.insert(tag);
    return tagrules::transform(ts, rs);
}
