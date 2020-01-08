"""
Generates tasks based on the ./build/Tasks directory.

All of the tasks in all files in the `build/Artifacts/Tasks`
directory will be transformed by `eb` into a `generate.tasks`
file called `build/Tasks.json`.

The 'eb' command in evergreen will call `generate.tasks` on
this file.

Input:

    # build/Artifacts/Tasks/1.yml
    Tasks:
    - Name: foo
    # build/Artifacts/Tasks/2.yml
    Tasks:
    - Name: bar

Output:

    // build/Tasks.json
    {"tasks": [
        {"name": "foo", "commands": [
            {"command": "shell.exec", "params": {
                "working_dir": "src",
                "shell": "bash",
                "script": "./eb foo"
            }},
        ]},
        {"name": "bar", "commands": [
            {"command": "shell.exec", "params": {
                "working_dir": "src",
                "shell": "bash",
                "script": "./eb bar"
            }},
        ]}
    ]}

Note there are a few "preamble" things before/after
that will be added to e.g. write `build/Expansions.yml`
if needed, etc.

This format is intentionally easy for even shell scripts
to write. Future versions of "eb" could easily re-run
itself with generated tasks allowing a build to be done
fully locally even though on evergreen it would be run
as multiple distinct tasks.

"""

import copy
import os
import re
import yaml
import json
import typing as typ

_TASK_TEMPLATE = {
    'name': 'foo',
    'commands': [
        {
            'command': 'shell.exec',
            'params': {
                'working_dir': 'src',
                'shell': 'bash',
                'script': './eb foo',
            }
        }
    ]
}


def _task(name: str):
    task = copy.deepcopy(_TASK_TEMPLATE)
    task['name'] = name
    # TODO: need to shell-escape name
    task['commands'][0]['params']['script'] = f"./eb '{name}'"
    return task


_IS_YAML_FILE = re.compile('$.*\\.yml$')


# Example yaml:
"""
SchemaVersion: 2020-01-01
Tasks:
- Name: foo
"""


def _gen_tasks(repo_root: str) -> typ.List[typ.Dict]:
    task_dir = os.path.join(repo_root, 'build', 'Tasks')
    if not os.path.exists(task_dir):
        # Didn't create any files
        return []
    if not os.path.isdir(task_dir):
        raise Exception(f"Tasks must be a directory {task_dir}")
    files = os.listdir(task_dir)
    not_matched = [f for f in files if not _IS_YAML_FILE.match(f)]
    if not_matched:
        raise Exception(f"Invalid Tasks yaml file names: {','.join(not_matched)}")

    # in case order of tasks matters?
    files.sort()

    tasks = []
    for file in files:
        with open(file, 'r') as conts:
            fc = yaml.load(conts, yaml.SafeLoader)
            if fc['SchemaVersion'] != '2020-01-01':
                raise Exception(f"Invalid schema version {fc['SchemaVersion']} in {file}")
            for task in fc["Tasks"]:
                if not task["Name"]:
                    raise Exception(f"No Name in task {task}")
                tasks.append(_task(task["Name"]))

    return tasks


def gen_tasks(repo_root: str):
    tasks = _gen_tasks(repo_root)
    full_obj = {'tasks': tasks}
    result = json.dumps(full_obj)

    out_path = os.path.join(repo_root, 'build', 'Tasks.json')
    with open(out_path, 'w') as out:
        out.writelines(result)
