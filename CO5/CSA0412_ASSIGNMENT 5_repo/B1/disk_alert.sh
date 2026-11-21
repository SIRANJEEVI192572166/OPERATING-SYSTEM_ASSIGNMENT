#!/usr/bin/env bash
# disk_alert.sh - warn when a filesystem is fuller than THRESHOLD percent.
# Usage: disk_alert.sh [threshold] [mountpoint]
set -euo pipefail
THRESHOLD="${1:-80}"
MOUNT="${2:-/}"
USED=$(df --output=pcent "$MOUNT" | tail -1 | tr -dc '0-9')
STAMP=$(date '+%F %T')
if [ "$USED" -ge "$THRESHOLD" ]; then
    echo "$STAMP ALERT: $MOUNT is ${USED}% full (threshold ${THRESHOLD}%)" | tee -a /tmp/disk_alert.log
    exit 1
else
    echo "$STAMP OK: $MOUNT is ${USED}% full (threshold ${THRESHOLD}%)"
fi
