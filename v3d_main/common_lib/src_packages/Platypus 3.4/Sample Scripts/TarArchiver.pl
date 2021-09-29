#!/usr/bin/perl
#
# This script will create a gzipped tar archive on your Desktop
# from the files dropped on it.  Make sure to set it as droppable, 
# and output type to Progress Bar for some style. 
#

$cnt = 0;

$cmd = "/usr/bin/tar cvfz ~/Desktop/archive.tgz ";

# loop through list of files dropped
foreach(@ARGV)
{
	if ($cnt != 0) # We ignore the first argument (the app bundle path)
	{
		# add each file in turn
		$cmd .= "'$_' ";
	}
	$cnt++;
}

system($cmd);