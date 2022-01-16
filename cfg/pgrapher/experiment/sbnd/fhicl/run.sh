# source /cvmfs/sbnd.opensciencegrid.org/products/sbnd/setup_sbnd.sh;
# setup sbndcode v09_14_00 -q e19:prof

# export WIRECELL_PATH=`pwd`/wirecell:$WIRECELL_PATH

lar -n1 -c singlegen_muon.fcl -o gen.root
lar -n1 -c standard_g4_sbnd.fcl gen.root -o g4.root
lar -n1 -c standard_detsim_sbnd.fcl g4.root -o detsim.root
lar -n1 -c wcls-sp.fcl detsim.root -o reco.root

# lar -n1 -c celltree_sbnd.fcl detsim.root
# root -l -b -q dump_waveform.C
# 
# rm hist*root *log *db
