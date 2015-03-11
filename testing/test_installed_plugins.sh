#!/bin/bash
# This script aims to  1) detect missing plugins, according to the
# wish list (./pluginlist.txt)  2) test the help function for each plugin.
# This is a useful sannity check on the inventory of released plugins for
# installer releases.

############################# Warning #############################
#only tested on mac so far! try it on other OSs at your own risk :p
############################# Warning #############################

if [[ "$OSTYPE" == "darwin"* ]]; then
  V3D=../bin/vaa3d64.app/Contents/MacOS/vaa3d64
fi

if [[ "$OSTYPE" == "win32" ]]; then
  V3D=../bin/vaa3d64
fi

if [[ "$OSTYPE" == "win32" ]]; then
  V3D=../bin/vaa3d64
fi

if ! [ -e $V3D ]; then
    echo "$V3D does not exsit"
    echo "please input vaa3d executable path:"
    read input
    V3D=$input
fi

echo "Start to check each plugin listed in ./pluginlist.txt:"

let count=0
let error_count=0

if ! [ -e pluginlist.txt ];then
     echo " could not find pluginlist.txt in the current directory"
     echo " please go to the testing directory first"
     exit
fi

for plugin in `cat pluginlist.txt `;
do
  ((count++))
  if [[ "$OSTYPE" == "win32" ]]; then
    command="$V3D \h -x $plugin -f help"
  else
    command="$V3D -h -x $plugin -f help"
  fi
  echo ""
  echo "$count: testing $plugin"

  output=$($command 2>&1 >/dev/null)
  if echo "$output" | grep -qi "Error"; then
    echo "error found!"
    echo "${output}"
    error_plugin_list="${error_plugin_list} ${plugin}"
   ((error_count++))
  else
    echo "pass!";
  fi
done;

echo "Checked $count released plugins."
if [ "$error_count" -gt 0 ]; then
  echo "Found errors running $error_count plugins: $error_plugin_list."
else
  echo "No errors found!"
fi
