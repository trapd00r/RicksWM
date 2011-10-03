#!/bin/sh -v

perl -wc RicksWM || exit

sync; sync; sync

export PATH=$PATH:$PWD
export RWMPATH=$PWD
cd
exec xinit $RWMPATH/RicksWM -- :1 -ac
