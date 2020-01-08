#!/usr/bin/env bash

_scons_compile_targets=(
  core
  # TODO: not working yet for some reason
  # archive-dist archive-dist-debug install-core install-tools
  "distsrc-${EB_X_ext:-tgz}"
  ${EB_X_additional_targets:-}
  "${EB_X_msi_target:-}"
  "${EB_X_mh_target:-}"
)

# TODO setup python virtualenv for Server
python3 -m pip install -r etc/pip/dev-requirements.txt -q -q

# TODO: change CWD to scripts/
source ./buildscripts/eb/eb/scripts/_scons_compile.sh

# TODO
# mv mongo ./build/Artifacts/mongo
