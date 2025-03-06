from __future__ import annotations

import re
from abc import ABC, abstractmethod

from typing import Dict, List, Optional, Tuple

from .cpp import (
    CppDef,
    CppEnumDef,
    CppIdentifier,
    CppObjectDef,
    CppObjectType,
    CppPrimitiveType,
    CppProperty,
    CppType,
    CppTypeAliasDef,
    CppVariantType,
    CppVectorType,
)
from .schema import Schema, SCHEMA_BASE_URL, Version
from .util import (
    DefinitionType,
    get_value_for_optional_key,
    get_value_for_required_key,
    PRIMITIVE_TYPES,
    SchemaKey,
)


class Property(object):
    def __init__(self, name: str, parent_schema: Schema, raw_property: dict):
        self.name = name
        self.raw_property = raw_property
        self.definition = create_definition(None, parent_schema, raw_property)

    def get_cpp_property(self, definition_map) -> CppProperty:
        return CppProperty(self.name, self.definition.get_cpp_type(definition_map))


class Definition(ABC):
    def __init__(
        self, name: Optional[str], parent_schema: Schema, raw_definition: dict
    ):
        self.name = name
        self.parent_schema = parent_schema
        self.raw_definition = raw_definition

    def get_cpp_namespace(self) -> str:
        return self.parent_schema.name

    def get_cpp_identifier(self) -> CppIdentifier:
        if self.name is None:
            raise ValueError("No cpp identifier for unnamed definition")
        return CppIdentifier(self.get_cpp_namespace(), self.name)

    @abstractmethod
    def get_cpp_type(self, definition_map) -> CppType:
        pass

    def get_cpp_def(self, definition_map) -> CppDef:
        return CppTypeAliasDef(
            self.get_cpp_identifier(), self.get_cpp_type(definition_map)
        )

    def should_skip(self) -> bool:
        return False

    def get_deps(self, definition_map) -> List[Definition]:
        return []

    def whoami(self) -> str:
        return type(self).__name__


class RefDefinition(Definition):
    def __init__(
        self, name: Optional[str], parent_schema: Schema, raw_definition: dict, ref: str
    ):
        super().__init__(name, parent_schema, raw_definition)
        match = re.match(
            r"(?P<schema_id>.+)?#/definitions/(?P<definition_name>.+)", ref
        )
        if not match:
            raise ValueError(f"Invalid ref: {ref}")
        schema_id = match.group("schema_id")
        definition_name = match.group("definition_name")
        self.ref_definition_id = f"{schema_id if schema_id else parent_schema.id}#/definitions/{definition_name}"

    def deref(self, definition_map) -> Definition:
        if self.ref_definition_id not in definition_map:
            raise KeyError(
                f"Unable to find definition {self.ref_definition_id} in definition map"
            )
        return definition_map[self.ref_definition_id]

    def get_cpp_type(self, definition_map) -> CppType:
        return CppObjectType(self.deref(definition_map).get_cpp_identifier())

    def should_skip(self) -> bool:
        return not self.ref_definition_id.startswith(SCHEMA_BASE_URL)

    def get_deps(self, definition_map) -> List[Definition]:
        return [self.deref(definition_map)]

    def is_id_ref(self) -> bool:
        return (
            self.ref_definition_id
            == SCHEMA_BASE_URL + "odata-v4.json#/definitions/idRef"
        )


