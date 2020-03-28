#make clean
make clean
rm -rf psdd_inference xclbin/{*sw_emu*,*hw_emu*} sdaccel_* TempConfig system_estimate.xtxt *.rpt
rm -rf src/*.ll _xocc_* .Xil emconfig.json  dltmp* xmltmp* *.log *.jou *.wcfg *.wdb
