//
//  GameSpace.swift
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
import RealityKit

struct GameSpace: View {
    @State private var renderer = ScreenRenderer.shared
    @State private var displayManager = DisplayManager.shared

    var body: some View {
        RealityView { content in
            let root = Entity()
            content.add(root)
            displayManager.rootEntity = root
        }
        .task {
            await displayManager.startTracking()
            displayManager.placeDisplayCircle()
        }
        .onChange(of: renderer.screenImage) { oldValue, newValue in
            guard let image = newValue, let cgImage = image.cgImage else { return }
            displayManager.updateTexture(cgImage)
        }
    }
}
