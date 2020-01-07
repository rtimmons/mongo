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


def _run_sh(args: typ.List[str],  env: typ.Mapping[str, str], cwd:str):
    subprocess.run(args,
                   env=env,
                   shell=False,
                   check=True,
                   stdin=sys.stdin,
                   stdout=sys.stdout,
                   cwd=cwd)


# lol idk if this really needs to be a different function versus _run_sh
def _run_py(args: typ.List[str],  env: typ.Mapping[str, str], cwd:str):
    subprocess.run(args,
                   env=env,
                   shell=False,
                   check=True,
                   stdin=sys.stdin,
                   stdout=sys.stdout,
                   cwd=cwd)


def _runner_and_fname(dirname: str, command_no_suffix: str):
    sh = os.path.join(dirname, f"{command_no_suffix}.sh")
    py = os.path.join(dirname, f"{command_no_suffix}.py")
    sh_exists = os.path.exists(sh)
    py_exists = os.path.exists(py)
    if not sh_exists ^ py_exists:
        raise Exception(f"Need exactly one of {sh} xor {py}")
    return (sh, _run_sh) if sh_exists else (py, _run_py)


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
    subp_env = _create_env(env, expansions)
    (script, runner) = _runner_and_fname(scripts_dir, command)
    runner([script], subp_env, repo_root)




