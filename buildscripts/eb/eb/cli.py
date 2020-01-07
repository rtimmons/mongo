import os
import sys
import typing as typ
import yaml

from eb import commands


def _expansions(repo_root: str) -> typ.Mapping[str, typ.Any]:
    path = os.path.join(repo_root, 'build', 'Expansions.yml')
    if not os.path.exists(path):
        raise Exception(f"Expansions file {path} not found.")
    with open(path, 'r') as conts:
        return yaml.load(conts, Loader=yaml.FullLoader)


def _trim_env(env: typ.Mapping[str,str]):
    return {key: value for (key, value) in env.items() if key.startswith("EB_") or key == "PATH"}


def main(args: typ.List[str] = None, env: typ.Mapping[str,str] = None):
    if args is None:
        args = sys.argv

    if env is None:
        env = os.environ
    env = _trim_env(env)
    if "EB_REPO_ROOT" not in env.keys():
        raise Exception("Need to define the EB_REPO_ROOT env var.")
    repo_root = env["EB_REPO_ROOT"]
    expansions = _expansions(repo_root)
    print(f"Loaded expansions {expansions}")

    commands.dispatch(args, env, repo_root, expansions)


