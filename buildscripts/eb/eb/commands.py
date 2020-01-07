import typing as typ
import os
import sys
import subprocess


def _create_env(env: typ.Mapping[str, str],
                expansions: typ.Mapping[str, typ.Any]) -> typ.Mapping[str, str]:
    out = {}
    out.update(env)
    prefix_expansions = {f"EB_X_{key}": f"{value}" for (key, value) in expansions.items()}
    out.update(prefix_expansions)
    return out


def dispatch(args: typ.List[str],
             env: typ.Mapping[str, str],
             repo_root: str,
             expansions: typ.Mapping[str, typ.Any]):
    if len(args) < 2:
        raise Exception("Usage: eb command")
    command = args[1]
    # TODO: use pkg_resources or similar
    scripts_dir = os.path.join(repo_root, 'buildscripts', 'eb', 'eb', 'scripts')
    if not os.path.isdir(scripts_dir):
        raise Exception(f"Cannot find scripts_dir {scripts_dir}")
    to_run = os.path.join(scripts_dir, command) + ".sh"
    if not os.path.isfile(to_run):
        raise Exception(f"Command script {to_run} doesn't exist.")
    subp_env = _create_env(env, expansions)
    subprocess.run([to_run],
                   env=subp_env,
                   shell=False,
                   check=True,
                   stdin=sys.stdin,
                   stdout=sys.stdout,
                   cwd=repo_root)




