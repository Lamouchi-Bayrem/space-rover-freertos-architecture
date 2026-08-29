#!/usr/bin/env bash
set -euo pipefail
sudo apt update
sudo apt install -y build-essential cmake ninja-build git python3-pip python3-colcon-common-extensions shellcheck
printf '%s
' "Install ROS 2 Jazzy and Gazebo from their official instructions if ROS development is required."
