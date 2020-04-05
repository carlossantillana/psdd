# psdd
FPGA implementation of hahaxD's implementation of psdd.

# Preq.
Download dataset from this link
https://drive.google.com/open?id=1dyMke7JY5UURVj4wGWxfSsAp3jExwYvw
grids.psdd, and grids.vtree is the small dataset
weighted_map_network.psdd and weighted_map_network.vtree is the large dataset.
 and put them one level below root of this repo.

Make sure you have gcc 4.9.2 or greater


#Run
To run sw emulation
./faux_make_sw_emu.sh

To run hw emulation
./faux_make_sw_emu.sh

To run hw
./faux_make_sw.sh

#Common errors
run
ulimit -s  unlimited

#change datasets
