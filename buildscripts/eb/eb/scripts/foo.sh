#!/usr/bin/env bash

source "$(dirname $0)/_evg.sh"

which python3
python --version
env

save_artifact foo

_schedule_task somepy
