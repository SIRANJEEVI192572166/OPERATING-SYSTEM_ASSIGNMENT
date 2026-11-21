#!/usr/bin/env bash
# Time log_analysis.sh on logs of increasing size to show O(L) behaviour.
echo "lines    seconds"
for n in 100000 200000 400000 800000; do
  awk -v n=$n 'BEGIN{for(i=0;i<n;i++) printf "10.0.%d.%d - - [21/Nov/2026:10:00:00 +0000] \"GET /result/%d HTTP/1.1\" %d 512 \"-\" \"ab\"\n", i%7, i%250, i, (i%50==0)?500:200}' > /tmp/big.log
  s=$(date +%s.%N); /opt/portal/log_analysis.sh /tmp/big.log >/dev/null; e=$(date +%s.%N)
  printf "%-8s %.2f\n" $n $(echo "$e - $s" | bc)
done
rm -f /tmp/big.log
