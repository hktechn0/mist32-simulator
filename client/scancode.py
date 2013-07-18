#-*- coding: utf-8 -*-
# Convert Gtk hardware_keycode to raw PS/2 scan code

scancode = [
    # 0x0x
    None, None, None, None, None, None, None, None,
    None, None, 0x16, 0x1e, 0x26, 0x25, 0x2e, 0x36,

    # 0x1x
    0x3d, 0x3e, 0x46, 0x45, None, None, None, None,
    None, None, None, None, None, None, None, None,

    # 0x2x
    None, None, None, None, None, None, None, None,
    None, None, None, None, None, None, None, None,

    # 0x3x
    None, None, None, None, None, None, None, None,
    None, None, None, None, None, None, None, None,

    # 0x4x
    None, None, None, None, None, None, None, None,
    None, None, None, None, None, None, None, None,

    # 0x5x
    None, None, None, None, None, None, None, None,
    None, None, None, None, None, None, None, None,

    # 0x6x
    None, None, None, None, None, None, None, None,
    None, None, None, None, None, None, None, None,

    # 0x7x
    None, (0xe0, 0x6b), None, None, None, None, None, None,
    None, None, None, None, None, None, None, None,

    # 0x8x
    None, None, None, None, None, None, None, None,
    None, None, None, None, None, None, None, None,

    # 0x9x
    None, None, None, None, None, None, None, None,
    None, None, None, None, None, None, None, None,

    # 0xAx
    None, None, None, None, None, None, None, None,
    None, None, None, None, None, None, None, None,

    # 0xBx
    None, None, None, None, None, None, None, None,
    None, None, None, None, None, None, None, None,

    # 0xCx
    None, None, None, None, None, None, None, None,
    None, None, None, None, None, None, None, None,

    # 0xDx
    None, None, None, None, None, None, None, None,
    None, None, None, None, None, None, None, None,

    # 0xEx
    None, None, None, None, None, None, None, None,
    None, None, None, None, None, None, None, None,

    # 0xFx
    None, None, None, None, None, None, None, None,
    None, None, None, None, None, None, None, None,
]
