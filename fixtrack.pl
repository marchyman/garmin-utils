#!/usr/bin/perl -w

# $snafu: fixtrack.pl,v 1.1 2001/05/02 00:34:51 marc Exp $

# Script to convert timestamps in tracklogs from the old style found
# in versions 1.0-1.2 [yymmddhhmss] to the new style in version 1.3
# [yyyy-mm-dd hh:mm:ss]

use strict;

my $type = "";
my $str;

print "; converted from old garmin-utils file\n";
while (defined ($str = <STDIN>)) {
	chomp ($str);
	if (index ($str, "[") == 0) {
		if (index ($str, "[waypoints") == 0) {
			$type = "WPT";
		} elsif (index ($str, "[routes") == 0) {
			$type = "RTE";
		} elsif (index ($str, "[tracks") == 0) {
			$type = "TRK";
		} elsif (index ($str, "[end") == 0) {
			$type = "";
		}
		print "$str\n";
	} elsif ($type eq "TRK") {
		# format: <space>COORD1 COORD2 TIME [start]
		$str =~ /^ ([\d\+\-\.]*) *([\d\+\-\.]*) (\d*)(\d\d)(\d\d)(\d\d)(\d\d)(\d\d) *(start)*/;
		printf "%14.10f %15.10f %4.4d-%2.2d-$5 $6:$7:$8", $1, $2,
			$3 + 1900, $4 + 1;
		(defined ($9)) && print " start\n";	# start point
		(defined ($9)) || print "\n";		# otherwise
	} else {
		print "$str\n";
	}
}
