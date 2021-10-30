cd ~/work/v3d_external/
rm -fr bin/*
svn up
build.macx -m -j4 -B

cd ../vaa3d_tools/hackathon/zhi/neuron_comparison_script/
sh batch_build_tracing_methods.sh

cd ~/work/vaa3d_tools/bigneuron_ported
sh build_bigneuron_plugins_v2.940.sh

cd ~/work/v3d_external
v3d -h

