//
//  MainWindow.swift
//
//  Vision OS - From Zero to Hero
//  This code was written as part of a tutorial at https://visionos.substack.com
//
//  Created by Roger Boesch on 01/01/2024.
//
//  DISCLAIMER:
//  The intention of this tutorial is not to always write the best possible code but
//  to show different ways to create a game or app that even can be published.
//  I will also refactor a lot during the tutorial and improve things step by step
//  or even show completely different approaches.
//
//  Feel free to use the code in the way you want :)
//

import SwiftUI

enum AppleState {
    case cold
    case loaded
    case started
}

// Launcher window — initializes emulator and opens the immersive space
struct MainWindow: View {
    @Environment(\.openImmersiveSpace) var openImmersiveSpace
    @Environment(\.dismissWindow) var dismissWindow

    private let emulatorFPS = 1.0 / 60.0

    var body: some View {
        Color.clear
            .onAppear {
                EmulatorInit()

                Timer.scheduledTimer(withTimeInterval: emulatorFPS, repeats: true) { _ in
                    EmulatorFrame()
                }

                Task {
                    await openImmersiveSpace(id: "wall_display")
                    dismissWindow(id: "main_window")
                }
            }
    }
}

// MARK: - Colored button style (Shockwave-inspired)

struct PanelButton: View {
    let label: String
    var color: Color = .white
    let action: () -> Void

    var body: some View {
        Button(action: action) {
            Text(label)
                .font(.system(size: 13, weight: .bold, design: .monospaced))
                .foregroundColor(color)
                .padding(.horizontal, 14)
                .padding(.vertical, 10)
                .background(
                    RoundedRectangle(cornerRadius: 10)
                        .fill(color.opacity(0.15))
                )
                .overlay(
                    RoundedRectangle(cornerRadius: 10)
                        .strokeBorder(color.opacity(0.5), lineWidth: 1)
                )
        }
        .buttonStyle(.plain)
    }
}

// MARK: - Control panel UI (attachment in the immersive space)

struct ControlPanel: View {
    @State private var renderer = ScreenRenderer.shared
    @State private var displayManager = DisplayManager.shared
    @State var basicState = AppleState.cold
    @State var assemblerState = AppleState.cold

    let basicListing = "10 PRINT \"Hello Apple I \";\n20 GOTO 10\nRUN\n"
    let assemblerListing = " LDA #'A'\nLOOP JSR $FFEF\n CLC\n ADC #$1\n CMP #'Z'+1\n BNE LOOP\n RTS\n~a\n"

    var body: some View {
        VStack(spacing: 16) {
            // Title
            Text("APPLE I")
                .font(.system(size: 42, weight: .bold, design: .monospaced))
                .kerning(6)

            // Emulator screen + portrait buttons side by side
            HStack(spacing: 12) {
                // Emulator screen
                Group {
                    if let image = renderer.screenImage {
                        Image(uiImage: image)
                            .resizable()
                            .interpolation(.none)
                            .aspectRatio(contentMode: .fit)
                    }
                    else {
                        Rectangle()
                            .fill(Color.black)
                            .aspectRatio(336.0 / 208.0, contentMode: .fit)
                    }
                }
                .clipShape(RoundedRectangle(cornerRadius: 8))

                // Portrait buttons (right of screen)
                VStack(spacing: 8) {
                    PanelButton(label: "JOBS") {
                        EmulatorShowJobs()
                    }

                    PanelButton(label: "WOZ") {
                        EmulatorShowWozniak()
                    }

                    PanelButton(label: "BOTH") {
                        EmulatorShowBothSteves()
                    }

                    PanelButton(
                        label: "ROTATE",
                        color: displayManager.carouselRotating ? .green : .white
                    ) {
                        displayManager.carouselRotating.toggle()
                    }
                }
            }

            // Control buttons
            HStack(spacing: 8) {
                PanelButton(label: "RESET", color: .red) {
                    EmulatorHardReset()
                    self.basicState = .cold
                    self.assemblerState = .cold
                }

                PanelButton(label: "BREAK", color: .orange) {
                    EmulatorSkipSplash()
                    KeyboardHandler().sendText("~")
                }

                basicButtons

                assemblerButtons
            }
        }
        .padding()
    }

    @ViewBuilder
    private var basicButtons: some View {
        switch self.basicState {
        case .cold:
            PanelButton(label: "LOAD BASIC", color: .cyan) {
                EmulatorLoadBasic()
                self.basicState = .loaded
            }
        case .loaded:
            PanelButton(label: "START BASIC", color: .green) {
                KeyboardHandler().sendText("e000r\n")
                self.basicState = .started
            }
        case .started:
            PanelButton(label: "TYPE LISTING", color: .blue) {
                KeyboardHandler().sendText(basicListing)
            }
        }
    }

    @ViewBuilder
    private var assemblerButtons: some View {
        switch self.assemblerState {
        case .cold:
            PanelButton(label: "LOAD ASSEMBLER", color: .cyan) {
                EmulatorLoadCore()
                self.assemblerState = .loaded
            }
        case .loaded:
            HStack(spacing: 8) {
                PanelButton(label: "START ASSEMBLER", color: .green) {
                    KeyboardHandler().sendText("f000r\n")
                    self.assemblerState = .started
                }

                PanelButton(label: "RUN EXAMPLE", color: .blue) {
                    KeyboardHandler().sendText("e2b3r\n")
                    KeyboardHandler().sendText("run\n", wait: 1.0)
                }
            }
        case .started:
            PanelButton(label: "TYPE LISTING", color: .blue) {
                KeyboardHandler().sendText("n\n")
                KeyboardHandler().sendText(assemblerListing, wait: 1.0)
                KeyboardHandler().sendText("r $300\n", wait: 10.0)
            }
        }
    }
}
