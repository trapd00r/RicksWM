#!/bin/sh

exec menuRK -square -ws sticky -t '%s...' -p 1 -fg white -bg blue -fn fixed \
  -geometry '-3-49' -cws cyan blue -clock <<END &

utilRK ws main     #main
utilRK ws code     #code
utilRK ws net      #net
utilRK ws pictures #pictures
utilRK ws TV       #TV
utilRK ws xchat    #xchat
utilRK ws golf     #golf
utilRK ws servers  #servers
utilRK ws sticky   #sticky

END
