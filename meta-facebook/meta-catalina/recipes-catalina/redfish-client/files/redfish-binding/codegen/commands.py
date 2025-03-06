import logging
import sys

from mako.lookup import TemplateLookup

from .generator import generate_cpp_defs, generate_cpp_files, get_cpp_file_name

from .schema import load_schemas

logging.basicConfig(stream=sys.stdout, level=logging.ERROR)


def list_files(schema_dir):
    schemas = load_schemas(schema_dir)
    cpp_defs = generate_cpp_defs(schemas)
    for cpp_def in cpp_defs:
        print(get_cpp_file_name(cpp_def))


def call_generator(template_dir, cpp_include_dir, schema_dir, output_dir) -> None:
    schemas = load_schemas(schema_dir)
    cpp_defs = generate_cpp_defs(schemas)

    template_lookup = TemplateLookup(
        directories=[template_dir],
        imports=["from codegen import cpp"],
    )
    generate_cpp_files(cpp_defs, template_lookup, output_dir, cpp_include_dir)
