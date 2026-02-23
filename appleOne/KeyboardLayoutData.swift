import Foundation

// MARK: - Key definition

struct KeyDef {
    let normal: String
    let shifted: String
    let asciiNormal: Int32
    let asciiShifted: Int32
    let width: CGFloat
    let isSpecial: Bool

    init(_ normal: String, _ shifted: String,
         _ asciiNormal: Int32, _ asciiShifted: Int32,
         width: CGFloat = KEY_WIDTH_STANDARD, isSpecial: Bool = false)
    {
        self.normal = normal
        self.shifted = shifted
        self.asciiNormal = asciiNormal
        self.asciiShifted = asciiShifted
        self.width = width
        self.isSpecial = isSpecial
    }
}

// MARK: - Key width multipliers (relative to standard key)

let KEY_WIDTH_STANDARD: CGFloat = 1.0
let KEY_WIDTH_WIDE: CGFloat = 1.3
let KEY_WIDTH_SHIFT: CGFloat = 1.8
let KEY_WIDTH_RETURN: CGFloat = 1.8
let KEY_WIDTH_RUB_OUT: CGFloat = 1.3
let KEY_WIDTH_SPACEBAR: CGFloat = 8.0

// MARK: - Special action sentinels (negative = not an ASCII code)

let KEY_ACTION_RESET: Int32 = -1
let KEY_ACTION_CTRL: Int32 = -2
let KEY_ACTION_SHIFT: Int32 = -3
let KEY_ACTION_BREAK: Int32 = -4

// MARK: - Keyboard rows (Datanetics DC-50 layout)
// Apple I number row is inverted: unshifted = symbols, shifted = digits

let KEYBOARD_ROW_1: [KeyDef] = [
    KeyDef("!", "1", 0x21, 0x31),
    KeyDef("\"", "2", 0x22, 0x32),
    KeyDef("#", "3", 0x23, 0x33),
    KeyDef("$", "4", 0x24, 0x34),
    KeyDef("%", "5", 0x25, 0x35),
    KeyDef("&", "6", 0x26, 0x36),
    KeyDef("'", "7", 0x27, 0x37),
    KeyDef("(", "8", 0x28, 0x38),
    KeyDef(")", "9", 0x29, 0x39),
    KeyDef("*", "0", 0x2A, 0x30),
    KeyDef(":", ":", 0x3A, 0x3A),
    KeyDef("-", "=", 0x2D, 0x3D),
    KeyDef("RESET", "RESET", KEY_ACTION_RESET, KEY_ACTION_RESET,
           width: KEY_WIDTH_WIDE, isSpecial: true),
    KeyDef("BREAK", "BREAK", KEY_ACTION_BREAK, KEY_ACTION_BREAK,
           width: KEY_WIDTH_WIDE, isSpecial: true),
]

let KEYBOARD_ROW_2: [KeyDef] = [
    KeyDef("ESC", "ESC", 0x1B, 0x1B, isSpecial: true),
    KeyDef("Q", "Q", 0x51, 0x51),
    KeyDef("W", "W", 0x57, 0x57),
    KeyDef("E", "E", 0x45, 0x45),
    KeyDef("R", "R", 0x52, 0x52),
    KeyDef("T", "T", 0x54, 0x54),
    KeyDef("Y", "Y", 0x59, 0x59),
    KeyDef("U", "U", 0x55, 0x55),
    KeyDef("I", "I", 0x49, 0x49),
    KeyDef("O", "O", 0x4F, 0x4F),
    KeyDef("P", "P", 0x50, 0x50),
    KeyDef("RUB", "RUB", 0x5F, 0x5F, width: KEY_WIDTH_RUB_OUT, isSpecial: true),
    KeyDef("RETURN", "RETURN", 0x0D, 0x0D, width: KEY_WIDTH_RETURN, isSpecial: true),
]

let KEYBOARD_ROW_3: [KeyDef] = [
    KeyDef("CTRL", "CTRL", KEY_ACTION_CTRL, KEY_ACTION_CTRL,
           width: KEY_WIDTH_WIDE, isSpecial: true),
    KeyDef("A", "A", 0x41, 0x41),
    KeyDef("S", "S", 0x53, 0x53),
    KeyDef("D", "D", 0x44, 0x44),
    KeyDef("F", "F", 0x46, 0x46),
    KeyDef("G", "G", 0x47, 0x47),
    KeyDef("H", "H", 0x48, 0x48),
    KeyDef("J", "J", 0x4A, 0x4A),
    KeyDef("K", "K", 0x4B, 0x4B),
    KeyDef("L", "L", 0x4C, 0x4C),
    KeyDef(";", "+", 0x3B, 0x2B),
    KeyDef("+", "+", 0x2B, 0x2B),
    KeyDef("\u{2190}", "\u{2190}", 0x08, 0x08, isSpecial: true),
]

let KEYBOARD_ROW_4: [KeyDef] = [
    KeyDef("SHIFT", "SHIFT", KEY_ACTION_SHIFT, KEY_ACTION_SHIFT,
           width: KEY_WIDTH_SHIFT, isSpecial: true),
    KeyDef("Z", "Z", 0x5A, 0x5A),
    KeyDef("X", "X", 0x58, 0x58),
    KeyDef("C", "C", 0x43, 0x43),
    KeyDef("V", "V", 0x56, 0x56),
    KeyDef("B", "B", 0x42, 0x42),
    KeyDef("N", "N", 0x4E, 0x4E),
    KeyDef("M", "M", 0x4D, 0x4D),
    KeyDef(",", "<", 0x2C, 0x3C),
    KeyDef(".", ">", 0x2E, 0x3E),
    KeyDef("/", "?", 0x2F, 0x3F),
    KeyDef("SHIFT", "SHIFT", KEY_ACTION_SHIFT, KEY_ACTION_SHIFT,
           width: KEY_WIDTH_SHIFT, isSpecial: true),
]

let KEYBOARD_ROW_5: [KeyDef] = [
    KeyDef("", "", 0x20, 0x20, width: KEY_WIDTH_SPACEBAR),
]

let KEYBOARD_ROWS: [[KeyDef]] = [
    KEYBOARD_ROW_1,
    KEYBOARD_ROW_2,
    KEYBOARD_ROW_3,
    KEYBOARD_ROW_4,
    KEYBOARD_ROW_5,
]
