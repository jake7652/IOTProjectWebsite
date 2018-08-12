#!/bin/bash
export DISPLAY=:0
x-terminal-emulator --geometry=100x10 -e /var/www/daemons/CommandControlDaemon
exit 0