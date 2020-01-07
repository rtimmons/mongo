#!/usr/bin/env bash

# TODO: pyenv

EB_REPO_ROOT=$(git rev-parse --git-dir)/..
cd "$EB_REPO_ROOT"

if [[ ! -d ./build/eb_venv ]]; then
    python3 -m pip install virtualenv
    virtualenv ./build/eb_venv
    source ./build/eb_venv/bin/activate
    pushd ./buildscripts/eb >/dev/null
        python3 setup.py develop
    popd >/dev/null
fi

eb "$@"

