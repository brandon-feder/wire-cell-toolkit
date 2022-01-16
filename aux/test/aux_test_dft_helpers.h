// This is only for sharing some common code betweeen different
// aux/test/*.cxx tests.  Not for "real" use.

#include "WireCellUtil/NamedFactory.h"
#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/Exceptions.h"
#include "WireCellUtil/Persist.h"
#include "WireCellUtil/CLI11.hpp"

#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IDFT.h"

#include <ctime>                // std::clock
#include <chrono>

#include <string>
#include <cassert>
#include <fstream>
#include <iostream>

// note: likely will move in the future.
#include "custard/nlohmann/json.hpp"

namespace WireCell::Aux::Test {

    using object_t = nlohmann::json;

    // probably move this to util
    struct Stopwatch {
        using clock = std::chrono::high_resolution_clock;
        using time_point = clock::time_point;
        using function_type = std::function<void()>;

        std::clock_t c_ini = std::clock();
        time_point t_ini = clock::now();

        object_t results;

        Stopwatch(const object_t& first = object_t{}) {
            (*this)([](){}, first);
        }

        // Run the func, add timing info to a "stopwatch" attribute of
        // data and save data to results.  A pair of clock objects are
        // saved, "clock" (std::clock) and "time" (std::chrono).  Each
        // have "start" and "elapsed" which are the number of
        // nanoseconds from creation of stopwatch and for just this
        // job, respectively.
        void operator()(function_type func, object_t data = object_t{})
        {
            auto c_now =std::clock();
            auto t_now = clock::now();
            func();
            auto c_fin =std::clock();
            auto t_fin = clock::now();
            
            double dc_now = 1e9 * (c_now - c_ini) / ((double) CLOCKS_PER_SEC);
            double dc_fin = 1e9 * (c_fin - c_now) / ((double) CLOCKS_PER_SEC);
            double dt_now = std::chrono::duration_cast<std::chrono::nanoseconds>(t_now - t_ini).count();
            double dt_fin = std::chrono::duration_cast<std::chrono::nanoseconds>(t_fin - t_now).count();

            data["stopwatch"]["clock"]["start"] = dc_now;
            data["stopwatch"]["clock"]["elapsed"] = dc_fin;
            data["stopwatch"]["time"]["start"] = dt_now;
            data["stopwatch"]["time"]["elapsed"] = dt_fin;

            results.push_back(data);
        }        

        void save(const std::string& jsonfile) {
            std::ofstream fp(jsonfile.c_str());
            fp << results.dump(4) << std::endl;
        }
            

    };

    // fixme: add support for config
    IDFT::pointer make_dft(const std::string& tn="FftwDFT",
                           const std::string& pi="WireCellAux",
                           Configuration cfg = Configuration())
    {
        std::cerr << "Making DFT " << tn << " from plugin " << pi << std::endl;

        PluginManager& pm = PluginManager::instance();
        pm.add(pi);
        
        // create first
        auto idft = Factory::lookup_tn<IDFT>(tn);
        assert(idft);
        // configure before use if configurable
        auto icfg = Factory::find_maybe_tn<IConfigurable>(tn);
        if (icfg) {
            auto def_cfg = icfg->default_configuration();
            def_cfg = update(def_cfg, cfg);
            icfg->configure(def_cfg);
        }
        return idft;
    }
    struct DftArgs {
        std::string tn{"FftwDFT"};
        std::string pi{"WireCellAux"};
        std::string cfg_name{""};
        std::string output{""};
        Configuration cfg;
    };        

    // remove command name from main()'s argc/argv[0]
    int make_dft_args(DftArgs& args, int argc, char** argv) 
    {
        CLI::App app{"wct dft test"};
        app.add_option("-o,--output", args.output,
                   "Output file")->type_size(1)->allow_extra_args(false);
        app.add_option("-p,--plugin", args.pi,
                   "Plugin")->type_size(1)->allow_extra_args(false);
        app.add_option("-t,--typename", args.tn,
                   "Type or Type:Name of IDFT imp")->type_size(1)->allow_extra_args(false);
        app.add_option("-c,--config", args.cfg_name,
                   "Config file for IDFT imp")->type_size(1)->allow_extra_args(false);
        CLI11_PARSE(app, argc, argv);

        if (not args.cfg_name.empty()) {
            // Either we get directly a "data" object 
            auto cfg = Persist::load(args.cfg_name);
            // or we go searching a list for matching type/name.
            if (cfg.isArray()) {
                for (auto one : cfg) {
                    std::string tn = get<std::string>(one, "type");
                    std::string n = get<std::string>(one, "name", "");
                    if (not n.empty()) {
                        tn = tn + ":" + n;
                    }
                    if (tn == args.tn) {
                        cfg = one["data"];
                        break;
                    }
                }
            }
            args.cfg = cfg;
        }
        return 0;
    }

    const double default_eps = 1e-8;
    const std::complex<float> czero = 0.0;
    const std::complex<float> cone = 1.0;

    void assert_small(double val, double eps = default_eps) {
        if (val < eps) {
            return;
        }
        std::stringstream ss;
        ss << "value " << val << " >= " << eps;
        std::cerr << ss.str() << std::endl;
        THROW(WireCell::ValueError() << errmsg{ss.str()});
    }

    // Assert the array has only value val at index and near zero elsewhere
    template <typename ValueType>
    void assert_impulse_at_index(const ValueType* vec, size_t size,
                                 size_t index=0, ValueType val = 1.0)
    {
        ValueType tot = 0;
        for (size_t ind=0; ind<size; ++ind) {
            auto v = vec[ind];
            if (ind == index) {
                v -= val;
            }
            tot += v;
            assert_small(std::abs(v));
        }
        assert_small(std::abs(tot));
    }

    // Same as above but with pass by vector 
    template <typename VectorType>
    void assert_impulse_at_index(const VectorType& vec, 
                                 size_t index=0, const typename VectorType::value_type& val = 1.0)
    {
        assert_impulse_at_index(vec.data(), vec.size(), index, val);
    }

    // Assert all values in array are near given val
    template <typename ValueType>
    void assert_flat_value(const ValueType* vec, size_t size, ValueType val = 1.0)
    {
        ValueType tot = 0;
        for (size_t ind=0; ind<size; ++ind) {
            auto v = vec[ind];
            tot += v;
            assert_small(std::abs(v - val));
        }
        assert_small(std::abs(std::abs(tot) - std::abs(val)*size));
    }

    // As above but pass by vector
    template <typename VectorType>
    void assert_flat_value(const VectorType& vec, const typename VectorType::value_type& val = 1.0)
    {
        assert_flat_value(vec.data(), vec.size(), val);
    }

    // Print eigen array
    template<typename array_type>
    void dump(std::string name, const array_type& arr)
    {
        std::cerr << name << ":(" << arr.rows() << "," << arr.cols() << ") row-major:" << arr.IsRowMajor << "\n";
    }


    // Like std::iota, but dummer
    template<typename ValueType>
    void iota(ValueType* vec, size_t size, ValueType start = 0)
    {
        for (size_t ind=0; ind<size; ++ind) {
            vec[ind] = start++;
        }
    }
}
