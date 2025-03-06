#!/usr/bin/env python3
import argparse
import os

from codegen.commands import call_generator, list_files


def _main():
    cur_dir = os.path.join(os.path.dirname(__file__), "codegen")
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-s",
        "--schema-dir",
        type=str,
        help="Location of schema files.",
    )
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "-o",
        "--output-dir",
        type=str,
        help="Location of output files.",
    )
    group.add_argument(
        "-l",
        "--list",
        action="store_true",
        help="List output files.",
    )
    args = parser.parse_args()

    assert os.path.exists(args.schema_dir)
    if args.list:
        list_files(args.schema_dir)
        return

    cpp_include_dir = os.path.join(cur_dir, "cpp", "include")
    template_dir = os.path.join(cur_dir, "templates")
    call_generator(template_dir, cpp_include_dir, args.schema_dir, args.output_dir)


if __name__ == "__main__":
    _main()
