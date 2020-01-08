#!/usr/bin/env bash

_scons_compile_targets=(
  all
)

# TODO setup python virtualenv for Server
python3 -m pip install -r etc/pip/dev-requirements.txt -q -q

# TODO: change CWD to scripts/
source ./buildscripts/eb/eb/scripts/_scons_compile.sh

# TODO
# mv mongo ./build/Artifacts/mongo
