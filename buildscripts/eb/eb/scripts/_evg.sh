#!/usr/bin/env bash

# .sh scripts can `source _evg.sh` to get some behavior
# like saving artifacts etc

function save_artifact() {
    local repo_root="$EB_REPO_ROOT"
    local artifact="$1"

    echo "Copying $artifact to $repo_root/build/Artifacts/$artifact"
}

function _schedule_tasks_gen() {
    local tasks=("$@")
    local str=""
    for task in "${tasks[@]}"; do
      str="$str\n- Name: $task"
    done
    cat << EOF
SchemaVersion: 2020-01-01
Tasks:$str
EOF
}

_schedule_task_invs=0

function schedule_tasks() {
    local str
    local repo_root="$EB_REPO_ROOT"
    mkdir -p "$repo_root/build/Tasks"
    str="$(_schedule_tasks_gen "$@")"
    echo "$str" > "$repo_root/build/Tasks/$$-${_schedule_task_invs}.yml"
    ((_schedule_task_invs=_schedule_task_invs+1))
}

# TODO: _sudo
# TODO: _zip/_unzip/_tar_cfz/_tar_xfz
# TODO: _run_python
# TODO:
