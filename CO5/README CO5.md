# CSA04-<RegisterNumber>-Assignment5

- **Name:** <your name>   **Register number:** <your register number>
- **Linux distribution:** Ubuntu 24.04.5 LTS
- **Compiler:** gcc 13.3.0 (no C program is needed in this assignment; tasks use Bash, Python, Docker, YAML)
- **Tools:** Python 3.12.3, gunicorn 27.0.1, Flask 3.1.3, Nginx 1.24.0, Docker 29.1.3, ApacheBench 2.3

## Folders and how to run
| Folder | Contents | Run |
|---|---|---|
| B1 | log_summary.sh (Complete the Code), log_analysis.sh, disk_alert.sh, scale_test.sh, crontab.txt, nginx_portal.conf, result-portal.service | `bash log_summary.sh` |
| B2 | app.py, requirements.txt, Dockerfile, docker-compose.yml | `docker build -t result-portal . && docker run -p 8000:8000 result-portal` |
| B3 | measure.sh, deployment.yaml, hpa.yaml, validate_yaml.py | `./measure.sh`; `kubectl apply -f deployment.yaml` |
| B4 | lms_app.py, Dockerfile.lms, docker-compose.yml | see report |

## Results (short)
B1 output matches the expected output. B2 image builds, runs as user `portal`, CPU/memory limits verified with docker stats.
B3 measured bare metal vs container (see report). Kubernetes and VM steps: add your own screenshots.
Note: part of the work was run with a locally built stand-in for python:3.12-slim (Docker Hub unreachable); rebuild with the official image.
