import logging
import sys

from .generator import (
    COMMON_HPP_FILE_NAME,
    generate_cpp_defs,
    generate_cpp_files,
    get_cpp_file_name,
)

from .schema import load_schemas

logging.basicConfig(stream=sys.stdout, level=logging.ERROR)


def list_files(schema_dir):
    schemas = load_schemas(schema_dir)
    cpp_defs = generate_cpp_defs(schemas)
    for cpp_def in cpp_defs:
        print(get_cpp_file_name(cpp_def))
    print(COMMON_HPP_FILE_NAME)


def call_generator(schema_dir, output_dir) -> None:
    schemas = load_schemas(schema_dir)
    cpp_defs = generate_cpp_defs(schemas)
    generate_cpp_files(cpp_defs, output_dir)
