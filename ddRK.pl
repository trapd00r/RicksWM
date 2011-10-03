#!/usr/bin/perl

# ddRK.pl - Copyright (C) 2007-2008 - Rick Klement
# $Revision: 1.6 $

use strict;

@ARGV == 2 or die "usage: $0 <fromtext> <totext>\n";

#print "from: $ARGV[0]\n";
#print "  to: $ARGV[1]\n";

# find window ID

my ($wid, $oldws) = $ARGV[0] =~ / raise (\d+) ws '(.*?)'/;

# find workspace for it

my ($tid, $ws);

($tid, $ws) = $ARGV[1] =~ / raise (\d+) ws '(.*?)'/;
($ws) = $ARGV[1] =~ / ws '(.*?)'/ unless $ws;

if($wid and $ws and ($ws ne $oldws || $wid ne $tid))
  {
  system "utilRK movetows $wid '$ws'" if $ws ne $oldws;
  system "utilRK raiseover $wid $tid" if $tid;

  #system 'menuRK -3d -timeout 1 -midscreen working...';

  for (1..10)
    {
    select undef, undef, undef, $_ / 100;
    `utilRK listwithws` =~ /raise $wid ws '$ws'/ and last;
    }
  }

exec 'gotoRK.pl';
