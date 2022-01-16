#ifndef WIRECELL_RANGEFEED
#define WIRECELL_RANGEFEED

namespace WireCell {

    template <typename Iter>
    struct RangeFeed {
        typedef typename Iter::value_type value_type;
        Iter m_begin, m_end;
        RangeFeed(const Iter& begin, const Iter& end)
          : m_begin(begin)
          , m_end(end)
        {
        }
        value_type operator()()
        {
            if (m_begin == m_end) {
                return nullptr;
            }
            value_type ret = *m_begin;
            ++m_begin;
            return ret;
        }
    };

}  // namespace WireCell
#endif
