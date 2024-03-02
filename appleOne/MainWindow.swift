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
    @Environment(\.openImmersiveSpace) var openImmersiveSpace
    @Environment(\.dismissImmersiveSpace) var dismissImmersiveSpace
    @Environment(\.openWindow) var openWindow
    @Environment(\.dismissWindow) var dismissWindow

    @State var basicState = AppleState.cold
    @State var assemblerState = AppleState.cold

    let apple1Info = "The Apple Computer 1 (Apple-1), later known predominantly as the Apple I (written with a Roman numeral), is an 8-bit motherboard-only personal computer designed by Steve Wozniak and released by the Apple Computer Company (now Apple Inc.) in 1976. The company was initially formed to sell the Apple I – its first product – and would later become the world's largest technology company. The idea of starting a company and selling the computer came from Wozniak's friend and Apple co-founder Steve Jobs. One of the main innovations of the Apple I was that it included video display terminal circuitry and a keyboard interface on a single board, allowing it to connect to a low-cost composite video monitor instead of an expensive computer terminal, compared to most existing computers at the time. Contrary to popular belief, it was not the first personal computer to include such video output, predated by machines such as the Sol-20 and add-in cards such as the VDM-1."
    let basicListing = "10 PRINT \"Hello Apple I \";\n20 GOTO 10\nRUN\n"
    let assemblerListing = " LDA #'A'\nLOOP JSR $FFEF\n CLC\n ADC #$1\n CMP #'Z'+1\n BNE LOOP\n RTS\n~a\n"

    var body: some View {
        HStack() {
            Text(" ")
            VStack {
                Text("APPLE I")
                    .font(.system(size: 38))
                Text("")
                Text(apple1Info)
                    .font(.system(size: 22))
                
                HStack() {
                    Button(action: {
                        EmulatorHardReset()
                        self.basicState = .cold
                        self.assemblerState = .cold
                    }) {
                        Text("RESET")
                    }
                    .padding()
                    .buttonStyle(.bordered)
                    
                    Button(action: {
                        KeyboardHandler().sendText("~")
                    }) {
                        Text("BREAK")
                    }
                    .padding()
                    .buttonStyle(.bordered)

                    // BASIC
                    switch self.basicState {
                    case .cold:
                        Button(action: {
                            EmulatorLoadBasic()
                            self.basicState = .loaded
                        }) {
                            Text("LOAD BASIC")
                        }
                        .padding()
                        .buttonStyle(.bordered)
                    case .loaded:
                        Button(action: {
                            KeyboardHandler().sendText("e000r\n")
                            self.basicState = .started
                        }) {
                            Text("START BASIC")
                        }
                        .padding()
                        .buttonStyle(.bordered)
                    case .started:
                        Button(action: {
                            KeyboardHandler().sendText(basicListing)
                        }) {
                            Text("TYPE LISTING")
                        }
                        .padding()
                        .buttonStyle(.bordered)
                    }
                    
                    // ASSEMBLER
                    switch self.assemblerState {
                    case .cold:
                        Button(action: {
                            EmulatorLoadCore()
                            self.assemblerState = .loaded
                        }) {
                            Text("LOAD ASSEMBLER")
                        }
                        .padding()
                        .buttonStyle(.bordered)
                    case .loaded:
                        Button(action: {
                            KeyboardHandler().sendText("f000r\n")
                            self.assemblerState = .started
                        }) {
                            Text("START ASSEMBLER")
                        }
                        .padding()
                        .buttonStyle(.bordered)
                    case .started:
                        Button(action: {
                            KeyboardHandler().sendText("n\n")
                            KeyboardHandler().sendText(assemblerListing, wait: 1.0)
                            KeyboardHandler().sendText("r $300\n", wait: 10.0)
                        }) {
                            Text("TYPE LISTING")
                        }
                        .padding()
                        .buttonStyle(.bordered)
                    }
                    
                    if (self.assemblerState == .loaded) {
                        Button(action: {
                            KeyboardHandler().sendText("e2b3r\n")
                            KeyboardHandler().sendText("run\n", wait: 1.0)
                        }) {
                            Text("RUN EXAMPLE")
                        }
                        .padding()
                        .buttonStyle(.bordered)
                    }
                }
            }
            Text(" ")
        }
        .padding()
        .onAppear() {
            Task {
                await openImmersiveSpace(id: "game_space")
            }
        }
    }
}
