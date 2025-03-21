import re
from abc import ABC, abstractmethod
from functools import reduce
from typing import List

from .util import DefinitionType


def sanitize_name(name: str) -> str:
    return re.sub(r"[^a-zA-Z0-9_]", "_", name)


class CppIdentifier(object):
    def __init__(self, namespace: str, id: str):
        self.namespace = sanitize_name(namespace)
        self.id = sanitize_name(id)

    def __hash__(self):
        return hash((self.namespace, self.id))

    def __eq__(self, other):
        return (self.namespace, self.id) == (other.namespace, other.id)


class CppType(ABC):
    def __init__(self):
        self.include_headers = set()
        self.include_generated_headers = set()

    @abstractmethod
    def get_name(self, current_namespace: str) -> str:
        pass


class CppPrimitiveType(CppType):
    DEFINITION_TYPE_MAP = {
        DefinitionType.BOOLEAN: "bool",
        DefinitionType.INTEGER: "int",
        DefinitionType.NUMBER: "double",
        DefinitionType.STRING: "std::string",
    }

    def __init__(self, definition_type: str):
        super().__init__()
        if definition_type == DefinitionType.STRING:
            self.include_headers.add("string")
        self.name = self.DEFINITION_TYPE_MAP[definition_type]

    def get_name(self, current_namespace: str) -> str:
        return self.name


class CppVariantType(CppType):
    def __init__(self, types: List[CppType]):
        super().__init__()
        self.include_headers.add("variant")
        for t in types:
            self.include_headers.update(t.include_headers)
            self.include_generated_headers.update(t.include_generated_headers)
        self.types = types

    def get_name(self, current_namespace: str) -> str:
        return f"std::variant<{', '.join([t.get_name(current_namespace) for t in self.types])}>"


class CppObjectType(CppType):
    def __init__(self, identifier: CppIdentifier):
        super().__init__()
        self.include_generated_headers.add(f"{identifier.namespace}_{identifier.id}")
        self.identifier = identifier

    def get_name(self, current_namespace: str) -> str:
        return (
            self.identifier.id
            if self.identifier.namespace == current_namespace
            else f"{self.identifier.namespace}::{self.identifier.id}"
        )


class CppVectorType(CppType):
    def __init__(self, element_type: CppType):
        super().__init__()
        self.include_headers.add("vector")
        self.include_headers.update(element_type.include_headers)
        self.include_generated_headers.update(element_type.include_generated_headers)
        self.element_type = element_type

    def get_name(self, current_namespace: str) -> str:
        return f"std::vector<{self.element_type.get_name(current_namespace)}>"


class CppProperty(object):
    def __init__(self, name: str, cpp_type: CppType):
        self.name = name
        self.cpp_type = cpp_type
        self.name_as_member = sanitize_name(name)


class CppDef(object):
    def __init__(
        self,
        identifier: CppIdentifier,
    ):
        self.identifier = identifier
        self.include_headers = set()
        self.include_generated_headers = set()
        self.has_body = True


class CppObjectDef(CppDef):
    def __init__(self, identifier: CppIdentifier, properties: List[CppProperty]):
        super().__init__(identifier)
        self.include_headers = reduce(
            set.union, [p.cpp_type.include_headers for p in properties], set()
        )
        self.include_generated_headers = reduce(
            set.union, [p.cpp_type.include_generated_headers for p in properties], set()
        )
        self.properties = properties


class CppTypeAliasDef(CppDef):
    def __init__(self, identifier: CppIdentifier, type_alias: CppType):
        super().__init__(identifier)
        self.include_headers = type_alias.include_headers
        self.include_generated_headers = type_alias.include_generated_headers
        self.has_body = not (
            isinstance(type_alias, CppObjectType)
            and type_alias.identifier == identifier
        )
        self.type_alias = type_alias


class CppEnumDef(CppDef):
    def __init__(self, identifier: CppIdentifier, enum: List[str]):
        super().__init__(identifier)
        self.enum = {sanitize_name(e): e for e in enum}
