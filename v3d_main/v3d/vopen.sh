#! /bin/bash
if [ "$#" = "0" ]; then v3d; exit 0; fi

while [ "$#" != "0" ]
do
	file=$1
	ft=`echo $file | sed "s/^.*\.//"`
	if [ "$ft" = "raw" -o "$ft" = "tiff" -o "$ft" = "tif" -o "$ft" = "lsm" ]; then echo "RAWIMG=$file"; fi
	if [ "$ft" = "swc" ]; then echo "SWCFILE=$file"; fi
	if [ "$ft" = "apo" ]; then echo "APOFILE=$file"; fi
	if [ "$ft" = "marker" ]; then echo "SURFFILE=$file"; fi
	shift
done > .output.ano
v3d .output.ano
rm .output.ano
