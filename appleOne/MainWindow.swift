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

struct MainWindow: View {
    @Environment(\.openImmersiveSpace) var openImmersiveSpace
    @Environment(\.dismissImmersiveSpace) var dismissImmersiveSpace
    @Environment(\.openWindow) var openWindow
    @Environment(\.dismissWindow) var dismissWindow

    let text = "The Apple Computer 1 (Apple-1), later known predominantly as the Apple I (written with a Roman numeral), is an 8-bit motherboard-only personal computer designed by Steve Wozniak and released by the Apple Computer Company (now Apple Inc.) in 1976. The company was initially formed to sell the Apple I – its first product – and would later become the world's largest technology company. The idea of starting a company and selling the computer came from Wozniak's friend and Apple co-founder Steve Jobs. One of the main innovations of the Apple I was that it included video display terminal circuitry and a keyboard interface on a single board, allowing it to connect to a low-cost composite video monitor instead of an expensive computer terminal, compared to most existing computers at the time. Contrary to popular belief, it was not the first personal computer to include such video output, predated by machines such as the Sol-20 and add-in cards such as the VDM-1."

    var body: some View {
        HStack() {
            Text(" ")
            VStack {
                Text("APPLE I")
                    .font(.system(size: 38))
                Text("")
                Text(text)
                    .font(.system(size: 22))
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
