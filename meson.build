project(
  'hyprstep',
  'cpp',
  version: run_command('git', 'describe', '--abbrev=7', check: true)
    .stdout()
    .strip()
    .replace('-g', '.')
    .replace('-', '+dev.'),
  default_options: [
    'cpp_std=c++23',
    'warning_level=3',
    'buildtype=release',
  ]
)

shared_module(
  meson.project_name(),
  [
    'src/main.cpp',
  ],
  include_directories: ['src'],
  cpp_args: ['-DVERSION="@0@"'.format(meson.project_version())],
  dependencies: [
    dependency('hyprland', include_type: 'system'),
    dependency('pixman-1', include_type: 'system'),
  ],
  name_prefix: '',
  install: true,
)
