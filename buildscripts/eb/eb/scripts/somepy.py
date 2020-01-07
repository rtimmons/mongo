#!/usr/bin/env python3

import platform
import os
import sys

import eb.evg as evg

print(f"sys.executable={sys.executable}")
print(f"sys.path={sys.path}")
print(f"platform.platform()={platform.platform()}")
print(f"os.environ[\"PATH\"]={os.environ['PATH']}")
print(f"sys.implementation={sys.implementation}")

evg.save_artifact('foo')
