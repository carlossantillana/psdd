#initialize to your local path
project_dir="$AWS_FPGA_REPO_DIR/Vitis/examples/xilinx_2019.2/psdd"
platform_dir="$AWS_FPGA_REPO_DIR/Vitis/aws_platform"

# creates .o of project
cmake .

make

#runs xcpp
mkdir -p ./_x.hw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1


#xocc compiles makes xo file
v++ -t hw_emu --platform $platform_dir/xilinx_aws-vu9p-f1_shell-v04261818_201920_1/xilinx_aws-vu9p-f1_shell-v04261818_201920_1.xpfm --save-temps  -g --temp_dir ./_x.hw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1 -c -k fpga_evaluate -I'src' -o'_x.hw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1/fpga_evaluate.xo' 'src/fpga_evaluate.cpp'   -I$project_dir/include

#makes xcl bin
mkdir -p ./build_dir.hw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1
v++ -t hw_emu --platform $platform_dir/xilinx_aws-vu9p-f1_shell-v04261818_201920_1/xilinx_aws-vu9p-f1_shell-v04261818_201920_1.xpfm --save-temps  -g --temp_dir ./build_dir.hw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1 -l  -o'build_dir.hw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1/fpga_evaluate.xclbin' _x.hw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1/fpga_evaluate.xo

#runs emconfigutil
emconfigutil --platform $platform_dir/xilinx_aws-vu9p-f1_shell-v04261818_201920_1/xilinx_aws-vu9p-f1_shell-v04261818_201920_1.xpfm --od ./_x.hw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1

#runs program
#Large network
cp -rf ./_x.hw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1/emconfig.json .

#map_network
# XCL_EMULATION_MODE=hw_emu ./psdd_inference ../weighted_map_network.psdd  ../weighted_map_network.vtree ./build_dir.hw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1/fpga_evaluate.xclbin

#mastermind
XCL_EMULATION_MODE=hw_emu ./psdd_inference ../mastermind.psdd  ../mastermind.vtree ./build_dir.hw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1/fpga_evaluate.xclbin

#small network
# XCL_EMULATION_MODE=hw_emu ./psdd_inference ../grids.psdd  ../grids.vtree