class AnyOfDefinition(Definition):
    def __init__(
        self,
        name: Optional[str],
        parent_schema: Schema,
        raw_definition: dict,
        any_of_definitions: List[dict],
    ):
        super().__init__(name, parent_schema, raw_definition)
        self.any_of_definitions = [
            create_definition(name, parent_schema, definition)
            for definition in any_of_definitions
        ]

    def get_filtered_any_of_definitions(self, definition_map) -> List[Definition]:
        filtered_definitions: List[Definition] = []
        latest_version_refs: Dict[CppIdentifier, Tuple[Version, RefDefinition]] = {}
        for definition in self.any_of_definitions:
            if definition.should_skip():
                continue
            if isinstance(definition, PrimitiveDefinition) or isinstance(
                definition, ObjectDefinition
            ):
                filtered_definitions.append(definition)
            elif isinstance(definition, RefDefinition):
                deref_definition = definition.deref(definition_map)
                cpp_identifier = deref_definition.get_cpp_identifier()
                version = deref_definition.parent_schema.version
                if version is None:
                    filtered_definitions.append(definition)
                elif (
                    cpp_identifier not in latest_version_refs
                    or version > latest_version_refs[cpp_identifier][0]
                ):
                    latest_version_refs[cpp_identifier] = (
                        version,
                        definition,
                    )
            else:
                raise ValueError(
                    f"Unable to handle {definition.whoami()} within {self.whoami()}"
                )
        filtered_definitions.extend(
            [definition for _, definition in latest_version_refs.values()]
        )
        if len(filtered_definitions) > 1:
            filtered_definitions = [
                definition
                for definition in filtered_definitions
                if not (
                    isinstance(definition, RefDefinition) and definition.is_id_ref()
                )
            ]
        return filtered_definitions

    def get_cpp_def(self, definition_map) -> CppDef:
        definitions = self.get_filtered_any_of_definitions(definition_map)
        if len(definitions) == 1:
            return definitions[0].get_cpp_def(definition_map)
        else:
            return CppTypeAliasDef(
                self.get_cpp_identifier(),
                CppVariantType(
                    [
                        definition.get_cpp_type(definition_map)
                        for definition in definitions
                    ]
                ),
            )

    def get_cpp_type(self, definition_map) -> CppType:
        definitions = self.get_filtered_any_of_definitions(definition_map)
        if len(definitions) == 1:
            return definitions[0].get_cpp_type(definition_map)
        else:
            return CppVariantType(
                [definition.get_cpp_type(definition_map) for definition in definitions]
            )

    def get_deps(self, definition_map) -> List[Definition]:
        return [
            dep
            for definition in self.get_filtered_any_of_definitions(definition_map)
            for dep in definition.get_deps(definition_map)
        ]


class VariantDefinition(Definition):
    def __init__(
        self,
        name: Optional[str],
        parent_schema: Schema,
        raw_definition: dict,
        definition_types: List[str],
    ):
        super().__init__(name, parent_schema, raw_definition)
        if not all(
            definition_type in PRIMITIVE_TYPES for definition_type in definition_types
        ):
            raise ValueError(
                f"{self.whoami()} can only contain {PRIMITIVE_TYPES} but got: {definition_types}"
            )
        self.definition_types = definition_types

    def get_cpp_type(self, definition_map) -> CppType:
        filtered_types = [
            definition_type
            for definition_type in self.definition_types
            if definition_type != DefinitionType.NULL
        ]
        return (
            CppPrimitiveType(filtered_types[0])
            if len(filtered_types) == 1
            else CppVariantType(
                [
                    CppPrimitiveType(definition_type)
                    for definition_type in filtered_types
                ]
            )
        )


class EnumDefinition(Definition):
    def __init__(
        self,
        name: Optional[str],
        parent_schema: Schema,
        raw_definition: dict,
        enum: List[str],
    ):
        super().__init__(name, parent_schema, raw_definition)
        self.enum = enum

    def get_cpp_def(self, definition_map) -> CppDef:
        return CppEnumDef(self.get_cpp_identifier(), self.enum)

    def get_cpp_type(self, definition_map) -> CppType:
        raise NotImplementedError(f"Unexpected call for {self.whoami()}")


