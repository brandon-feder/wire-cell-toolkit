#include "PMTNoiseROI.h"

#include <algorithm>
#include <cmath>

using namespace WireCell;

using namespace WireCell::SigProc;

PMTNoiseROI::PMTNoiseROI(int start_bin, int end_bin, int peak, int channel, float peak_height)
  : start_bin(start_bin)
  , end_bin(end_bin)
{
    peaks.push_back(peak);
    collection_wwires.push_back(channel);
    wwires_peak_heights[channel] = fabs(peak_height);
}

PMTNoiseROI::~PMTNoiseROI() {}

void PMTNoiseROI::insert_peak(int peak)
{
    if (find(peaks.begin(), peaks.end(), peak) == peaks.end()) peaks.push_back(peak);
}

void PMTNoiseROI::insert_uwires(int wire_no, float peak_height)
{
    if (find(induction_uwires.begin(), induction_uwires.end(), wire_no) == induction_uwires.end()) {
        induction_uwires.push_back(wire_no);
        uwires_peak_heights[wire_no] = peak_height;
    }
}

void PMTNoiseROI::insert_vwires(int wire_no, float peak_height)
{
    if (find(induction_vwires.begin(), induction_vwires.end(), wire_no) == induction_vwires.end()) {
        induction_vwires.push_back(wire_no);
        vwires_peak_heights[wire_no] = peak_height;
    }
}

float PMTNoiseROI::get_max_wwires_peak_height()
{
    float max = 0;
    for (int i = 0; i != int(sorted_col_wwires.size()); i++) {
        if (wwires_peak_heights[sorted_col_wwires.at(i)] > max) max = wwires_peak_heights[sorted_col_wwires.at(i)];
    }
    return max;
}

float PMTNoiseROI::get_average_wwires_peak_height()
{
    float ave = 0;
    float ave1 = 0;
    for (int i = 0; i != int(sorted_col_wwires.size()); i++) {
        ave += wwires_peak_heights[sorted_col_wwires.at(i)];
        ave1++;
    }
    if (ave1 > 0) {
        return ave / ave1;
    }
    else {
        return 0;
    }
}

float PMTNoiseROI::get_max_uwires_peak_height(int group)
{
    float max = 0;
    if (group < int(sorted_ind_uwires.size())) {
        for (int i = 0; i != int(sorted_ind_uwires.at(group).size()); i++) {
            if (uwires_peak_heights[sorted_ind_uwires.at(group).at(i)] > max)
                max = uwires_peak_heights[sorted_ind_uwires.at(group).at(i)];
        }
    }
    return max;
}

float PMTNoiseROI::get_max_vwires_peak_height(int group)
{
    float max = 0;
    if (group < int(sorted_ind_vwires.size())) {
        for (int i = 0; i != int(sorted_ind_vwires.at(group).size()); i++) {
            if (vwires_peak_heights[sorted_ind_vwires.at(group).at(i)] > max)
                max = vwires_peak_heights[sorted_ind_vwires.at(group).at(i)];
        }
    }
    return max;
}

float PMTNoiseROI::get_average_uwires_peak_height(int group)
{
    float ave = 0;
    float ave1 = 0;
    if (group < int(sorted_ind_uwires.size())) {
        for (int i = 0; i != int(sorted_ind_uwires.at(group).size()); i++) {
            ave += uwires_peak_heights[sorted_ind_uwires.at(group).at(i)];
            ave1++;
        }
    }

    if (ave1 > 0) {
        return ave / ave1;
    }
    else {
        return 0;
    }
}

float PMTNoiseROI::get_average_vwires_peak_height(int group)
{
    float ave = 0;
    float ave1 = 0;
    if (group < int(sorted_ind_vwires.size())) {
        for (int i = 0; i != int(sorted_ind_vwires.at(group).size()); i++) {
            ave += vwires_peak_heights[sorted_ind_vwires.at(group).at(i)];
            ave1++;
        }
    }
    if (ave1 > 0) {
        return ave / ave1;
    }
    else {
        return 0;
    }
}

