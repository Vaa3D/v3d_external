#!/usr/bin/perl
#
# Make sure you set this Perl script as droppable.  It loops
# through the @ARGV array and gzips the files passed to it via drop.
#
# You may want to set output to "Text Window" in order to see
# the app reporting as it gzips each file in turn
$cnt = 0;

# loop through list of files dropped
foreach(@ARGV)
{
	if ($cnt != 0) # We ignore the first argument (the app bundle path)
	{
		# gzip each file in turn
		print "Gzipping '$_'";
		system("gzip '$_'");
		
	}
	$cnt++;
}