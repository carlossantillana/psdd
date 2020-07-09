query="mpe_query"
if [ "$1" == "mar" ];
then
  query="mar_query"
fi
if [ "$1" == "mpe" ];
then
  query="mpe_query"
fi

cmake .

make

# ./psdd_inference ../weighted_map_network.psdd ../weighted_map_network.vtree  fpga_evaluate.awsxclbin $query

./psdd_inference ../mastermind.psdd  ../mastermind.vtree  fpga_evaluate.awsxclbin $query
