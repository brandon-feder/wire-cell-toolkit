#ifndef WIRECELLUTIL_INDEXEDSET
#define WIRECELLUTIL_INDEXEDSET

#include <unordered_map>
#include <vector>

namespace WireCell {

    /** IndexedSet - maintain a collection of exactly one thing in a
     * first added order */

    template <class TYPE>
    class IndexedSet {
       public:
        // the passed objects in order of first seen

        typedef std::unordered_map<TYPE, int> index_type;
        typedef std::vector<TYPE> collection_type;
        typedef typename collection_type::size_type size_type;

        index_type index;
        collection_type collection;
        size_type size() { return collection.size(); }

        int operator()(const TYPE& obj) const
        {
            auto mit = index.find(obj);
            if (mit != index.end()) {
                return mit->second;
            }
            return -1;
        }
        int operator()(const TYPE& obj)
        {
            auto mit = index.find(obj);
            if (mit != index.end()) {
                return mit->second;
            }
            int index_number = collection.size();
            index[obj] = index_number;
            collection.push_back(obj);
            return index_number;
        }

        bool has(const TYPE& obj)
        {
            auto mit = index.find(obj);
            return mit != index.end();
        }
    };

}  // namespace WireCell

#endif
