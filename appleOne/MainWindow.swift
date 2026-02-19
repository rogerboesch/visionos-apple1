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

struct MainWindow: View {
    @State private var renderer = ScreenRenderer.shared
    @State var basicState = AppleState.cold
    @State var assemblerState = AppleState.cold

    let apple1Info = "The Apple Computer 1 (Apple-1), later known predominantly as the Apple I (written with a Roman numeral), is an 8-bit motherboard-only personal computer designed by Steve Wozniak and released by the Apple Computer Company (now Apple Inc.) in 1976."

    let basicListing = "10 PRINT \"Hello Apple I \";\n20 GOTO 10\nRUN\n"
    let assemblerListing = " LDA #'A'\nLOOP JSR $FFEF\n CLC\n ADC #$1\n CMP #'Z'+1\n BNE LOOP\n RTS\n~a\n"

    private let emulatorFPS = 1.0 / 60.0

    var body: some View {
        VStack(spacing: 16) {
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

            // Info text
            Text(apple1Info)
                .font(.system(size: 14))
                .foregroundColor(.secondary)

            // Control buttons
            HStack(spacing: 12) {
                Button(action: {
                    EmulatorHardReset()
                    self.basicState = .cold
                    self.assemblerState = .cold
                }) {
                    Text("RESET")
                }
                .buttonStyle(.bordered)

                Button(action: {
                    KeyboardHandler().sendText("~")
                }) {
                    Text("BREAK")
                }
                .buttonStyle(.bordered)

                basicButtons

                assemblerButtons
            }
        }
        .padding()
        .onAppear {
            EmulatorInit()

            Timer.scheduledTimer(withTimeInterval: emulatorFPS, repeats: true) { _ in
                EmulatorFrame()
            }
        }
    }

    @ViewBuilder
    private var basicButtons: some View {
        switch self.basicState {
        case .cold:
            Button(action: {
                EmulatorLoadBasic()
                self.basicState = .loaded
            }) {
                Text("LOAD BASIC")
            }
            .buttonStyle(.bordered)
        case .loaded:
            Button(action: {
                KeyboardHandler().sendText("e000r\n")
                self.basicState = .started
            }) {
                Text("START BASIC")
            }
            .buttonStyle(.bordered)
        case .started:
            Button(action: {
                KeyboardHandler().sendText(basicListing)
            }) {
                Text("TYPE LISTING")
            }
            .buttonStyle(.bordered)
        }
    }

    @ViewBuilder
    private var assemblerButtons: some View {
        switch self.assemblerState {
        case .cold:
            Button(action: {
                EmulatorLoadCore()
                self.assemblerState = .loaded
            }) {
                Text("LOAD ASSEMBLER")
            }
            .buttonStyle(.bordered)
        case .loaded:
            HStack(spacing: 12) {
                Button(action: {
                    KeyboardHandler().sendText("f000r\n")
                    self.assemblerState = .started
                }) {
                    Text("START ASSEMBLER")
                }
                .buttonStyle(.bordered)

                Button(action: {
                    KeyboardHandler().sendText("e2b3r\n")
                    KeyboardHandler().sendText("run\n", wait: 1.0)
                }) {
                    Text("RUN EXAMPLE")
                }
                .buttonStyle(.bordered)
            }
        case .started:
            Button(action: {
                KeyboardHandler().sendText("n\n")
                KeyboardHandler().sendText(assemblerListing, wait: 1.0)
                KeyboardHandler().sendText("r $300\n", wait: 10.0)
            }) {
                Text("TYPE LISTING")
            }
            .buttonStyle(.bordered)
        }
    }
}
