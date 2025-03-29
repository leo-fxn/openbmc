project('redfish-schema-pack', 'cpp', version: '2024.4', license: 'BSD 3-Clause')

redfish_schema_pack_dep = declare_dependency(
    variables: {'schema_dir': meson.current_source_dir()},
)

meson.override_dependency('redfish-schema-pack', redfish_schema_pack_dep)
