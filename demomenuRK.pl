#!/usr/bin/perl

# Copyright (C) 2007-2008 Rick Klement
$putilRK::VERSION = '$Revision: 1.6 $';

use strict;

my $menu = -x './menuRK' ? './menuRK' : 'menuRK';

my @basic = split /\n+/, <<END;
$0 #Basic Menu
$0 blue #Basic Blue
$0 fixed #Fixed Font
$0 left #Left Justify
$0 center #Center Justify
$0 right #Right Justify
$0 hpad #hpad to 100
$0 ipad #ipad to 25
$0 mr #mr to 7 rows
$0 midscreen #Midscreen
$0 north #North Placement
$0 east #East Placement
$0 south #South Placement
$0 west #West Placement
$0 3d #3d mode
$0 task #task mode
$0 notice #notice mode
$0 oval #Oval buttons
$0 fade #Fade decoration
$0 ovalfade #Oval and Fade
$0 or #OvalFade&shape
$0 onlyshape #Shape no override
$0 fitx #fitx
$0 ifitx #ifitx
$0 ifitxfade #ifitx fade
$0 wide #Max Width
$0 across #Across fill-in
$0 dial #Dial (Round) menu
$0 round #Rounded End
$0 roundfade #Fade Rounded End
$0 roundfadeshape #Fade Rounded Shape
$0 glop #Cluster
$0 solidglop #SolidCluster
 #Exit
END

my %options = (
left => ['-3d', '-left'],
center => ['-3d', '-center'],
right => ['-3d', '-right'],
hpad => ['-3d', '-center', '-hpad', 100],
ipad => ['-3d', '-center', '-ipad', 25],
mr => ['-3d', '-mr', 7],
across => ['-3d', '-mr', 6, '-across'],
'3d' => ['-3d'],
task => ['-task'],
notice => ['-notice', '#This', '#is', '#a', '#Notice',
  "$0 #OK", "$0 #Cancel", ''],
midscreen => ['-3d', '-midscreen'],
fitx => ['-3d', '-fitx', '-vpad', 0, '-or', '-wide', '-north',
  '-border', 0, '-shape', '-across'],
ifitx => ['-3d', '-center', '-ifitx', '-or', '-wide', '-north',
  '-border', 0, '-shape', '-across'],
ifitxfade => [qw(-center -ifitx -or -wide -north -border 0 -vpad 0
  -ipad 6 -hpad 20 -across -slide 0 -500
  -shape -fade -bg red -fg white -active black white -slantbg black)],
north => ['-3d', '-or', '-wide', '-north', '-border', 0, '-shape', '-across',
  '-slide', -2000, -1000],
east => [qw(-task -or -east -shape -vpad 9 -gap 0)],
south => ['-3d', '-or', '-wide', '-south', '-slantbg', 'black', '-across'],
west => [qw(-task -or -west -slide -500 0)],
wide => ['-3d', '-wide'],
vpad => ['-3d', '-vpad', 50],
slant => ['-3d', '-slant', 10],
depress => ['-3d', '-depress', 2],
oval => [qw(-oval -hpad 20 -ipad 10 -center -bg red -fg white -slantbg black)],
round => [qw(-round -hpad 10 -ipad 17 -center -gap 37 -vpad 27
  -bg red -fg white -slantbg black)],
roundfade => [qw(-fade -round -hpad 10 -ipad 17 -center -gap 37 -vpad 27
  -bg red -fg white -slantbg black)],
roundfadeshape => [qw(-shape -or -midscreen -fade -round -hpad 30 -ipad 17
  -center -gap 30 -vpad 40
  -bg red -fg white -active black yellow -slantbg black)],
glop => [qw(-shape -or -midscreen -fade -round
  -hpad 25 -ipad 30 -center -gap 10 -vpad 30
  -bg red -fg white -active black yellow -slantbg black)],
solidglop => [qw(-shape -or -midscreen -round
  -hpad 25 -ipad 30 -center -gap 10 -vpad 30
  -bg red -fg white -active black yellow -slantbg black)],
fade => [qw(-fade -bg red -fg white -active black cyan -slantbg black
  -center -ipad 15 -hpad 20)],
ovalfade => [qw(-midscreen -oval -fade -bg red -fg white
  -active black cyan -slantbg black -center -ipad 25 -hpad 20)],
'or' => [qw(-or -shape -midscreen -oval -fade -bg red -fg white
  -active black cyan -slantbg black -center -ipad 15 -hpad 20
    -gap 30 -vpad 30)],
shape => ['-shape'],
onlyshape => [qw(-shape -round -fade -hpad 25 -center -ipad 12
  -gap 10 -vpad 10
  -bg white -fg black -active white red -slantbg black)],
dial => [qw(-dial 350 -or -shape -midscreen -oval -fade
  -bg red -fg white -active black cyan -slantbg black
    -center -ipad 70 -hpad 10), @basic[0..10], ' #Exit', ''],
blue => [qw(-fg white -bg blue2 -mr 20 -midscreen)],
fixed => [qw(-center -fn fixed -midscreen -bg white -fg black)],
);

my $arg = shift;
#print "$0: arg $arg\n";
my @title = $arg ? ('-t', "Example of $arg") : ();

my @args = exists($options{$arg}) ? @{$options{$arg}} : ();
if(@args > 1 && $args[-1] eq '')
  {
  pop @args;
  @basic = ();
  }
exec $menu, @title, @args, @basic;

__END__
