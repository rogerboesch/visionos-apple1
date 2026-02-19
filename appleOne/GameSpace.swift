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
        RealityView { content, attachments in
            let root = Entity()
            content.add(root)
            displayManager.rootEntity = root

            // Add the main control panel as a tilted attachment
            if let panelAttachment = attachments.entity(for: "control_panel") {
                root.addChild(panelAttachment)
                displayManager.panelEntity = panelAttachment
            }
        } attachments: {
            Attachment(id: "control_panel") {
                ControlPanel()
                    .frame(width: 600, height: 500)
                    .glassBackgroundEffect()
            }
        }
        .task {
            await displayManager.startTracking()
            displayManager.placePanel()
            displayManager.placeDisplayCircle()

            // Update loop: billboard rotation + carousel
            let frameDelta: Float = 1.0 / 60.0
            while true {
                displayManager.updatePanelBillboard()
                displayManager.updateCarousel(deltaTime: frameDelta)
                try? await Task.sleep(for: .milliseconds(16))
            }
        }
        .onChange(of: renderer.screenImage) { oldValue, newValue in
            guard let image = newValue, let cgImage = image.cgImage else { return }
            displayManager.updateTexture(cgImage)
        }
        .onChange(of: renderer.portraitSingleCount) { oldValue, newValue in
            guard let image = renderer.portraitImage, let cg = image.cgImage
            else { return }
            displayManager.updatePortraitSingle(cg)
        }
        .onChange(of: renderer.portraitPairCount) { oldValue, newValue in
            guard let imageA = renderer.portraitImageA, let cgA = imageA.cgImage,
                  let imageB = renderer.portraitImageB, let cgB = imageB.cgImage
            else { return }
            displayManager.updatePortraitPair(cgA, cgB)
        }
    }
}
