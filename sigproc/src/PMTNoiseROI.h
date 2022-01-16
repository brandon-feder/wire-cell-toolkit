#ifndef WIRECELLSIGPROC_PMTNoiseROI_h
#define WIRECELLSIGPROC_PMTNoiseROI_h

#include <vector>
#include <map>

namespace WireCell {
    namespace SigProc {
        class PMTNoiseROI {
           public:
            PMTNoiseROI(int start_bin, int end_bin, int peak, int channel, float peak_height);
            ~PMTNoiseROI();

            int get_start_bin() { return start_bin; };
            int get_end_bin() { return end_bin; };

            void insert_peak(int peak);

            void insert_uwires(int wire_no, float peak_height);
            void insert_vwires(int wire_no, float peak_height);

            std::vector<int>& get_peaks() { return peaks; }

            std::vector<int>& get_uwires() { return induction_uwires; }
            std::vector<int>& get_vwires() { return induction_vwires; }
            std::vector<int>& get_wwires() { return collection_wwires; }

            std::map<int, float>& get_uwires_peak_heights() { return uwires_peak_heights; }
            std::map<int, float>& get_vwires_peak_heights() { return vwires_peak_heights; }
            std::map<int, float>& get_wwires_peak_heights() { return wwires_peak_heights; }

            float get_average_uwires_peak_height(int group);
            float get_average_vwires_peak_height(int group);

            float get_max_uwires_peak_height(int group);
            float get_max_vwires_peak_height(int group);

            float get_average_wwires_peak_height();
            float get_max_wwires_peak_height();

            std::vector<std::vector<int>>& get_sorted_uwires() { return sorted_ind_uwires; }
            std::vector<std::vector<int>>& get_sorted_vwires() { return sorted_ind_vwires; }
            std::vector<int>& get_sorted_wwires() { return sorted_col_wwires; }

            void sort_wires(int nwire = 1);
            bool merge_ROI(PMTNoiseROI& ROI);

           private:
            std::vector<int> peaks;
            int start_bin;
            int end_bin;

            std::map<int, float> uwires_peak_heights;
            std::map<int, float> vwires_peak_heights;
            std::map<int, float> wwires_peak_heights;

            std::vector<int> induction_uwires;
            std::vector<int> induction_vwires;
            std::vector<int> collection_wwires;

            std::vector<std::vector<int>> sorted_ind_uwires;
            std::vector<std::vector<int>> sorted_ind_vwires;
            std::vector<int> sorted_col_wwires;
        };
    }  // namespace SigProc
}  // namespace WireCell

#endif
