"""
Deals with installing and setting up udb.
"""
import copy
import os
import sys
from typing import List, Dict, Tuple
from urllib.request import urlretrieve

# https://docs.undo.io/SystemRequirements.html

_UDB_URL = "http://TODO/undodb-5.3.4.198.tgz"  # TODO
_UNTAR_LOCATION = "pwd"  # TODO


def log(*args):
    print(*args, file=sys.stderr)


class SetupException(Exception):
    def __init__(self, message: str):
        super().__init__(message)


class System:
    def __init__(self,
                 cwd: str,
                 env: Dict[str, str],
                 downloader=urlretrieve):
        self.cwd = cwd
        self.env = copy.deepcopy(env)
        self.downloader = downloader

    def im_on_vpn(self) -> bool:
        return True  # TODO

    def has_on_path(self) -> bool:
        # look for `udb` executable
        return True

    def run(self, argv: List[str]) -> int:
        log(f"Going to run {argv[1:]}")
        return 0

# class Setup:
#     def __init__(self, , system: System):
#         self.system = system
#
#     @staticmethod
#     def _filename(url: str) -> str:
#         return os.path.basename(url)
#
#     # undodb-5.3.4.198.tgz -> undodb-5.3.4.198 -> undodb-5.3.4.198/undodb
#
#     @staticmethod
#     def _fileparts(url: str) -> Tuple[str, str]:
#         parts = Setup._filename(url).split(".")
#         if len(parts) != 2 or parts[1] != "tgz":
#             raise SetupException(f"Must give url with trailing filename.tgz gave {url}")
#         return parts[0], parts[1]
#
#     def download(self, url: str, dest_dir: str) -> str:
#         if self.system.path_exists
#

class CLI:
    def __init__(self, , system: System):
        self.system = system

    def main(self, argv: List[str]) -> int:
        if not self.system.has_on_path():
            raise SetupException(f"Must setup udb first")
        if not self.system.im_on_vpn():
            raise SetupException("Must be on the MongoDB VPN to access udb license server.")
        return self.system.run(argv)

if __name__ == "__main__":
    mainsys = System(os.getcwd(), os.environ)
    cli = CLI(mainsys)
    out = cli.main(sys.argv)
    sys.exit(out)
