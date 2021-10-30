#!/usr/bin/perl

# Use install name tool to correct local library references in v3d plugins

use strict;
use File::Copy;

my $usage = "Usage:\n  $0 <foo>.app\n" ;
my $app_name = shift;
die $usage unless $app_name;
die "Application $app_name does not exist" unless -e $app_name;


process_all_plugins($app_name);
process_one_library( "$app_name/Contents/MacOS/vaa3d",$app_name);

sub process_all_plugins
{
    my $app_name = shift;
    my $dir = "$app_name/Contents/MacOS/plugins";
    # devide into smaller groups, to avoid seg fault
    my @plugins1 = glob("$dir/*/*.dylib");
    my @plugins2 = glob("$dir/*/*/*.dylib");
    my @plugins3 = glob("$dir/*/*/*/*.dylib");
    my @plugins4 = glob("$dir/*/*/*/*/*.dylib");

    foreach my $plugin (@plugins1) {
      if (-f $plugin) {
        print "deploy $plugin \n";
        process_one_library($plugin, $app_name);
      }
    }
    foreach my $plugin (@plugins2) {
      if (-f $plugin) {
        print "deploy $plugin \n";
        process_one_library($plugin, $app_name);
      }
    }

    foreach my $plugin (@plugins3) {
      if (-f $plugin) {
        print "deploy $plugin \n";
        process_one_library($plugin, $app_name);
      }
    }
    foreach my $plugin (@plugins4) {
      if (-f $plugin) {
        print "deploy $plugin \n";
        process_one_library($plugin, $app_name);
      }
    }

  }

  sub process_one_library 
  {
    my $lib_file = shift;
    my $app_name = shift;
    if (-f $lib_file  ) {
      open FH, "otool -L $lib_file |";
    }
    my $line_num = 0;
    my $cmd = "install_name_tool";
    my $args = "";
    LIB: while (<FH>) {

      $line_num ++;
      # Skip header line and self entry
      next unless $line_num >= 3;

      my @fields = split;
      my $lib_name = $fields[0];

      # Handle libtiff first
      if ( $lib_name =~ m!^(.*/libtiff.*\.dylib)$! ) {
        my $old = $1;
        #force to update libtiff path
        #next unless -e $old;
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

      # Skip entries that have already been modified
      next if $lib_name =~ /^\@executable_path/;
      # Skip system entries
      next if $lib_name =~ m!^/usr/lib/!;

      #print "\n lib name :  ", $lib_name,"\n";
      # Handle Qt entries
      if ( $lib_name =~ m!(Qt\w+\.framework.*Qt\w+$)! ) {
        my $old = $1;
        my $new = "\@executable_path/../Frameworks/$old";
        $args .= " -change $lib_name $new";
        print "change", $lib_name, " to ",$new,"\n";
      }




    }
    if (length($args) > 0) {
      #print "$cmd $args $lib_file";
      system "$cmd $args $lib_file";
    }
  }
