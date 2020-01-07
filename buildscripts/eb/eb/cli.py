import sys
import typing as typ


def main(args: typ.List[str] = None):
    if args is None:
        args = sys.argv
    print(f"got args {args}")

