#!/usr/bin/env bash
# Measure start-up time, memory, disk and requests/second for bare-metal vs container.
APP=/home/claude/repo/B2
now(){ date +%s%N; }
wait_up(){ until curl -sf -o /dev/null "$1"; do sleep 0.02; done; }
ab_rps(){ for i in 1 2 3; do ab -n 5000 -c 20 -q "$1" 2>/dev/null | awk '/Requests per second/{print $4}'; done; }

echo "=== BARE METAL (gunicorn on host, port 8100) ==="
cd $APP; t0=$(now)
gunicorn --bind 127.0.0.1:8100 app:app >/dev/null 2>&1 & GPID=$!
wait_up http://127.0.0.1:8100/health; t1=$(now)
echo "start-up ms : $(( (t1-t0)/1000000 ))"
sleep 1
echo "memory MiB  : $(ps -o rss= --ppid $GPID -p $GPID | awk '{s+=$1} END {printf "%.1f", s/1024}') (master+worker RSS)"
echo "disk MiB    : $(du -sm /usr/local/lib/python3.12/dist-packages/{flask,werkzeug,jinja2,markupsafe,itsdangerous,click,blinker,gunicorn,packaging} $APP/app.py | awk '{s+=$1} END {print s}') (app + Python packages)"
echo "req/s (3 runs):"; ab_rps http://127.0.0.1:8100/result/21CS001
kill $GPID; wait $GPID 2>/dev/null

echo "=== CONTAINER (docker run -p 8101:8000 result-portal) ==="
docker rm -f b3c >/dev/null 2>&1; t0=$(now)
docker run -d --name b3c -p 8101:8000 result-portal >/dev/null
wait_up http://127.0.0.1:8101/health; t1=$(now)
echo "start-up ms : $(( (t1-t0)/1000000 ))"
sleep 1
echo "memory      : $(docker stats --no-stream --format '{{.MemUsage}}' b3c | cut -d/ -f1)"
echo "image size  : $(docker images result-portal --format '{{.Size}}')"
echo "req/s (3 runs):"; ab_rps http://127.0.0.1:8101/result/21CS001
docker rm -f b3c >/dev/null
