project(
    'redfish-client',
    'cpp',
    version: '1.0',
    default_options: [
        'cpp_std=c++23',
    ],
    meson_version: '>=1.1.1',
)

phosphor_logging_dep = dependency('phosphor-logging')
phosphor_dbus_interfaces_dep = dependency('phosphor-dbus-interfaces')
sdbusplus_dep = dependency('sdbusplus')
nlohmann_json_dep = dependency('nlohmann_json', include_type: 'system')
curl_dep = dependency('libcurl')
boost_dep = dependency('boost', required: true)
dl_dep = dependency('dl')

dsp_spec_jsondir = meson.project_source_root() / 'DSP8010_2024.4' / 'json-schema'

subdir('redfish-binding')

subdir('configurations')

packagedir = join_paths(
    get_option('prefix'),
    get_option('datadir'),
    meson.project_name(),
)

foreach conf : configurations
    install_data(
        join_paths('configurations', conf),
        install_dir: join_paths(
            packagedir,
            'configurations',
        )
    )
endforeach

base_deps = [
    boost_dep,
    curl_dep,
    dl_dep,
    nlohmann_json_dep,
    phosphor_logging_dep,
    phosphor_dbus_interfaces_dep,
    redfish_binding_dep,
    sdbusplus_dep
]

cpp_args = [
  '-DBOOST_STACKTRACE_USE_ADDR2LINE',
  '-ggdb',
  '-fPIC',
]

link_args = ['-rdynamic']

srcs = [
    'daemon.cpp',
    'http_client.cpp',
    'sensor.cpp',
    'source.cpp',
]

redfish_client_library = static_library(
    'redfish-client-lib',
    srcs,
    cpp_args: cpp_args,
    link_args: link_args,
    dependencies: base_deps,
)

redfish_client = executable(
    'redfish-client',
    'daemon_main.cpp',
    dependencies: base_deps,
    cpp_args: cpp_args,
    link_args: link_args,
    link_with: redfish_client_library,
    install: true,
)

cc = meson.get_compiler('cpp')

test_deps = [
    cc.find_library('gmock'),
    cc.find_library('gtest'),
    cc.find_library('gtest_main'),
]

test_srcs = [
    'test/eventlog_parser_tests.cpp',
    'test/daemon_tests.cpp',
    'test/dbus_server_tests.cpp',
    'test/http_client_tests.cpp',
    'test/logentry_tests.cpp',
    'test/redfishlib_tests.cpp',
    'test/sensor_tests.cpp',
    'test/source_tests.cpp',
]

redfish_client_test = executable(
    'test-redfish-client',
    test_srcs,
    dependencies: [
                test_deps,
                base_deps,
            ],
    cpp_args: cpp_args,
    link_args: link_args,
    link_with: redfish_client_library,
)

test('redfish-client-tests', redfish_client_test)
