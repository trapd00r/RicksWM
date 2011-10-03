#!/usr/bin/perl

# gotoRK.pl - Copyright (C) 2006-2008 - Rick Klement
# $Revision: 1.10 $

use strict;
my %windows;
my %counts;
my $length = '';
my @args = (
  qw( -3d -slantbg gray60 -ws sticky -ipad 10 -midscreen -minw 50 -t) ,
  'Pick Workspace/Window or Drag & Drop',
  '-drop', 'ddRK.pl',
  );

chomp(my $ws = `utilRK listcurrentws`);
$counts{$ws} = undef;
$windows{$ws} = [];

open my $in, 'utilRK listwithws |' or die $!;

while(<$in>)
  {
  next unless /^(\S+ raise \d+ ws '(.*?)' #)\2: (.*)/;
  chomp;
  $counts{$2}++;
  push @{ $windows{$2} }, [$1, $3];
  $length |= $2;
  }
close $in;
$length = length $length;

my @rows = sort keys %windows;
my $rows = @rows;

push @args, '-c', $length, '-mr', $rows;

for (@rows)
  {
  push @args, "utilRK ws '$_' #$_\n";
  }

for (@rows)
  {
  push @args, qw(-center -push -fg white -bg blue4), '#:',
    qw(-pop -left);
  }

my $maxcount = (sort {$a <=> $b} values %counts)[-1];
for my $i (0..$maxcount-1)
  {
  for (@rows)
    {
    if($windows{$_}[$i])
      {
      push @args, qw(-push -fg red3 -active red3 gray80)
        if $windows{$_}[$i][1] =~ /VIM/;
      push @args, "$windows{$_}[$i][0]$windows{$_}[$i][1]";
      push @args, qw(-pop) if $windows{$_}[$i][1] =~ /VIM/;
      }
    else
      {
      push @args, "#";
      }
    }
  }
exec 'menuRK', @args;

__END__
