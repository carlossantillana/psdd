#initialize to your local path
project_dir="$AWS_FPGA_REPO_DIR/Vitis/examples/xilinx_2019.2/psdd"
platform_dir="$AWS_FPGA_REPO_DIR/Vitis/aws_platform"

# creates .o of project
cmake .

make

#makes xcl bin
mkdir -p ./_x.hw.xilinx_aws-vu9p-f1_shell-v04261818_201920_1

#xocc compiles makes xo file
v++ -t hw  --optimize 3 --vivado.prop run.impl_1.STRATEGY=Performance_EarlyBlockPlacement   --platform $platform_dir/xilinx_aws-vu9p-f1_shell-v04261818_201920_1/xilinx_aws-vu9p-f1_shell-v04261818_201920_1.xpfm --save-temps  --temp_dir ./_x.hw.xilinx_aws-vu9p-f1_shell-v04261818_201920_1 -c -k fpga_evaluate -I'src' -o'_x.hw.xilinx_aws-vu9p-f1_shell-v04261818_201920_1/fpga_evaluate.xo' 'src/fpga_evaluate.cpp' -I$project_dir/include

#makes xcl bin
mkdir -p ./build_dir.hw.xilinx_aws-vu9p-f1_shell-v04261818_201920_1

#makes xclbin file
v++ -t hw --optimize 3 --vivado.prop run.impl_1.STRATEGY=Performance_EarlyBlockPlacement  --platform $platform_dir/xilinx_aws-vu9p-f1_shell-v04261818_201920_1/xilinx_aws-vu9p-f1_shell-v04261818_201920_1.xpfm --save-temps  --temp_dir ./build_dir.hw.xilinx_aws-vu9p-f1_shell-v04261818_201920_1 -l  -o'build_dir.hw.xilinx_aws-vu9p-f1_shell-v04261818_201920_1/fpga_evaluate.xclbin' _x.hw.xilinx_aws-vu9p-f1_shell-v04261818_201920_1/fpga_evaluate.xo
