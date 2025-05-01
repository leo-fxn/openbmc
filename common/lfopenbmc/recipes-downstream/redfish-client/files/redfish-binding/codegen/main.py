import argparse
import os

from .commands import call_generator, list_files


def main():
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

    call_generator(args.schema_dir, args.output_dir)
