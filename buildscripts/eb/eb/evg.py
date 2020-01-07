"""
Small set of utilities useful for writing custom packaging, building logic.
"""

import os


def save_artifact(artifact:str, allow_not_exists=True, env=None):
    if env is None:
        env = os.environ
    repo_root = env["EB_REPO_ROOT"]
    print(f"Copying {artifact} to {repo_root}/build/Artifacts/{artifact}")


