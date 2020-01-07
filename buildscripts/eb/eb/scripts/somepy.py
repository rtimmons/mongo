#!/usr/bin/env python3

import platform
import os
import sys

print(f"sys.executable={sys.executable}")
print(f"sys.path={sys.path}")
print(f"platform.platform()={platform.platform()}")
print(f"os.environ[\"PATH\"]={os.environ['PATH']}")
print(f"sys.implementation={sys.implementation}")

