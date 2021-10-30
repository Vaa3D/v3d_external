#!/usr/bin/perl

# first developed by Lowell
# Revised by Hanchuan Peng, 20110814

use strict;
use Cwd;
use Getopt::Long;

# setup options;
# Ignore args with no options (eg, the list of files)
$Getopt::Long::passthrough = 1;
# Be case sensitive
$Getopt::Long::ignorecase = 0;
my $options = { };
GetOptions($options, "-H", "-help", "--help", "-i:s", "-f:s" ,"-o:s","-s:s","-debug");
my $USAGE = qq~
Usage:
        generate_stitch_wrapper.pl is a perl wrapper around v3d to blend and stitch single neuron imaging data.

        Example:  generate_stitch_wrapper.pl

        Set parameter files:
                -i          specify a slide group directory.
                -f          specify a slide group info file.
                -o          specify an output file for shell script.
                -debug      will execute script but write out shell script, will print instead
                -v          verbose mode to print out more statements.
~;

if ( $options->{'H'} || $options->{'-help'} || $options->{'help'}) {
    print $USAGE;
    exit 0;
}
my $slide_group_dir = "";
if ( $options->{'i'} ) {
    $slide_group_dir = $options->{'i'};
    $slide_group_dir =~ s/\/$//;
} else {
    print "Error, you need to specify -i option";
    print $USAGE;
    exit 1;
}

my $slide_group = "";
if ( $options->{'s'} ) {
    $slide_group = $options->{'s'};
} else {
    my @dirs = split(/\//,$slide_group_dir);
    $slide_group = pop(@dirs);
}

my $slide_info_file = "slide_group_info.txt";
if (  $options->{'f'} ) {
    $slide_info_file = $options->{'f'};
}

my $slide_info_file_path = $slide_group_dir . "/" . $slide_info_file;

unless (-e $slide_info_file_path) {
    print "Can't find $slide_info_file_path\n";
    exit 1;
}

my $stitched_dir = $slide_group_dir . "/stitched"; 
mkdir($stitched_dir);
chmod(0775,$stitched_dir);

my %lines;
my %regions;
open(IN, "$slide_info_file_path") || die "You do not have permissions to read this file\n";
while (my $in = <IN>) {
    chomp($in);
    #print "$in\n";
    my @info = split(/\t/, $in);

    # record line
    $lines{$info[2]} = 1;
    $info[1] =~ s/\s+/\_/g;
    $info[1] = lc($info[1]);

    # record region
    if ($regions{$info[1]}->{'images'}) {
	my $hr_images = $regions{$info[1]}->{'images'};
        $$hr_images{$info[0]}->{'line'} = $info[2];
        $$hr_images{$info[0]}->{'track_num'} = $info[3];
        $$hr_images{$info[0]}->{'channel_num'} = $info[4];
        $$hr_images{$info[0]}->{'channel_names'} = $info[5];
    } else {
	my %images;
	$images{$info[0]}->{'line'} = $info[2];
	$images{$info[0]}->{'track_num'} = $info[3];
	$images{$info[0]}->{'channel_num'} = $info[4];
	$images{$info[0]}->{'channel_names'} = $info[5];
	$regions{$info[1]}->{'images'} = \%images;
    }
}
close(IN);

#line check
my $line_name = "";
if (keys(%lines) > 1) {
    print "Error, more than one line found for this slide group\n";
    exit 1;
} else {
    foreach my $n (keys %lines) {
	$line_name = $n;
    }
}

#print "$line_name\n";

#my $v3d_cmd_path = "/groups/scicomp/jacsData/sampleData/stitchedData/v3d_linux_fc14/v3d";
#my $v3d_cmd_path = "/groups/peng/home/brainaligner/program/v3d/v3d";
my $v3d_cmd_path = "~/work/v3d_external/v3d/v3d";

my $shell_output = qq~\#!/bin/bash

#/usr/bin/Xvfb :99 -screen 0 1x1x24 -sp /usr/lib64/xserver/SecurityPolicy -fp /usr/share/X11/fonts/misc &
#MYPID=\$!
#export DISPLAY="localhost:99.0"

~;

foreach my $region_name (sort keys %regions) {
    print "$region_name\n";

    my $blend_raw = $line_name . "-" . $slide_group . "-" . $region_name . ".v3draw";

    my $hr_images = $regions{$region_name}->{'images'};    
    my $blend_cmd = "$v3d_cmd_path -x libblend_multiscanstacks.so -f multiscanblend -p \"#k 1\" -i"; 
    #order by smallest channel number. We can change this logic to use channel name or track later on
    foreach my $image (sort {$$hr_images{$a}->{'channel_num'} cmp $$hr_images{$b}->{'channel_num'}} keys %$hr_images) {
	print "\t$image $$hr_images{$image}->{'channel_num'} \n";
	$blend_cmd .= " '$slide_group_dir/$image'";
    }
    $blend_cmd .= " -o '$stitched_dir/$blend_raw'";
    $shell_output .= "$blend_cmd\;\n\n";

}

# here need to add a judgment call to ensure the availability of all 5 tiles and thus a final stitching would be needed.
# Noted by Hanchuan Peng, 20110815

my $stitch_raw = $line_name . "-" . $slide_group . "-stitched.v3draw";
$shell_output .= "$v3d_cmd_path -x imageStitch.so -f v3dstitch -i '$stitched_dir' -o '$stitched_dir/$stitch_raw' -p \"#c 4\"\;\n";

$shell_output .= qq~
# We're done with the frame buffer
#kill -9 \$MYPID
    ~;

my $output_sh = "blend_stitch.sh";
if ($options->{'o'}) {
    $output_sh = $options->{'o'};
}

my $shell_path = $stitched_dir . "/" . $output_sh;
open (OUT,">$shell_path") || die "cant write $shell_path\n";
print OUT "$shell_output";
close(OUT);
chmod (0755,$shell_path);
exit;

sub numerically {
    $a <=> $b;
}
