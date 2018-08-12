#!/bin/bash
killall SensorDaemon
DISPLAY=:0 x-terminal-emulator --geometry=38x4 -e /var/www/daemons/SensorDaemon & disown
sleep 1