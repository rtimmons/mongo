"""Parser for command line arguments."""

import shlex

import argparse

from . import configure_resmoke

from .run import RunPlugin
from .hang_analyzer import HangAnalyzerPlugin
from .undodb import UndoDbPlugin

_PLUGINS = [
    RunPlugin(),
    HangAnalyzerPlugin(),
    UndoDbPlugin(),
]


def _add_subcommands():
    """Create and return the command line arguments parser."""
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command")

    # Add sub-commands.
    for plugin in _PLUGINS:
        plugin.add_subcommand(subparsers)

    return parser


# def to_local_args(args=None):  # pylint: disable=too-many-branches,too-many-locals
#     """
#     Return a command line invocation for resmoke.py suitable for being run outside of Evergreen.
#     This function parses the 'args' list of command line arguments, removes any Evergreen-centric
#     options, and returns a new list of command line arguments.
#     """

#     if args is None:
#         args = sys.argv[1:]

#     parser = _make_parser()

#     # We call optparse.OptionParser.parse_args() with a new instance of optparse.Values to avoid
#     # having the default values filled in. This makes it so 'options' only contains command line
#     # options that were explicitly specified.
#     options, extra_args = parser.parse_args(args=args, values=optparse.Values())

#     # If --originSuite was specified, then we replace the value of --suites with it. This is done to
#     # avoid needing to have engineers learn about the test suites generated by the
#     # evergreen_generate_resmoke_tasks.py script.
#     origin_suite = getattr(options, "origin_suite", None)
#     if origin_suite is not None:
#         setattr(options, "suite_files", origin_suite)

#     # optparse.OptionParser doesn't offer a public and/or documented method for getting all of the
#     # options. Given that the optparse module is deprecated, it is unlikely for the
#     # _get_all_options() method to ever be removed or renamed.
#     all_options = parser._get_all_options()  # pylint: disable=protected-access

#     options_by_dest = {}
#     for option in all_options:
#         options_by_dest[option.dest] = option

#     suites_arg = None
#     storage_engine_arg = None
#     other_local_args = []

#     options_to_ignore = {
#         "--archiveLimitMb",
#         "--archiveLimitTests",
#         "--buildloggerUrl",
#         "--log",
#         "--perfReportFile",
#         "--reportFailureStatus",
#         "--reportFile",
#         "--staggerJobs",
#         "--tagFile",
#     }

#     def format_option(option_name, option_value):
#         """
#         Return <option_name>=<option_value>.
#         This function assumes that 'option_name' is always "--" prefix and isn't "-" prefixed.
#         """
#         return "%s=%s" % (option_name, option_value)

#     for option_dest in sorted(vars(options)):
#         option_value = getattr(options, option_dest)
#         option = options_by_dest[option_dest]
#         option_name = option.get_opt_string()

#         if option_name in options_to_ignore:
#             continue

#         option_group = parser.get_option_group(option_name)
#         if option_group is not None and option_group.title == _EVERGREEN_OPTIONS_TITLE:
#             continue

#         if option.takes_value():
#             if option.action == "append":
#                 args = [format_option(option_name, elem) for elem in option_value]
#                 other_local_args.extend(args)
#             else:
#                 arg = format_option(option_name, option_value)

#                 # We track the value for the --suites and --storageEngine command line options
#                 # separately in order to more easily sort them to the front.
#                 if option_dest == "suite_files":
#                     suites_arg = arg
#                 elif option_dest == "storage_engine":
#                     storage_engine_arg = arg
#                 else:
#                     other_local_args.append(arg)
#         else:
#             other_local_args.append(option_name)

#     return [arg for arg in (suites_arg, storage_engine_arg) if arg is not None
#             ] + other_local_args + extra_args


def _parse(sys_args):
    """Parse the CLI args."""

    # Split out this function for easier testing.
    parser = _add_subcommands()
    parsed_args = parser.parse_args(sys_args)

    return (parser, parsed_args)


def parse_command_line(sys_args, **kwargs):
    """Parse the command line arguments passed to resmoke.py and return the subcommand object to execute."""
    parser, parsed_args = _parse(sys_args)

    subcommand = parsed_args.command

    for plugin in _PLUGINS:
        subcommand_obj = plugin.parse(subcommand, parser, parsed_args, **kwargs)
        if subcommand_obj is not None:
            return subcommand_obj

    raise RuntimeError(f"Resmoke configuration has invalid subcommand: {subcommand}. Try '--help'")


def set_run_options(argstr=''):
    """Populate the config module variables for the 'run' subcommand with the default options."""
    parser, parsed_args = _parse(['run'] + shlex.split(argstr))
    configure_resmoke.validate_and_update_config(parser, parsed_args)