class ObjectDefinition(Definition):
    def __init__(
        self, name: Optional[str], parent_schema: Schema, raw_definition: dict
    ):
        super().__init__(name, parent_schema, raw_definition)
        self.properties = [
            Property(name, parent_schema, raw_property)
            for name, raw_property in get_value_for_required_key(
                raw_definition, SchemaKey.PROPERTIES, dict
            ).items()
        ]

    def get_filtered_properties(self) -> List[Property]:
        return [prop for prop in self.properties if not prop.definition.should_skip()]

    def get_cpp_def(self, definition_map) -> CppDef:
        return CppObjectDef(
            self.get_cpp_identifier(),
            [
                prop.get_cpp_property(definition_map)
                for prop in self.get_filtered_properties()
            ],
        )

    def get_cpp_type(self, definition_map) -> CppType:
        raise NotImplementedError(f"Unexpected call for {self.whoami()}")

    def get_deps(self, definition_map) -> List[Definition]:
        return [
            dep
            for prop in self.get_filtered_properties()
            for dep in prop.definition.get_deps(definition_map)
        ]


class ArrayDefinition(Definition):
    def __init__(
        self, name: Optional[str], parent_schema: Schema, raw_definition: dict
    ):
        super().__init__(name, parent_schema, raw_definition)
        self.item_definition = create_definition(
            None,
            parent_schema,
            get_value_for_required_key(raw_definition, SchemaKey.ITEMS, dict),
        )

    def get_cpp_type(self, definition_map) -> CppType:
        return CppVectorType(self.item_definition.get_cpp_type(definition_map))

    def get_deps(self, definition_map) -> List[Definition]:
        return self.item_definition.get_deps(definition_map)

    def should_skip(self) -> bool:
        return self.item_definition.should_skip()


class PrimitiveDefinition(Definition):
    def __init__(
        self,
        name: Optional[str],
        parent_schema: Schema,
        raw_definition: dict,
        definition_type: str,
    ):
        super().__init__(name, parent_schema, raw_definition)
        if definition_type not in PRIMITIVE_TYPES:
            raise ValueError(
                f"{self.whoami()} only accept {PRIMITIVE_TYPES} but got: {definition_type}"
            )
        self.definition_type = definition_type

    def get_cpp_type(self, definition_map) -> CppType:
        return CppPrimitiveType(self.definition_type)

    def should_skip(self) -> bool:
        return self.definition_type == DefinitionType.NULL


def create_definition(
    name: Optional[str], parent_schema: Schema, raw_definition: dict
) -> Definition:
    if ref := get_value_for_optional_key(raw_definition, SchemaKey.REF, str):
        return RefDefinition(name, parent_schema, raw_definition, ref)
    if any_of_definitions := get_value_for_optional_key(
        raw_definition, SchemaKey.ANY_OF, list
    ):
        return AnyOfDefinition(
            name,
            parent_schema,
            raw_definition,
            any_of_definitions,
        )
    if enum := get_value_for_optional_key(raw_definition, SchemaKey.ENUM, list):
        return EnumDefinition(name, parent_schema, raw_definition, enum)
    if definition_types := get_value_for_optional_key(
        raw_definition, SchemaKey.TYPE, list
    ):
        return VariantDefinition(name, parent_schema, raw_definition, definition_types)
    definition_type = get_value_for_required_key(raw_definition, SchemaKey.TYPE, str)
    if definition_type == DefinitionType.OBJECT:
        return ObjectDefinition(name, parent_schema, raw_definition)
    if definition_type == DefinitionType.ARRAY:
        return ArrayDefinition(name, parent_schema, raw_definition)
    return PrimitiveDefinition(name, parent_schema, raw_definition, definition_type)


def create_definition_map(schemas: List[Schema]) -> Dict[str, Definition]:
    definition_map = {}
    for schema in schemas:
        for definition in [
            create_definition(definition_name, schema, raw_definition)
            for definition_name, raw_definition in get_value_for_required_key(
                schema.raw_schema, SchemaKey.DEFINITIONS, dict
            ).items()
        ]:
            definition_id = f"{schema.id}#/definitions/{definition.name}"
            if definition_id in definition_map:
                raise KeyError(
                    f"Duplicated definition id {definition_id} found in {schema.id}"
                )
            definition_map[definition_id] = definition
    return definition_map
