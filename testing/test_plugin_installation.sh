#!/bin/bash
# This script aims to  1) detect missing plugins, according to the
# wish list (./pluginlist.txt)  2) test the help function for each plugin.
# 3) detect empty plugin folders.
# This is a useful sannity check on the inventory of released plugins for
# installer releases.


V3D="../bin/vaa3d"
plugin_path="../bin/plugins"

TESTING_PACKAGE=1

if [[ "$OSTYPE" == "darwin"* ]]; then
  V3D=../bin/vaa3d64.app/Contents/MacOS/vaa3d64
  plugin_path=../bin/plugins
  if [[ $TESTING_PACKAGE>0 ]]; then
    V3D=../Vaa3d.app/Contents/MacOS/vaa3d
    plugin_path=../plugins
  fi
fi

if [[ "$OSTYPE" == "cygwin" ]]; then
  V3D=./bin/vaa3d.exe
  plugin_path=./bin/plugins
  if [[ TESTING_PACKAGE>0 ]]; then
    V3D=../bin/vaa3d.exe
    plugin_path=../bin/plugins
  fi
fi

if ! [ -e "$plugin_path" ];then
    echo "$plugin_path does not exist"
    echo "please input plugin path:"
    read input
    plugin_path=$input
fi

## detect empy plugin folders
echo "Checking for empty plugin folders in $plugin_path:"
command="find  $plugin_path  -type d -empty"
output=$($command 2>&1 )
if  ! [ -z  $output ]; then
    echo "Empty plugin folders found!"
    echo "${output}"
   # find  $plugin_path  -type d -empty -delete
else
    echo "No empty plugin folders!";
fi

echo ""
echo ""
echo ""

if ! [ -e $V3D ]; then
    echo "$V3D does not exsit"
    echo "please input vaa3d executable path:"
    read input
    V3D=$input
fi

echo "Start to check each plugin listed in ./pluginlist.txt:"
echo $V3D

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
  echo ""
  echo "$count: testing $plugin"
  if [[ "$OSTYPE" == "cygwin" ]]; then
    command="$V3D /h /x $plugin /f help"
    echo $command
  else
    command="$V3D -h -x $plugin -f help"
    echo $command
  fi


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
