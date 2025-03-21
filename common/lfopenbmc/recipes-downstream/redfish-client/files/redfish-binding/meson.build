prog_python = find_program('python3')

output = run_command(
    [
        prog_python,
        meson.current_source_dir() / 'codegen.py',
        '-l',
        '-s',
        dsp_spec_jsondir,
    ],
    check: true,
).stdout().strip().split(
    '\n',
)

redfish_binding = custom_target(
    'redfish-binding',
    input: ['codegen.py'],
    output: output,
    command: [prog_python, '@INPUT@', '-o', '@OUTDIR@', '-s', dsp_spec_jsondir],
)

redfish_binding_dep = declare_dependency(sources: redfish_binding)
