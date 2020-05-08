import copy
import logging
import os
import shutil
import sys
from typing import List, Dict, Optional

from buildscripts.resmokelib.commands import interface
from buildscripts.resmokelib.core import process

_NOT_FOUND = """

    😱 Could not find `{}` on your `$PATH`.

You must manually download install the undodb package.

UndoDB is only supported on linux platforms. It will not work on macOS or Windows.

1. Download the tarball from Google Drive.
    https://drive.google.com/open?id=18dx1hHRrPgc27TtvvCe9rLXSYx_rKjzq
    You must be logged into your MongoDB/10gen google account to access this link.
    This file has MongoDB's private undodb key-server parameters baked into it,
    so do not share this file outside of the company.

2. Untar and install:

        tar xzf undodb-5.3.4.198.tgz
        cd undodb-5.3.4.198 && sudo make install

There is good README help in the undodb directory if you have questions.
There is also extensive documentation at https://docs.undo.io. 

Please also refer to William Schultz's intro talk at [1] with code and resources at [2].

[1]: https://mongodb.zoom.com/rec/share/5eBrDqHJ7k5If6uX9Fn7Wo0sGKT6T6a8gydK-_QOxBkaEyZzwv6Yf4tjTB4cS0f1
[2]: https://github.com/will62794/mongo/tree/370f66fcbdbaa06e2243d8e61bdd73f420a7a6c9/udb
"""

_OVERVIEW = """\
This is a simple wrapper atop of the UndoDB toolsuite. It does some quick checks to
ensure you have UndoDB installed and configured correctly and then just `exec`s the
program with the args you indicate.

Example usage:

    resmoke.py undodb run udb ./mongod --version

"""

_COMMAND = "undodb"


def add_subcommand(subparsers) -> None:
    """
    Add 'undodb' subcommand.
    """
    parser = subparsers.add_parser(_COMMAND, help=_OVERVIEW)
    parser.add_argument("args", nargs="*")


def subcommand(command, parsed_args) -> Optional[interface.Subcommand]:
    """
    :param command:
        The first arg to resmoke.py (e.g. resmoke.py run => command = run).
    :param parsed_args:
        Additional arguments parsed as a result of the `parser.parse` call.
    :return:
        Callback if the command is for undodb else none.
    """
    if command != _COMMAND:
        return None
    return UndoDb(parsed_args)


class SetupException(Exception):
    """
    Something has gone wrong with setup (e.g. programs missing from $PATH)
    """
    def __init__(self, message: str):
        super().__init__(message)


class System:
    """
    Simplified process interface. Used to make the logic more testable.
    """
    def __init__(self, cwd: str, env: Dict[str, str]):
        """
        :param cwd: cwd of the current process. Programs started will use this as the cwd.
        :param env: env vars. Programs started will use this as the env.
        """
        self.cwd = cwd
        self.env = copy.deepcopy(env)
        # 🌲 Logging 🌳
        self.root_logger = logging.Logger(_COMMAND, level=logging.DEBUG)
        handler = logging.StreamHandler(sys.stdout)
        handler.setFormatter(logging.Formatter(fmt="%(message)s"))
        self.root_logger.addHandler(handler)

    def run(self, argv: List[str]) -> int:
        self.root_logger.info("Running undodb command", argv)
        proc = process.Process(args=argv, env=self.env, cwd=self.cwd, logger=self.root_logger)
        proc.start()
        return proc.wait()

    def which(self, program) -> Optional[str]:
        return shutil.which(program)


class UndoDb(interface.Subcommand):
    def __init__(self, parsed_args, system=None):
        self.args = parsed_args.args
        if system is not None:
            self.system = system
        else:
            self.system = System(os.getcwd(), os.environ)

    def execute(self):
        if len(self.args) <= 1 \
                or self.args[0] == "help" \
                or self.args[0] != "run" \
                or self.args[0] == "--help":
            print(_OVERVIEW)
            return
        cmd = self.system.which(self.args[1])
        if cmd is None:
            raise SetupException(_NOT_FOUND.format(self.args[1]))
        self.system.run(self.args)
