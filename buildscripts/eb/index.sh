#!/usr/bin/env bash

if [[ -e "/opt/mongodbtoolchain/v3/bin" ]]; then
    export PATH="/opt/mongodbtoolchain/v3/bin:$PATH"
    # TODO: support pyenv in an `else` block.
fi

EB_REPO_ROOT=$(git rev-parse --git-dir)/..
cd "$EB_REPO_ROOT" || exit 1

EB_REPO_ROOT=$(pwd -P)
export EB_REPO_ROOT

mkdir -p ./build

if [[ ! -d ./build/eb_venv ]]; then
    python3 -m pip install virtualenv --isolated -q -q
    python3 -m virtualenv ./build/eb_venv
    source ./build/eb_venv/bin/activate
    pushd ./buildscripts/eb || exit 1 >/dev/null
        python3 setup.py --no-user-cfg -q -q install > ./build/eb_setup.log 2>&1
    popd >/dev/null || exit 1
else
    source ./build/eb_venv/bin/activate
fi

eb "$@"

