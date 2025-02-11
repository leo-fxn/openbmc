#!/bin/bash -e

FAN_MANUAL_MODE_ENV_FILE="/home/root/.fan_manual_mode.env"

if [ ! -f "$FAN_MANUAL_MODE_ENV_FILE" ]; then
    # default to fan mode being in auto
    /usr/bin/mfg-tool fan-mode -a
    exit 0
fi

source "/home/root/.fan_manual_mode.env"
FAN_PCT="${FAN_MANUAL_MODE_PCT:-80}"

/usr/bin/mfg-tool fan-mode -m
/usr/bin/mfg-tool fan-speed | /usr/bin/jq 'keys' | /usr/bin/grep FANBOARD | /usr/bin/tr -d \", | /usr/bin/awk '{print $1}' | xargs -n1 /usr/bin/mfg-tool fan-speed -t "$FAN_PCT" -p
