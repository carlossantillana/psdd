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
mkdir -p ./_x.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2


#xocc compiles makes xo file
v++ -t sw_emu --platform $platform_dir/xilinx_aws-vu9p-f1_shell-v04261818_201920_2/xilinx_aws-vu9p-f1_shell-v04261818_201920_2.xpfm --save-temps --advanced.prop kernel.$kernel.kernel_flags="-std=c++0x" -g --temp_dir ./_x.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2 -c -k $kernel -I'src' -o"_x.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2/$kernel.xo" "src/$kernel.cpp" -I$project_dir/include

#makes xcl bin
mkdir -p ./build_dir.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2
v++ -t sw_emu --platform $platform_dir/xilinx_aws-vu9p-f1_shell-v04261818_201920_2/xilinx_aws-vu9p-f1_shell-v04261818_201920_2.xpfm -R estimate --save-temps  -g --temp_dir ./build_dir.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2 -l  -o"build_dir.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2/$kernel.xclbin" _x.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2/$kernel.xo

#runs emconfigutil
emconfigutil --platform $platform_dir/xilinx_aws-vu9p-f1_shell-v04261818_201920_2/xilinx_aws-vu9p-f1_shell-v04261818_201920_2.xpfm  --od ./_x.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2

#runs program
#Large network
cp -rf ./_x.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2/emconfig.json .

# XCL_EMULATION_MODE=sw_emu ./psdd_inference  ../networks/weighted_map_network.psdd  ../networks/weighted_map_network.vtree ./build_dir.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2/$kernel.xclbin $query
#mastermind
XCL_EMULATION_MODE=sw_emu ./psdd_inference ../networks/mastermind.psdd  ../networks/mastermind.vtree ./build_dir.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2/$kernel.xclbin $query
#grid network
# XCL_EMULATION_MODE=sw_emu ./psdd_inference  ../networks/grids.psdd  ../networks/grids.vtree ./build_dir.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2/$kernel.xclbin $query
#blockmap
# XCL_EMULATION_MODE=sw_emu ./psdd_inference ../networks/blockmap.psdd  ../networks/blockmap.vtree ./build_dir.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2/$kernel.xclbin $query
#BN_78
# XCL_EMULATION_MODE=sw_emu ./psdd_inference ../networks/BN_78.psdd  ../networks/BN_78.vtree ./build_dir.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2/$kernel.xclbin $query
#fs-04
# XCL_EMULATION_MODE=sw_emu ./psdd_inference ../networks/fs-04.psdd  ../networks/fs-04.vtree ./build_dir.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2/$kernel.xclbin $query
#students
# XCL_EMULATION_MODE=sw_emu ./psdd_inference ../networks/students.psdd  ../networks/students.vtree ./build_dir.sw_emu.xilinx_aws-vu9p-f1_shell-v04261818_201920_2/$kernel.xclbin $query
