/* There is apparently a bug in at least oneapi-tbb-2021.3.0 which
 * uses "intptr_t" without declaring it.  Error found first in
 * test_tbb_max_threads.cxx:

In file included from ../../../../../opt/oneapi-tbb-2021.3.0/include/tbb/info.h:17,
                 from ../tbb/test/test_max_threads.cxx:1:
../../../../../opt/oneapi-tbb-2021.3.0/include/tbb/../oneapi/tbb/info.h:83:46: error: ‘intptr_t’ was not declared in this scope
   83 | unsigned __TBB_EXPORTED_FUNC core_type_count(intptr_t reserved = 0);
      |                                              ^~~~~~~~
../../../../../opt/oneapi-tbb-2021.3.0/include/tbb/../oneapi/tbb/info.h:84:67: error: ‘intptr_t’ has not been declared
   84 | void __TBB_EXPORTED_FUNC fill_core_type_indices(int* index_array, intptr_t reserved = 0);
      |                                                                   ^~~~~~~~
../../../../../opt/oneapi-tbb-2021.3.0/include/tbb/../oneapi/tbb/info.h:86:83: error: ‘intptr_t’ has not been declared
   86 | int __TBB_EXPORTED_FUNC constraints_default_concurrency(const d1::constraints& c, intptr_t reserved = 0);
      |                                                                                   ^~~~~~~~
../../../../../opt/oneapi-tbb-2021.3.0/include/tbb/../oneapi/tbb/info.h:87:80: error: ‘intptr_t’ has not been declared
   87 | int __TBB_EXPORTED_FUNC constraints_threads_per_core(const d1::constraints& c, intptr_t reserved = 0);

   * A work around is to add:

#include <cstdint>

   * which we do in that test.  This test thus will fail until TBB
   * fixes their bug. */
#warning "If this fails to build, it's a bug in TBB."
#include "tbb/info.h"

int main()
{
    return 0;
}
