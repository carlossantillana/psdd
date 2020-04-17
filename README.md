# psdd
FPGA implementation of hahaxD's implementation of psdd.

## Preq

#### First clone aws-fpga repo
https://github.com/aws/aws-fpga

by following the setup instructions found here
https://github.com/aws/aws-fpga/tree/master/Vitis

**NOTE: you need to source sdaccel_setup.sh every time you start up your instance**

**Then clone this repo psdd into path SDAccel/examples/xilinx_2018.3/getting_started of repo mentioned above**
if not you will have to edit the makefiles even more

#### Download datasets from this link
https://drive.google.com/open?id=1dyMke7JY5UURVj4wGWxfSsAp3jExwYvw

grids.psdd, and grids.vtree is the small dataset

weighted_map_network.psdd and weighted_map_network.vtree is the large dataset.

Place them one level below root of this repo.

#### Make sure you have gcc 4.9.2 or greater installed.

If running on centos this can be done by running the two commands

`yum install devtoolset-9`

`source scl_source enable devtoolset-9`

#### Make sure you have cmake installed

https://gist.github.com/1duo/38af1abd68a2c7fe5087532ab968574e

#### Make sure you have gmp installed

https://serverfault.com/questions/855344/how-to-install-enable-gmp-math-functions-for-php-on-centos

## How to Run
To run sw emulation
`./faux_make_sw_emu.sh`

To run hw emulation
`./faux_make_hw_emu.sh`

To run hw
`./faux_make_hw.sh`

To run make clean
`./faux_make_clean.sh`

### Common errors
if kernel hangs, or segfaults run command
ulimit -s  unlimited

### How to Change Datasets
To change between small and large datasets. There will be two lines  starting with 'XCL_EMULATION_MODE=' in both `./faux_make_sw_emu.sh` and `faux_make_hw_emu`. Comment out the one your don't want to run and comment in the one you do want.

In addition you need to edit files include/psdd/psdd_node.h, and include/psdd/fpga_kernel_psdd_node.h

ctrl+f for `//For map_network` and make sure the lines below are not commented out for large network support

ctrl+f for `//For grids network` and make sure the lines below are not commented out for small network support-
