#!/usr/bin/env bash

echo "printing dirname"
echo "$(dirname $0)/_evg.sh"

# shellcheck source=buildscripts/eb/scripts/_evg.sh
source "$(dirname $0)/_evg.sh"

which python3
python --version
env

save_artifact foo

# TODO fix multiline replace.
schedule_tasks somepy
