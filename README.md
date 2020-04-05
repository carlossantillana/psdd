# psdd
FPGA implementation of hahaxD's implementation of psdd.

## Preq

#### first clone aws-fpga repo
https://github.com/aws/aws-fpga

and follow setup instructions found here
https://github.com/aws/aws-fpga/tree/master/SDAccel

the clone this repo psdd into path SDAccel/examples/xilinx_2018.3/getting_started of repo mentioned above

#### Download datasets from this link
https://drive.google.com/open?id=1dyMke7JY5UURVj4wGWxfSsAp3jExwYvw

grids.psdd, and grids.vtree is the small dataset

weighted_map_network.psdd and weighted_map_network.vtree is the large dataset.

And put them one level below root of this repo.


#### Make sure you have gcc 4.9.2 or greater installed.

If running on centos this can be done by running the two commands

`yum install devtoolset-9`

`source devtoolset-9`

#### Edit Makefiles to use your directory
Change $project_dir and $platform_dir to your paths.

This needs to be done for the three files: faux_make_sw_emu.sh, faux_make_hw_emu.sh,
and faux_make_hw.sh.

You will also have to change CMakeLists.txt line
`include_directories("/dev/project_data/aws-fpga/SDAccel/examples/xilinx_2018.3/libs/")`
to point to your path.

## How to Run
To run sw emulation
`./faux_make_sw_emu.sh`

To run hw emulation
`./faux_make_sw_emu.sh`

To run hw
`./faux_make_hw.sh`

To run make clean
`./faux_make_clean.sh`

### Common errors
if kernel hangs, or segfaults run command
ulimit -s  unlimited

### How to Change Datasets
To change between small and large datasets. You need to comment out the respective
 line starting with 'XCL_EMULATION_MODE=' in both hw_emu and sw_emu makefiles.

In addition you need to edit files include/psdd/psdd_node.h, and include/psdd/fpga_kernel_psdd_node.h

ctrl+f for //For map_network and make sure the lines below are not commented out for large network support

ctrl+f for //For grids network and make sure the lines below are not commented out for small network support-
