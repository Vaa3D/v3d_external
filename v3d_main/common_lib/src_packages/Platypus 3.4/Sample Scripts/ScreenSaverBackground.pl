#!/usr/bin/perl
#
# This simple script is a rather cool demonstration of the power of Platypus.
# An app created with this Perl script will toggle on and off the hidden
# screen saver background feature in MacOS X.  
#
# *** Make sure not to run this if you have a screensaver password enabled ****
#
# Users of Mac OS 10.3:           Please note that this script disables ExposéŽ until your 
#                                 next reboot, due to a bug in Apple's ScreenSaverEngine.
#                                 This does not affect Tiger.

use Shell;

$matched = false;

@procs = `ps -cxa`;
for $proc (@procs )
{
	if ($proc =~ /ScreenSaverEngine/)
	{
		$matched = true;
		killall("ScreenSaverEngine");
	}
}

if ($matched eq false)
{
	system("/System/Library/Frameworks/ScreenSaver.framework/Versions/A/Resources/ScreenSaverEngine.app/Contents/MacOS/ScreenSaverEngine -background &");
}
