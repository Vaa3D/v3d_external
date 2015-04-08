#!/bin/sh

MACOS_DIR=`dirname $0`

# Setting DYLD_FRAMEWORK_PATH allows us to avoid
# running install_name_tool on plugins.
export DYLD_FRAMEWORK_PATH=${MACOS_DIR}/../Frameworks

${MACOS_DIR}/NeuronAnnotator -na $@

