cmake .

make

# ./psdd_inference ../networks/weighted_map_network.psdd ../networks/weighted_map_network.vtree  fpga_evaluate.awsxclbin

./psdd_inference ../networks/mastermind.psdd  ../networks/mastermind.vtree  fpga_evaluate.awsxclbin
