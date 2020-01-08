#!/usr/bin/env bash

_scons_compile_targets=(
  core archive-dist archive-dist-debug install-core install-tools
  "distsrc-${EB_X_ext:-tgz}"
  ${EB_X_additional_targets:-}
  "${EB_X_msi_target:-}"
  "${EB_X_mh_target:-}"
)

_scons_compile_compile_flags=(
  --use-new-tools
  "--build-mongoreplay=${EB_X_build_mongoreplay}"
  --detect-odr-violations
  "--install-mode=hygienic"
  --separate-debug
  --legacy-tarball
)

# TODO setup python virtualenv for Server
python3 -m pip install -r etc/pip/dev-requirements.txt -q -q

# TODO: change CWD to scripts/
source ./buildscripts/eb/eb/scripts/_scons_compile.sh

# TODO
# mv mongo ./build/Artifacts/mongo
