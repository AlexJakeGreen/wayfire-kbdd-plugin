project(
    'wayfire-kbdd-plugin',
    'c',
    'cpp',
    version: '0.1',
    license: 'MIT',
    meson_version: '>=0.51.0',
    default_options: [
        'cpp_std=c++17',
        'c_std=c11',
        'warning_level=2',
        'werror=false',
    ],
)

wayfire = dependency('wayfire')
wlroots = dependency('wlroots')
wfconfig = dependency('wf-config')
xkbcommon = dependency('xkbcommon')
xkbregistry = dependency('xkbregistry')

add_project_arguments(['-DWLR_USE_UNSTABLE'], language: ['cpp', 'c'])
add_project_arguments(['-DWAYFIRE_PLUGIN'], language: ['cpp', 'c'])
add_project_link_arguments(['-rdynamic'], language: 'cpp')

subdir('src')
subdir('metadata')

summary = [
    '',
    '----------------',
    'wayfire-kbdd-plugin @0@'.format(meson.project_version()),
    '----------------',
    ''
]
message('\n'.join(summary))
