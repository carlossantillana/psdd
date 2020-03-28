# creates .o of project
make

#runs xcpp
/opt/Xilinx/SDx/2018.3.op2405991/bin/xcpp -Wall -O0 -g -std=c++0x -I/home/centos/src/project_data/aws-fpga/SDAccel/examples/xilinx/getting_started/cs-259-19f/psdd/include -I/opt/Xilinx/Vivado/2018.3.op2405991/include -L/home/centos/src/project_data/aws-fpga/SDAccel/examples/xilinx/getting_started/cs-259-19f/psdd/lib/linux -I/home/centos/src/project_data/aws-fpga/SDAccel/examples/xilinx/libs -I./src/ -I../../..//libs/xcl2 -I/opt/xilinx/xrt/include/ psdd_inference_main.cpp ../../..//libs/xcl2/xcl2.cpp -o psdd_inference -L/opt/xilinx/xrt/lib/ -lOpenCL -lpthread -L/home/centos/src/project_data/aws-fpga/SDAccel/examples/xilinx/getting_started/cs-259-19f/psdd/lib/linux -Wl,-rpath,/home/centos/src/project_data/aws-fpga/SDAccel/examples/xilinx/getting_started/cs-259-19f/psdd/lib/linux libpsdd.a -lsdd -lgmp -lOpenCL libxcl2.a

#makes xcl bin
mkdir -p xclbin

#Breaks here
#xocc
/opt/Xilinx/SDx/2018.3.op2405991/bin/xocc -c --xp "param:compiler.preserveHlsOutput=1" --xp "param:compiler.generateExtraRunData=true" --xp param:prop:kernel.EvaluateWithoutPointer.kernel_flags="-std=c++0x"  -s -k fpga_evaluate  -o xclbin/fpga_evaluate.sw_emu.xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xo -t sw_emu --platform /home/centos/src/project_data/aws-fpga/SDAccel/aws_platform/xilinx_aws-vu9p-f1-04261818_dynamic_5_0/xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xpfm ./src/fpga_evaluate.cpp   -I/home/centos/src/project_data/aws-fpga/SDAccel/examples/xilinx/getting_started/cs-259-19f/psdd/include

#runs emconfigutil
/opt/Xilinx/SDx/2018.3.op2405991/bin/emconfigutil --platform /home/centos/src/project_data/aws-fpga/SDAccel/aws_platform/xilinx_aws-vu9p-f1-04261818_dynamic_5_0/xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xpfm --nd 1

#runs program
XCL_EMULATION_MODE=sw_emu ./psdd_inference ../weighted_map_network.psdd  ../weighted_map_network.vtree
