#!/bin/bash


download_change() {
  cid=$1
  outf=$2
  curl -ks "https://gerrit.openbmc.org/changes/$cid/revisions/current/patch?zip" -o tmp.zip
  unzip -p tmp.zip > "$outf"
  rm tmp.zip
}

test_change() {
  cid=$1
  if curl -sk "https://gerrit.openbmc.org/changes/$cid" | grep "Not found: " > /dev/null; then
    return 1
  fi
  return 0
}

patch_change() {
  file="$1"
  change=$(grep "Change-Id: " "$file" | awk '{print $2}')
  if test_change "$change"; then
    echo "$change"
  else
    echo "UNKNOWN"
  fi
}

patch_info() {
  cid="$1"
  curl -sk "https://gerrit.openbmc.org/changes/$cid" | jq -R 'fromjson?'
}

patch_status() {
  echo "$1" | jq -r .status
}

patch_updated() {
  echo "$1" | jq -r .updated
}

find_gerrit_patches() {
  grep -lR "Change-Id" "$1"
}

script_path="$0"
cmd="$1"
shift
if [ "$cmd" == "-h" ]; then
  echo "USAGE: $script_path COMMAND [PATCH|DIRECTORY]"
  echo "COMMAND is one of [status|update]"
  echo "If directory is provided it looks for all patches in said directory"
  exit 0
elif [ "$cmd" == "" ]; then
  echo "ERROR: Check $script_path -h"
  exit 1
fi
for ent in "$@"; do
  if [ -d "$ent" ]; then
    ents=$(find_gerrit_patches "$ent")
  elif [ -f "$ent" ]; then
    ents="$ent"
  else
    echo "ERROR: $ent not fount"
    continue
  fi
  for patch in $ents; do
    cid=$(patch_change "$patch")
    if [ "$cid" == "UNKNOWN" ]; then
      echo "$cid UNKNOWN UNKNOWN $patch"
      continue
    fi
    info=$(patch_info "$cid")
    pstatus=$(patch_status "$info")
    updated=$(patch_updated "$info")
    echo "$pstatus $updated $patch"
    if [ "$cmd" == "update" ]; then
      if [ "$pstatus" != "MERGED" ] && [ "$pstatus" != "ABANDONED" ]; then
        download_change "$cid" "$patch"
      else
        echo "NOT Updating merged/abandoned patch $patch"
      fi
    fi
  done
done