bool PMTNoiseROI::merge_ROI(PMTNoiseROI& ROI)
{
    // decide how to merge two ROIs? ...

    // one peak is contained in the other's range then merge
    if ((peaks.at(0) >= ROI.get_start_bin() && peaks.at(0) <= ROI.get_end_bin()) ||
        (start_bin <= ROI.get_peaks().at(0) && end_bin >= ROI.get_peaks().at(0))) {
        if (ROI.get_start_bin() < start_bin) start_bin = ROI.get_start_bin();
        if (ROI.get_end_bin() > end_bin) end_bin = ROI.get_end_bin();

        for (int i = 0; i != int(ROI.get_peaks().size()); i++) {
            if (find(peaks.begin(), peaks.end(), ROI.get_peaks().at(i)) == peaks.end())
                peaks.push_back(ROI.get_peaks().at(i));
        }

        for (int i = 0; i != int(ROI.get_wwires().size()); i++) {
            if (find(collection_wwires.begin(), collection_wwires.end(), ROI.get_wwires().at(i)) ==
                collection_wwires.end()) {
                collection_wwires.push_back(ROI.get_wwires().at(i));
            }

            // calculate wire height
            if (wwires_peak_heights.find(ROI.get_wwires().at(i)) == wwires_peak_heights.end()) {
                wwires_peak_heights[ROI.get_wwires().at(i)] = ROI.get_wwires_peak_heights()[ROI.get_wwires().at(i)];
            }
            else {
                if (ROI.get_wwires_peak_heights()[ROI.get_wwires().at(i)] > wwires_peak_heights[ROI.get_wwires().at(i)])
                    wwires_peak_heights[ROI.get_wwires().at(i)] = ROI.get_wwires_peak_heights()[ROI.get_wwires().at(i)];
            }
        }

        //    for (int i=0;i!=ROI.get_wwires_peak_heights().size();i++){
        // wwires_peak_heights.push_back(ROI.get_wwires_peak_heights().at(i));
        //}

        return true;
    }

    return false;
}

void PMTNoiseROI::sort_wires(int nwire)
{
    std::vector<int> temp_wires;
    if (int(induction_uwires.size()) >= nwire) {
        std::sort(induction_uwires.begin(), induction_uwires.end());
        temp_wires.push_back(induction_uwires.at(0));
        // do u wires;
        for (int i = 1; i < int(induction_uwires.size()); i++) {
            if (induction_uwires.at(i) - temp_wires.back() == 1) {
                temp_wires.push_back(induction_uwires.at(i));
            }
            else {
                if (int(temp_wires.size()) >= nwire) {
                    sorted_ind_uwires.push_back(temp_wires);
                }
                temp_wires.clear();
                temp_wires.push_back(induction_uwires.at(i));
            }
        }
        if (int(temp_wires.size()) >= nwire) {
            sorted_ind_uwires.push_back(temp_wires);
        }
        temp_wires.clear();
    }

    if (int(induction_vwires.size()) >= nwire) {
        std::sort(induction_vwires.begin(), induction_vwires.end());
        temp_wires.push_back(induction_vwires.at(0));
        // do u wires;
        for (int i = 1; i < int(induction_vwires.size()); i++) {
            if (induction_vwires.at(i) - temp_wires.back() == 1) {
                temp_wires.push_back(induction_vwires.at(i));
            }
            else {
                if (int(temp_wires.size()) >= nwire) {
                    sorted_ind_vwires.push_back(temp_wires);
                }
                temp_wires.clear();
                temp_wires.push_back(induction_vwires.at(i));
            }
        }
        if (int(temp_wires.size()) >= nwire) {
            sorted_ind_vwires.push_back(temp_wires);
        }
        temp_wires.clear();
    }

    if (int(collection_wwires.size()) >= nwire) {
        std::sort(collection_wwires.begin(), collection_wwires.end());
        temp_wires.push_back(collection_wwires.at(0));
        // do u wires;
        for (int i = 1; i < int(collection_wwires.size()); i++) {
            if (collection_wwires.at(i) - temp_wires.back() == 1) {
                temp_wires.push_back(collection_wwires.at(i));
            }
            else {
                if (int(temp_wires.size()) >= nwire) {
                    for (int j = 0; j != int(temp_wires.size()); j++) {
                        sorted_col_wwires.push_back(temp_wires.at(j));
                    }
                }
                temp_wires.clear();
                temp_wires.push_back(collection_wwires.at(i));
            }
        }
        if (int(temp_wires.size()) >= nwire) {
            for (int j = 0; j != int(temp_wires.size()); j++) {
                sorted_col_wwires.push_back(temp_wires.at(j));
            }
        }
        temp_wires.clear();
    }
}
