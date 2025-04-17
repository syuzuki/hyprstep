# hyprstep

A [Hyprland] plugin to add additional monitor specifier to find by the positions.

[Hyprland]: https://hyprland.org/

## Installation

### hyprpm (Recommended)

To install, run:

```sh
hyprpm add https://github.com/syuzuki/hyprstep.git
hyprpm enable hyprstep
```

To update, run:

```sh
hyprpm update
```

### Manual

To build, run:

```sh
meson setup build --wipe
ninja -C build
```

The library will be generated at build/hyprstep.so.

To load, run:

```sh
hyprpm plugin load $SO_PATH
```

`$SO_PATH` should be the full path to `hyprstep.so`.

## Configuration

This plugin add monitor specifiers with prefix `step:`.
The syntax is `step:<B><D>` where `<B>` is a base position command and `<D>` is a zero or more direction commands.

| Base Position Command | Description                   |
|-----------------------|-------------------------------|
| `o`                   | The monitor with position 0x0 |

| Direction Command | Description          |
|-------------------|----------------------|
| `l`               | Left of the monitor  |
| `r`               | Right of the monitor |
| `u`, `t`          | The upper monitor    |
| `d`, `b`          | The lower monitor    |

This specifier can be used in any context to select a monitor by string.

Example configuration:

```
# DP-1 is the origin and DP-2 is the right of DP-1
monitor = DP-1, 1920x1080, 0x0, 1
monitor = DP-2, 1920x1080, 1920x0, 1

# Focus DP-1
bind = SUPER, u, focusmonitor, step:o
# Focus DP-2
bind = SUPER, i, focusmonitor, step:or
# Also focus DP-2
bind = SUPER, o, focusmonitor, step:orlrlrlrlr

# Some commands require additional prefix
bind = SUPER, U, movewindow, mon:step:o
```

## Known problems

There are some known problems but currently not planned to fix them.

* Does not work on `monitor` rule in `windowrule`

## License

Licensed under either of

 * Apache License, Version 2.0
   ([LICENSE-APACHE](LICENSE-APACHE) or http://www.apache.org/licenses/LICENSE-2.0)
 * MIT license
   ([LICENSE-MIT](LICENSE-MIT) or http://opensource.org/licenses/MIT)

at your option.

## Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, as defined in the Apache-2.0 license, shall be dual licensed as above, without any additional terms or conditions.
