"""Interface for creating a resmoke plugin."""

import abc


class Subcommand(object):
    """A resmoke subcommand to execute."""

    def execute(self):
        """Execute the subcommand."""
        raise NotImplementedError("execue must be implemented by Subcommand subclasses")


class PluginInterface(abc.ABC):
    def add_subcommand(self, subparsers):
        raise NotImplementedError()

    def parse(self, subcommand, parser, parsed_args, **kwargs):
        raise NotImplementedError()
