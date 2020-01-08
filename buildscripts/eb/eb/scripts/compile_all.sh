#!/usr/bin/env bash

targets=(
  all
)

compile_flags=(
  --use-new-tools
  "--build-mongoreplay=${EB_X_build_mongoreplay}"
  --detect-odr-violations
)

# TODO setup python virtualenv for Server
python3 -m pip install -r etc/pip/dev-requirements.txt -q -q

# TODO: change CWD to scripts/
source ./buildscripts/eb/eb/scripts/_scons_compile.sh

# TODO
# mv mongo ./build/Artifacts/mongo
