#!/bin/bash
DISPLAY=:0 x-terminal-emulator --geometry=60x5 -e /var/www/daemons/SQLDaemon & disown
sleep 1