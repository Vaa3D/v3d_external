#!/usr/bin/perl -w

# Use install name tool to correct local library references in v3d plugins

use strict;
use File::Copy;

my $usage = "Usage:\n  $0 <foo>.app\n" ;

my $app_name = shift;
die $usage unless $app_name;
die "Application $app_name does not exist" unless -e $app_name;

process_all_plugins($app_name);

sub process_all_plugins
{
    my $app_name = shift;
    my $dir = "$app_name/Contents/MacOS/plugins";
    my @plugins = glob("$dir/*.dylib $dir/*/*.dylib $dir/*/*/*.dylib");
    foreach my $plugin (@plugins) {
        process_one_library($plugin, $app_name);
    }
}

sub process_one_library 
{
    my $lib_file = shift;
    my $app_name = shift;

    open FH, "otool -L $lib_file |";
    my $line_num = 0;
    my $cmd = "install_name_tool";
    my $args = "";
    LIB: while (<FH>) {
        $line_num ++;
        # Skip header line and self entry
        next unless $line_num >= 3;
    
        my @fields = split;
        my $lib_name = $fields[0];
    
        # Skip entries that have already been modified
        next if $lib_name =~ /^\@executable_path/;
        # Skip system entries
        next if $lib_name =~ m!^/usr/lib/!;
    
        # Handle Qt entries
        if ( $lib_name =~ m!^(Qt\w+\.framework/.*/Qt\w+$)! ) {
            my $old = $1;
            my $new = "\@executable_path/../Frameworks/$old";
            $args .= " -change $old $new";
        }

        # Handle libtiff
        if ( $lib_name =~ m!^(.*/libtiff.*\.dylib)$! ) {
            my $old = $1;
            next unless -e $old;
            my @pathparts = split '/', $old;
            my $term_name = pop @pathparts;
            my $full_new = "$app_name/Contents/Frameworks/$term_name";
            unless (-e $full_new) {
                print("Copying $old to $full_new\n");
                copy($old, $full_new);
            }
            my $new = "\@executable_path/../Frameworks/$term_name";
            $args .= " -change $old $new";
        }

    }
    if (length($args) > 0) {
        system "$cmd $args $lib_file";
    }
}

