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

// Wall display size in meters
private let WALL_DISPLAY_WIDTH: Float = 2.0
private let WALL_DISPLAY_HEIGHT: Float = WALL_DISPLAY_WIDTH * (208.0 / 336.0)
private let WALL_DISPLAY_DEPTH: Float = 0.01

struct GameSpace: View {
    @State private var renderer = ScreenRenderer.shared
    @State private var wallEntity: ModelEntity? = nil
    @State private var textureIndex = 0

    var body: some View {
        RealityView { content in
            let anchor: AnchorEntity

            if isVisionOSDevice() {
                // On device: attach to the nearest vertical wall
                anchor = AnchorEntity(.plane(.vertical, classification: .wall, minimumBounds: [1.0, 0.5]))
            }
            else {
                // In simulator: place at a fixed position in front of the user
                anchor = AnchorEntity(world: [0, 1.5, -2.0])
            }

            // Create a flat box as the display surface
            let mesh = MeshResource.generateBox(size: 1.0)
            let material = SimpleMaterial(color: .black, isMetallic: false)
            let display = ModelEntity(mesh: mesh, materials: [material])
            display.scale = [WALL_DISPLAY_WIDTH, WALL_DISPLAY_HEIGHT, WALL_DISPLAY_DEPTH]

            anchor.addChild(display)
            content.add(anchor)

            wallEntity = display
        }
        .onChange(of: renderer.screenImage) { oldValue, newValue in
            guard let image = newValue, let cgImage = image.cgImage else { return }
            updateWallTexture(cgImage)
        }
    }

    private func updateWallTexture(_ cgImage: CGImage) {
        guard let entity = wallEntity else { return }

        textureIndex += 1
        let name = "wall-\(textureIndex)"

        Task {
            do {
                let resource = try await TextureResource(
                    image: cgImage,
                    withName: name,
                    options: TextureResource.CreateOptions(semantic: .raw)
                )

                var material = SimpleMaterial(color: .white, isMetallic: false)
                material.color = .init(tint: .white, texture: .init(resource))
                entity.model?.materials = [material]
            }
            catch {
                rbError("Wall texture: \(error.localizedDescription)")
            }
        }
    }
}
