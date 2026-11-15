for B in bank_static bank_dyn; do
  ./$B sleep >/dev/null & P=$!; sleep 0.4
  awk -v b=$B '/^(VmRSS|RssFile|RssAnon|VmSize):/{printf "%s %s=%s %s  ", b, $1, $2, $3} END{print ""}' /proc/$P/status
  kill $P 2>/dev/null; wait $P 2>/dev/null
done
