#!/bin/bash

FAIL_STAMP=/run/ssh_failed
# 15 minute failure threshold
FAIL_THRESHOLD=$((60 * 15))


function log_sel {
    MESSAGE="$1"
    echo "$MESSAGE"
    busctl call \
    xyz.openbmc_project.Logging /xyz/openbmc_project/logging \
    xyz.openbmc_project.Logging.Create Create "ssa{ss}" "$MESSAGE" \
    xyz.openbmc_project.Logging.Entry.Level.Error 0
}

function exit_early_if_low_uptime {
    UPTIME=$(/usr/bin/awk '{printf "%d\n",$1}' /proc/uptime)
    if [ $UPTIME -lt $((60 * 60 * 3)) ]; then
        echo "Not rebooting; uptime less than 3 hours"
        exit 0
    fi
}

function fail_ssh_check {
  if [[ ! -f "$FAIL_STAMP" ]]; then
    touch $FAIL_STAMP
  fi
  AGE=$(($(date +%s) - $(stat -c %Y "$FAIL_STAMP")))
  echo "Failing for $AGE seconds"
  if [[ $AGE -gt $FAIL_THRESHOLD ]]; then
    exit_early_if_low_uptime
    log_sel "[sshd-remediate] Rebooting OOB due to continued ssh banner failure"
    /usr/sbin/reboot
  fi
  exit 1
}

function pass_ssh_check {
  rm -f $FAIL_STAMP
  exit 0
}

/usr/bin/timeout 10 bash -c 'echo -n "" | /usr/bin/nc 0 22 | /usr/bin/grep OpenSSH' && pass_ssh_check
fail_ssh_check
