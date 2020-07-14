project_dir=$pwd
platform_dir="$AWS_FPGA_REPO_DIR/Vitis/aws_platform"
query="mpe_query"
kernel="fpga_evaluate"
if [ "$1" == "mar" ];
then
  query="mar_query"
  kernel="fpga_mar"

fi
if [ "$1" == "mpe" ];
then
  query="mpe_query"
  kernel="fpga_evaluate"
fi

# creates .o of project
cmake .

make

#runs xcpp
mkdir -p ./_x.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1


#xocc compiles makes xo file
v++ -t sw_emu --platform $platform_dir/xilinx_aws-vu9p-f1_shell-v04261818_201920_1/xilinx_aws-vu9p-f1_shell-v04261818_201920_1.xpfm --save-temps --advanced.prop kernel.$kernel.kernel_flags="-std=c++0x" -g --temp_dir ./_x.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1 -c -k $kernel -I'src' -o"_x.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1/$kernel.xo" "src/$kernel.cpp" -I$project_dir/include

#makes xcl bin
mkdir -p ./build_dir.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1
v++ -t sw_emu --platform $platform_dir/xilinx_aws-vu9p-f1_shell-v04261818_201920_1/xilinx_aws-vu9p-f1_shell-v04261818_201920_1.xpfm -R estimate --save-temps  -g --temp_dir ./build_dir.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1 -l  -o"build_dir.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1/$kernel.xclbin" _x.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1/$kernel.xo

#runs emconfigutil
emconfigutil --platform $platform_dir/xilinx_aws-vu9p-f1_shell-v04261818_201920_1/xilinx_aws-vu9p-f1_shell-v04261818_201920_1.xpfm  --od ./_x.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1

#runs program
#Large network
cp -rf ./_x.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1/emconfig.json .

# XCL_EMULATION_MODE=sw_emu ./psdd_inference  ../weighted_map_network.psdd  ../weighted_map_network.vtree ./build_dir.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1/$kernel.xclbin $query
#mastermind
XCL_EMULATION_MODE=sw_emu ./psdd_inference ../mastermind.psdd  ../mastermind.vtree ./build_dir.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_1/$kernel.xclbin $query
#small network
# XCL_EMULATION_MODE=sw_emu ./psdd_inference  ../grids.psdd  ../grids.vtree $query
