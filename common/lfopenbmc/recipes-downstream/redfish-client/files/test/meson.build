gtest_dep = dependency('gtest', main: true, disabler: true, required: false)
gmock_dep = dependency('gmock', disabler: true, required: false)
if not gtest_dep.found() or not gmock_dep.found()
    gtest_proj = import('cmake').subproject('googletest', required: false)
    if gtest_proj.found()
        gtest_dep = declare_dependency(
            dependencies: [
                dependency('threads'),
                gtest_proj.dependency('gtest'),
                gtest_proj.dependency('gtest_main'),
            ],
        )
        gmock_dep = gtest_proj.dependency('gmock')
    else
        assert(
            not get_option('tests').enabled(),
            'Googletest is required if tests are enabled',
        )
    endif
endif


daemon_test_files = [
    'daemon_tests.cpp',
    'dbus_server_tests.cpp',
    'eventlog_parser_tests.cpp',
    'http_client_tests.cpp',
    'logentry_tests.cpp',
    'redfish_binding_common_tests.cpp',
    'redfish_binding_tests.cpp',
    'sensor_tests.cpp',
    'source_tests.cpp',
]

foreach t : daemon_test_files
    test(
        t.underscorify(),
        executable(
            'test-' + t.underscorify(),
            t,
            dependencies: [gtest_dep, gmock_dep, base_deps],
            cpp_args: cpp_args,
            link_args: link_args,
            link_with: redfish_client_library,
            include_directories: '../',
        ),
    )
endforeach
