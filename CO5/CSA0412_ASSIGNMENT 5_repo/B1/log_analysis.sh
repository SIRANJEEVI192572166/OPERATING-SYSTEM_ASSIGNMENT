#!/usr/bin/env bash
# log_analysis.sh - summarise an Nginx access log (combined format).
# Usage: log_analysis.sh [logfile]
set -euo pipefail
LOG="${1:-/var/log/nginx/access.log}"
[ -r "$LOG" ] || { echo "Cannot read $LOG" >&2; exit 1; }

echo "=== Log analysis: $LOG ($(date '+%F %T')) ==="
echo "Total requests : $(wc -l < "$LOG")"
echo "--- Requests per status code ---"
awk '{print $9}' "$LOG" | sort | uniq -c | sort -k2n | awk '{printf "  %s %s\n", $2, $1}'
echo "--- Top 3 clients ---"
awk '{print $1}' "$LOG" | sort | uniq -c | sort -rn | head -3 | awk '{printf "  %s %s\n", $2, $1}'
echo "--- Server errors (5xx) ---"
awk '$9 ~ /^5/ {n++} END {print "  " n+0 " request(s)"}' "$LOG"
