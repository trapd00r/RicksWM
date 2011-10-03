#!/usr/bin/perl

# edgeRK.pl - Copyright (C) 2007-2008 - Rick Klement
# $Revision: 1.14 $

use strict;
#my $program = '/home/rick/x11/wm/menuRK';
my $program = 'menuRK';

if($ARGV[0] eq 'right')
  {
  my @ws = qx(utilRK listnosticky);

  exec $program,
    qw(-or -ipad 3 -p 2 -timeout 2 -leave -c 20 -hpad 12 -vpad 0
      -slide 300 0 -3move -shrink -shape
      -fade -fg black -bg cyan -active white red -slantbg black),
    '-center', ' # Dismiss ',
    '-left', @ws,
    '-center', ' # Dismiss ';
  die "$0: exec failed $!";
  }
elsif($ARGV[0] eq 'bottom')
  {
  chomp(my $current = `utilRK listcurrentws`);
  my @ws = sort grep !/ ws '\Q$current\E'/, qx(utilRK listallws);

  exec $program,
    '-right', '#Workspace',
    qw(-or -geometry -2-0 -ipad 3 -p 2 -timeout 2 -leave
      -c 16 -wide -ifitx -slide 0 200 -shrink
      -center -fade -fg black -bg cyan -active white red -slantbg black),
    @ws,
    ' # Dismiss ';
  die "$0: exec failed $!";
  }
else
  {
  die "usage: $0 [right|bottom]\n";
  }

__END__
