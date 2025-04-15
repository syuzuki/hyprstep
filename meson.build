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
    dependency('aquamarine', include_type: 'system'),
    dependency('hyprgraphics', include_type: 'system'),
    dependency('hyprland', include_type: 'system'),
    dependency('hyprutils', include_type: 'system'),
    dependency('libdrm', include_type: 'system'),
    dependency('libglvnd', include_type: 'system'),
    dependency('libinput', include_type: 'system'),
    dependency('pixman-1', include_type: 'system'),
    dependency('wayland-server', include_type: 'system'),
    dependency('xkbcommon', include_type: 'system'),
  ],
  name_prefix: '',
  install: true,
)
