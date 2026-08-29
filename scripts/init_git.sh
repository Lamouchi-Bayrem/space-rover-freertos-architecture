#!/usr/bin/env bash
set -euo pipefail
git init
git add .
git commit -m "chore: initialize rover FreeRTOS architecture"
git branch -M main
printf '%s
' "Next: git remote add origin https://github.com/YOUR_USERNAME/space-rover-freertos-architecture.git"
