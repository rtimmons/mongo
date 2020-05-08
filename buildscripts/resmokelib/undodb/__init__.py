from buildscripts.resmokelib.commands import interface


class UndoDb(interface.Subcommand):
    def __init__(self, opts):
        self.args = opts.args

    def execute(self):
        pass
