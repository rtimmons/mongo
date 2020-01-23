class JSCore(object):
    def __init__(self):


    def suites(self):
        return "core !this-one-test"

    def compile_flags(self):
        pass

    def gen_task_yaml(self, buildvariant_options):
        suite = self.suites()
        flags = self.compile_flags()
        if buildvariant_options.asan:
            suite.remove('slow-tests')

        return gen_yaml(suite, flags)

class Txns(JSCore):

    def suites(self):
        parent = super.suites()
        my_suites = parent.run_only_with_tags('txns')
        return my_suites

    def compile_flags(self):
        pass

    def runtime_flag(self):
        parent = super.runtim_flag()
        return parent.append('--suite txns')
