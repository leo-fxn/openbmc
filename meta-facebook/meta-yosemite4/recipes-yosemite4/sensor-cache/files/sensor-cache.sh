#!/bin/bash -e
SENSOR_CACHE=/run/openbmc/sensors/sensor_cache.json

# this file's ctime is updated whenever cached data is requested.
# this allows the supervising timer to not pre-cache unless it's being asked
# for.
REQUESTED_CACHE_FLAG=/run/openbmc/sensors/requested_cached

QUIET="-q"
NOT_QUIET=""
THRESHOLD_NONE=0
THRESHOLD_MAX=120

# once requested, keep caching for this many seconds
KEEP_CACHING_FOR=$((60 * 60 * 1))

THRESHOLD="${1:-$THRESHOLD_NONE}"
QUIET_SETTING="${2:-$NOT_QUIET}"

function collect_and_store {
  mkdir -p "$(dirname $SENSOR_CACHE)"
  tmpfile=$(mktemp)
  if [ "$QUIET_SETTING" == "$NOT_QUIET" ]; then
    /usr/bin/mfg-tool sensor-display | tee "$tmpfile"
  else
    /usr/bin/mfg-tool sensor-display > "$tmpfile" 2>/dev/null
  fi
  mv "$tmpfile" "$SENSOR_CACHE"
}

function cleanup_cache_flag {
  if [ -f $REQUESTED_CACHE_FLAG ]; then
    AGE=$(($(date +%s) - $(stat -c %Y "$REQUESTED_CACHE_FLAG")))
    if [ $AGE -gt $KEEP_CACHING_FOR ]; then
      rm -f $REQUESTED_CACHE_FLAG
    fi
  fi
}

function collect {
  # default to always needing it
  AGE=$((THRESHOLD_MAX + 1))

  if [ -f $SENSOR_CACHE ]; then
    AGE=$(($(date +%s) - $(stat -c %Y "$SENSOR_CACHE")))
  fi

  if [ "$AGE" -lt "$THRESHOLD" ]; then
    if [ "$QUIET_SETTING" == "$NOT_QUIET" ]; then
      cat $SENSOR_CACHE
    fi
  else
    collect_and_store
  fi
}

if [ "$THRESHOLD" -gt 0 ]; then
  # record that someone is asking for cached data. This enables
  # the timer & service to run
  mkdir -p "$(dirname $REQUESTED_CACHE_FLAG)"
  rm -f "$REQUESTED_CACHE_FLAG" && touch "$REQUESTED_CACHE_FLAG"
fi

collect
cleanup_cache_flag
