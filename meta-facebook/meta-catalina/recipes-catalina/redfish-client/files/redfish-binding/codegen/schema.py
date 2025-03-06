import json
import logging
import os
import re

from functools import total_ordering
from typing import List

from .util import get_value_for_required_key, SchemaKey

logger = logging.getLogger(__name__)

SCHEMA_BASE_URL = "http://redfish.dmtf.org/schemas/v1/"
VALID_META_SCHEMA = SCHEMA_BASE_URL + "redfish-schema-v1.json"


@total_ordering
class Version(object):
    def __init__(self, version: str):
        match = re.match(r"v(?P<major>\d+)_(?P<minor>\d+)_(?P<errata>\d+)", version)
        if not match:
            raise ValueError(f"Invalid version: {version}")
        self.major = int(match.group("major"))
        self.minor = int(match.group("minor"))
        self.errata = int(match.group("errata"))
        self.version = version

    def __str__(self) -> str:
        return self.version

    def __eq__(self, other):
        return (self.major, self.minor, self.errata) == (
            other.major,
            other.minor,
            other.errata,
        )

    def __lt__(self, other):
        return (self.major, self.minor, self.errata) < (
            other.major,
            other.minor,
            other.errata,
        )


class Schema(object):
    def __init__(self, raw_schema: dict):
        self.raw_schema = raw_schema
        if not isinstance(raw_schema, dict):
            raise TypeError(f"Expected schema to be a JSON object, got: {raw_schema}")
        meta_schema = get_value_for_required_key(raw_schema, SchemaKey.META_SCHEMA, str)
        if meta_schema != VALID_META_SCHEMA:
            raise ValueError(
                f"Meta schema other than {VALID_META_SCHEMA} is not supported: {meta_schema}"
            )
        self.id = get_value_for_required_key(raw_schema, SchemaKey.ID, str)
        id_match = re.match(
            re.escape(SCHEMA_BASE_URL)
            + r"(?P<name>[^.]+)(?:\.(?P<version>[^.]+))?\.json",
            self.id,
        )
        if not id_match:
            raise ValueError(f"Invalid schema id: {self.id}")
        self.name = id_match.group("name")
        version = id_match.group("version")
        self.version = Version(version) if version else None


def load_schemas(schemas_dir: str) -> List[Schema]:
    schemas: list[Schema] = []
    for filename in os.listdir(schemas_dir):
        if (
            not re.match(
                r"^(?!odata|redfish-error|redfish-payload-annotations|redfish-schema).*\.json$",
                filename,
            )
            and filename != "odata-v4.json"
        ):
            logger.info(f"Skipping {filename}: not a resource schema")
            continue
        with open(os.path.join(schemas_dir, filename), "r") as f:
            try:
                schemas.append(Schema(json.load(f)))
            except Exception as e:
                logger.info(f"Skipping {filename}: {e}")
    logger.info(f"{len(schemas)} schemas are loaded")
    return schemas
