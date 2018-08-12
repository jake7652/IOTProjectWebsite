#!/bin/bash
killall SQLDaemon
DISPLAY=:0 x-terminal-emulator --geometry=42x4 -e /var/www/daemons/SQLDaemon & disown
sleep 1