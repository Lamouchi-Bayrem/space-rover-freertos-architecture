#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../ros2_ws"
colcon build --symlink-install
