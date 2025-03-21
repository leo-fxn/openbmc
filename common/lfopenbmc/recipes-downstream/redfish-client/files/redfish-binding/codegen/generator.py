import os
import shutil
from collections import deque

from typing import Deque, Dict, List, Set

from mako.lookup import TemplateLookup

from .cpp import (
    CppDef,
    CppIdentifier,
    CppObjectDef,
    CppObjectType,
    CppType,
    CppVariantType,
    CppVectorType,
)

from .definition import create_definition_map, Definition
from .schema import Schema


def generate_cpp_defs(schemas: List[Schema]) -> List[CppDef]:
    definition_map = create_definition_map(schemas)
    unprocessed_definitions: Deque[Definition] = deque(
        [
            definition
            for definition in definition_map.values()
            if definition.parent_schema.version is None
        ]
    )
    processed_definitions: Set[Definition] = set()
    cpp_defs = []
    generated_identifiers = set()
    while len(unprocessed_definitions) > 0:
        definition = unprocessed_definitions.pop()
        if definition in processed_definitions:
            continue
        processed_definitions.add(definition)
        unprocessed_definitions.extend(definition.get_deps(definition_map))
        cpp_def = definition.get_cpp_def(definition_map)
        if not cpp_def.has_body:
            continue
        if cpp_def.identifier in generated_identifiers:
            raise RuntimeError(
                f"Duplicated identifier generated: {cpp_def.identifier.namespace}::{cpp_def.identifier.id}"
            )
        generated_identifiers.add(cpp_def.identifier)
        cpp_defs.append(cpp_def)

    return cpp_defs


def get_forward_declarations(
    cpp_def: CppDef, cpp_object_identifiers: Set[CppIdentifier]
) -> Dict[str, Set[str]]:
    forward_declarations = {}

    def add_forward_declaration(cpp_type: CppType) -> None:
        if (
            isinstance(cpp_type, CppObjectType)
            and cpp_type.identifier in cpp_object_identifiers
        ):
            if cpp_type.identifier.namespace not in forward_declarations:
                forward_declarations[cpp_type.identifier.namespace] = set()
            forward_declarations[cpp_type.identifier.namespace].add(
                cpp_type.identifier.id
            )
        elif isinstance(cpp_type, CppVariantType):
            for t in cpp_type.types:
                add_forward_declaration(t)
        elif isinstance(cpp_type, CppVectorType):
            add_forward_declaration(cpp_type.element_type)

    if isinstance(cpp_def, CppObjectDef):
        for prop in cpp_def.properties:
            add_forward_declaration(prop.cpp_type)

    return forward_declarations


def get_cpp_file_name(cpp_def: CppDef) -> str:
    return f"{cpp_def.identifier.namespace}_{cpp_def.identifier.id}.hpp"


def generate_cpp_files(
    cpp_defs: List[CppDef],
    template_lookup: TemplateLookup,
    output_dir: str,
    cpp_include_dir: str,
) -> None:
    hpp_template = template_lookup.get_template("hpp.mako")
    shutil.copytree(cpp_include_dir, output_dir, dirs_exist_ok=True)
    cpp_object_identifiers = {
        cpp_def.identifier for cpp_def in cpp_defs if isinstance(cpp_def, CppObjectDef)
    }
    for cpp_def in cpp_defs:
        output_path = os.path.join(output_dir, get_cpp_file_name(cpp_def))
        with open(output_path, "w") as f:
            f.write(
                hpp_template.render(
                    cpp_def=cpp_def,
                    forward_declarations=get_forward_declarations(
                        cpp_def, cpp_object_identifiers
                    ),
                )
            )
