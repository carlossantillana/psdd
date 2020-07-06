cmake .

make

# ./psdd_inference ../weighted_map_network.psdd ../weighted_map_network.vtree  fpga_evaluate.awsxclbin

./psdd_inference ../mastermind.psdd  ../mastermind.vtree  fpga_evaluate.awsxclbin
