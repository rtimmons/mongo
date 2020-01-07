#!/usr/bin/env bash

# .sh scripts can `source _evg.sh` to get some behavior
# like saving artifacts etc

function save_artifact() {
    local repo_root="$EB_REPO_ROOT"
    local artifact="$1"

    echo "Copying $artifact to $repo_root/build/Artifacts/$artifact"
}

# TODO: _sudo
# TODO: _zip/_unzip/_tar_cfz/_tar_xfz
# TODO: _run_python
# TODO:
