#initialize to your local path
project_dir="/dev/project_data/aws-fpga/SDAccel/examples/xilinx_2018.3/getting_started/cs-259-19f/psdd"
platform_dir="/dev/project_data/aws-fpga/SDAccel/aws_platform"
# creates .o of project
cmake .

make

#runs xcpp
/opt/Xilinx/SDx/2018.3.op2405991/bin/xcpp -Wall -O0 -g -std=c++0x -I$project_dir/include -I/opt/Xilinx/Vivado/2018.3.op2405991/include -L$project_dir/lib/linux -I$project_dir/../../libs/  -I./src/ -I../../libs/xcl2 -I/opt/xilinx/xrt/include/ src/psdd_inference_main.cpp ../../libs/xcl2/xcl2.cpp -o psdd_inference -L/opt/xilinx/xrt/lib/ -lOpenCL -lpthread  -Wl,-rpath,$project_dir/lib/linux libpsdd.a -lsdd -lgmp -lOpenCL libxcl2.a

#makes xcl bin
mkdir -p xclbin

#xocc compiles makes xo file
/opt/Xilinx/SDx/2018.3.op2405991/bin/xocc -c --xp "param:compiler.preserveHlsOutput=1" --xp "param:compiler.generateExtraRunData=true" --xp param:prop:kernel.fpga_evalute.kernel_flags="-std=c++0x" -R estimate -s -k fpga_evaluate  -o xclbin/fpga_evaluate.hw_emu.xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xo -t hw_emu --platform $platform_dir/xilinx_aws-vu9p-f1-04261818_dynamic_5_0/xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xpfm ./src/fpga_evaluate.cpp   -I$project_dir/include

#makes xcl bin
mkdir -p xclbin

#makes xclbin file
/opt/Xilinx/SDx/2018.3.op2405991/bin/xocc -l --xp "param:compiler.preserveHlsOutput=1" --xp "param:compiler.generateExtraRunData=true"  --xp param:prop:kernel.fpga_evalute.kernel_flags="-std=c++0x" -R estimate -s -o xclbin/fpga_evaluate.hw_emu.xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xclbin -t hw_emu --platform $platform_dir/xilinx_aws-vu9p-f1-04261818_dynamic_5_0/xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xpfm xclbin/fpga_evaluate.hw_emu.xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xo

#runs emconfigutil
/opt/Xilinx/SDx/2018.3.op2405991/bin/emconfigutil --platform $platform_dir/xilinx_aws-vu9p-f1-04261818_dynamic_5_0/xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xpfm --nd 1

#runs program

#Large network
XCL_EMULATION_MODE=hw_emu ./psdd_inference ../weighted_map_network.psdd  ../weighted_map_network.vtree
#small network
# XCL_EMULATION_MODE=hw_emu ./psdd_inference ../grids.psdd  ../grids.vtree
