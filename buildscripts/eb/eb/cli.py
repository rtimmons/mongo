import argparse
import os
import sys
import typing as typ
import yaml

from eb import commands
from eb import taskgen


def _expansions(repo_root: str) -> typ.Mapping[str, typ.Any]:
    path = os.path.join(repo_root, 'build', 'Expansions.yml')
    if not os.path.exists(path):
        raise Exception(f"Expansions file {path} not found.")
    with open(path, 'r') as conts:
        return yaml.load(conts, Loader=yaml.FullLoader)


def _trim_env(env: typ.Mapping[str,str]):
    return {key: value for (key, value) in env.items() if key.startswith("EB_") or key == "PATH"}


def _parse_args():
    parser = argparse.ArgumentParser(prog='PROG')

    # TODO: the recursive part needs recursive generate.tasks()
    parser.add_argument('task', type=str, nargs=1, help='name of the task to generate')
    parser.add_argument('--convert', type=bool, help='convert generated tasks to Evergreen generate.tasks format instead of running them recursively')

    return parser.parse_args()


def main():
    env = os.environ

    env = _trim_env(env)
    if "EB_REPO_ROOT" not in env.keys():
        raise Exception("Need to define the EB_REPO_ROOT env var.")
    repo_root = env["EB_REPO_ROOT"]
    expansions = _expansions(repo_root)

    print(f'Have expansions as env vars: {", ".join(expansions.keys())}')

    args = _parse_args()

    next_tasks: typ.List = args.task
    while next_tasks:
        commands.dispatch(next_tasks.pop(), env, repo_root, expansions)
        if args.convert:
            taskgen.gen_tasks_evg(repo_root)
            break
        else:
            next_tasks.extend(taskgen.gen_tasks(repo_root))


