#initialize to your local path
project_dir="$AWS_FPGA_REPO_DIR/SDAccel/examples/xilinx_2019.1/getting_started/psdd"
platform_dir="$AWS_FPGA_REPO_DIR/SDAccel/aws_platform"

# creates .o of project
cmake .

make

#runs xcpp
/opt/Xilinx/SDx/2019.1.op2552052/bin/xcpp -Wall -O3 -g -std=c++0x -I$project_dir/include -I/opt/Xilinx/Vivado/2019.1.op2552052/include -L$project_dir/lib/linux -I$project_dir/../../libs/  -I./src/ -I../../libs/xcl2 -I/opt/xilinx/xrt/include/ src/psdd_inference_main.cpp ../../libs/xcl2/xcl2.cpp -o psdd_inference -L/opt/xilinx/xrt/lib/ -lOpenCL -lpthread  -Wl,-rpath,$project_dir/lib/linux libpsdd.a -lsdd -lgmp -lOpenCL libxcl2.a

#makes xcl bin
mkdir -p xclbin

#xocc compiles makes xo file
/opt/Xilinx/SDx/2019.1.op2552052/bin/xocc -c -O3 --kernel_frequency "0:100" --xp "param:compiler.preserveHlsOutput=1" --xp "param:compiler.generateExtraRunData=true" --xp param:prop:kernel.fpga_evalute.kernel_flags="-std=c++0x"  -s -k fpga_evaluate  -o xclbin/fpga_evaluate.hw.xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xo -t hw --platform $platform_dir/xilinx_aws-vu9p-f1-04261818_dynamic_5_0/xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xpfm ./src/fpga_evaluate.cpp   -I$project_dir/include

#makes xcl bin
mkdir -p xclbin

#makes xclbin file
/opt/Xilinx/SDx/2019.1.op2552052/bin/xocc -l -O3 --kernel_frequency "0:100" --xp "param:compiler.preserveHlsOutput=1" --xp "param:compiler.generateExtraRunData=true"  --xp param:prop:kernel.fpga_evaluate.kernel_flags="-std=c++0x" -s -o xclbin/fpga_evaluate.hw.xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xclbin -t hw --platform $platform_dir/xilinx_aws-vu9p-f1-04261818_dynamic_5_0/xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xpfm xclbin/fpga_evaluate.hw.xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xo

/opt/Xilinx/SDx/2019.1.op2552052/bin/xsltproc --stringparam xpath "spirit:component/spirit:name/text()" /opt/Xilinx/SDx/2019.1.op2552052/scripts/xdcc/xpathValueOf.xsl $project_dir/_x/link/sys_link/iprepo/xilinx_com_hls_fpga_evaluate_1_0/component.xml
