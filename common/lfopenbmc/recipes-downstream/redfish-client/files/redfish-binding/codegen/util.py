from typing import Optional, Type, TypeVar

T = TypeVar("T")


def get_value_for_required_key(obj: dict, key: str, expected_type: Type[T]) -> T:
    if key not in obj:
        raise KeyError(f"Missing required key: {key}")
    value = obj[key]
    if not isinstance(value, expected_type):
        raise TypeError(f"Expected {key} to be of type {expected_type}, got: {value}")
    return value


def get_value_for_optional_key(
    obj: dict, key: str, expected_type: Type[T]
) -> Optional[T]:
    try:
        return get_value_for_required_key(obj, key, expected_type)
    except (KeyError, TypeError):
        return None


class SchemaKey:
    META_SCHEMA = "$schema"
    ID = "$id"
    REF = "$ref"
    ANY_OF = "anyOf"
    DEFINITIONS = "definitions"
    TYPE = "type"
    ENUM = "enum"
    ITEMS = "items"
    PROPERTIES = "properties"


class DefinitionType:
    OBJECT = "object"
    STRING = "string"
    ARRAY = "array"
    INTEGER = "integer"
    NUMBER = "number"
    BOOLEAN = "boolean"
    NULL = "null"


PRIMITIVE_TYPES = [
    DefinitionType.STRING,
    DefinitionType.INTEGER,
    DefinitionType.NUMBER,
    DefinitionType.BOOLEAN,
    DefinitionType.NULL,
]
