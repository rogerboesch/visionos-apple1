import SwiftUI

// MARK: - Visual constants

private let KB_COLOR_PCB = Color(red: 0.05, green: 0.15, blue: 0.05)
private let KB_COLOR_KEY_CAP = Color(red: 0.18, green: 0.18, blue: 0.18)
private let KB_COLOR_KEY_LABEL = Color.white
private let KB_COLOR_ACCENT_RED = Color(red: 0.85, green: 0.15, blue: 0.15)
private let KB_COLOR_ACCENT_GREEN = Color(red: 0.2, green: 0.8, blue: 0.2)
private let KB_COLOR_BRAND = Color(red: 0.6, green: 0.6, blue: 0.5)

private let KB_KEY_SIZE: CGFloat = 38
private let KB_KEY_SPACING: CGFloat = 3
private let KB_KEY_CORNER_RADIUS: CGFloat = 5
private let KB_FONT_SIZE: CGFloat = 11
private let KB_FONT_SIZE_SMALL: CGFloat = 9

// MARK: - Keyboard panel view

struct KeyboardPanel: View {
    @State private var appState = AppState.shared
    @State private var shiftActive = false
    @State private var ctrlActive = false

    var body: some View {
        if appState.appMode == .emulator {
            VStack(spacing: 0) {
                VStack(spacing: KB_KEY_SPACING) {
                    ForEach(0..<KEYBOARD_ROWS.count, id: \.self) { rowIndex in
                        keyboardRow(KEYBOARD_ROWS[rowIndex])
                    }
                }
                .padding(.horizontal, 12)
                .padding(.top, 12)
                .padding(.bottom, 6)

                // Bottom brand label
                HStack {
                    Text("Datanetics Corp.")
                        .font(.system(size: 9, weight: .medium, design: .monospaced))
                        .foregroundColor(KB_COLOR_BRAND)
                    Spacer()
                    Text("DC-50  SN:1976")
                        .font(.system(size: 9, weight: .medium, design: .monospaced))
                        .foregroundColor(KB_COLOR_BRAND)
                }
                .padding(.horizontal, 16)
                .padding(.bottom, 8)
            }
            .background(KB_COLOR_PCB)
        }
    }

    // MARK: - Row builder

    private func keyboardRow(_ keys: [KeyDef]) -> some View {
        HStack(spacing: KB_KEY_SPACING) {
            ForEach(0..<keys.count, id: \.self) { index in
                keyView(keys[index])
            }
        }
    }

    // MARK: - Single key view

    private func keyView(_ key: KeyDef) -> some View {
        let label = shiftActive ? key.shifted : key.normal
        let keyWidth = KB_KEY_SIZE * key.width + KB_KEY_SPACING * (key.width - 1)
        let isReset = (key.asciiNormal == KEY_ACTION_RESET)
        let isShift = (key.asciiNormal == KEY_ACTION_SHIFT)
        let isCtrl = (key.asciiNormal == KEY_ACTION_CTRL)
        let isSpacebar = (key.width == KEY_WIDTH_SPACEBAR)

        return Button(action: {
            handleKeyPress(key)
        }) {
            Text(isSpacebar ? "" : label)
                .font(.system(
                    size: key.isSpecial ? KB_FONT_SIZE_SMALL : KB_FONT_SIZE,
                    weight: .bold,
                    design: .monospaced
                ))
                .foregroundColor(keyLabelColor(isReset: isReset, isShift: isShift, isCtrl: isCtrl))
                .frame(width: keyWidth, height: KB_KEY_SIZE)
                .background(
                    RoundedRectangle(cornerRadius: KB_KEY_CORNER_RADIUS)
                        .fill(keyCapColor(isReset: isReset, isShift: isShift, isCtrl: isCtrl))
                )
                .overlay(
                    RoundedRectangle(cornerRadius: KB_KEY_CORNER_RADIUS)
                        .strokeBorder(Color.white.opacity(0.08), lineWidth: 1)
                )
        }
        .buttonStyle(.plain)
    }

    // MARK: - Key colors

    private func keyCapColor(isReset: Bool, isShift: Bool, isCtrl: Bool) -> Color {
        if isReset {
            return KB_COLOR_ACCENT_RED.opacity(0.3)
        }
        if isShift && shiftActive {
            return KB_COLOR_ACCENT_GREEN.opacity(0.3)
        }
        if isCtrl && ctrlActive {
            return KB_COLOR_ACCENT_GREEN.opacity(0.3)
        }
        return KB_COLOR_KEY_CAP
    }

    private func keyLabelColor(isReset: Bool, isShift: Bool, isCtrl: Bool) -> Color {
        if isReset {
            return KB_COLOR_ACCENT_RED
        }
        if isShift && shiftActive {
            return KB_COLOR_ACCENT_GREEN
        }
        if isCtrl && ctrlActive {
            return KB_COLOR_ACCENT_GREEN
        }
        return KB_COLOR_KEY_LABEL
    }

    // MARK: - Key press handling

    private func handleKeyPress(_ key: KeyDef) {
        let code = shiftActive ? key.asciiShifted : key.asciiNormal

        switch code {
        case KEY_ACTION_RESET:
            EmulatorHardReset()
            appState.basicState = .cold
            appState.assemblerState = .cold

        case KEY_ACTION_BREAK:
            EmulatorSkipSplash()

        case KEY_ACTION_SHIFT:
            shiftActive.toggle()

        case KEY_ACTION_CTRL:
            ctrlActive.toggle()

        default:
            var ascii = code

            // CTRL + letter sends control character (ASCII 0x01-0x1A)
            if ctrlActive && ascii >= 0x41 && ascii <= 0x5A {
                ascii = ascii - 0x40
                ctrlActive = false
            }

            EmulatorKeyPress(ascii)

            // Auto-release shift after one character
            if shiftActive {
                shiftActive = false
            }
        }
    }
}
