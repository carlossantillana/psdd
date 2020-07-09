# psdd
FPGA implementation of hahaxD's implementation of psdd.

Instructions are specifically for aws-f1 instance using Vitis 2019.2

## Preq

#### First clone aws-fpga repo
https://github.com/aws/aws-fpga

by following the setup instructions found here
https://github.com/aws/aws-fpga/tree/master/Vitis

**NOTE: you need to source vitis_setup.sh every time you start up your instance**

**Then clone this repo psdd into path aws-fpga/Vitis/examples/xilinx_2019.2/ of repo mentioned above**
if not you will have to edit the makefiles even more

#### Download datasets from this link
https://drive.google.com/drive/folders/1ZaiVmhDfK5D0vXsOQIWdENW1qp8pNLqd?usp=sharing

grids.psdd, and grids.vtree is the small dataset

weighted_map_network.psdd and weighted_map_network.vtree is the large dataset.

mastermind.psdd and mastermind.vtree is a benchmark dataset.

Place them one level below root of this repo.

#### Make sure you have gcc 4.9.2 or greater installed.

If running on centos this can be done by running the two commands

`yum install devtoolset-9`

`source scl_source enable devtoolset-9`

#### Make sure you have cmake installed

https://gist.github.com/1duo/38af1abd68a2c7fe5087532ab968574e

#### Make sure you have gmp installed

yum install gmp-devel

## How to Run (Defaults to MPE)
To run sw emulation
`./faux_make_sw_emu.sh`

To run hw emulation
`./faux_make_hw_emu.sh`

To make hw
`./faux_make_hw.sh`

To run hw

`./run_hw.sh`

More instructions to run hw can be found here https://github.com/aws/aws-fpga/tree/master/Vitis

To run make clean
`./faux_make_clean.sh`

### Common errors
if kernel hangs, or segfaults run command
ulimit -s  unlimited

### How to Change Datasets
To change between small and large datasets. There will be two lines  starting with 'XCL_EMULATION_MODE=' in both `./faux_make_sw_emu.sh` and `faux_make_hw_emu`. Comment out the one your don't want to run and comment in the one you do want.

In addition you need to edit the file include/psdd/fpga_kernel_psdd_node.h

ctrl+f for `//For map_network` and make sure the lines below are not commented out for large network support

ctrl+f for `//For grids network` and make sure the lines below are not commented out for small network support

ctrl+f for `//For mastermind` and make sure the lines below are not commented out for mastermind network support

### How to Change Number of Queries.
edit: NUM_QUERIES variable found in `include/psdd/psdd_node.h`

### Supported Queries

* Most Probable Explanation (MPE):
  * `./faux_make_sw_emu.sh mpe`
  * `./faux_make_hw_emu.sh mpe`
  * `./faux_make_hw.sh `
  * `./run_hw.sh mpe`
* Marginals (MAR):
  * `./faux_make_sw_emu.sh mar`
  * `./faux_make_hw_emu.sh mar`
  * `./faux_make_hw.sh `
  * `./run_hw.sh mar`
