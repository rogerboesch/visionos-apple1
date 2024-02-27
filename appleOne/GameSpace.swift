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
import RealityKitContent

struct GameSpace: View {
    @State private var state_anchor: AnchorEntity? = nil        // Anchor where all game objecst are assigned
    @State private var apple1: Entity? = nil              // Models

    var body: some View {
        RealityView { content in
            if let scene = try? await Entity(named: "Immersive", in: realityKitContentBundle) {
                content.add(scene)

                // Anchor used for game objects
                if isVisionOSDevice() {
                    state_anchor = AnchorEntity(.plane(.horizontal, classification: .table, minimumBounds: [0.5, 0.5]))
                    rbDebug("Created plane anchor at \(state_anchor!.position.x),\(state_anchor!.position.y),\(state_anchor!.position.z)")
                }
                else {
                    state_anchor = AnchorEntity(world: [1, 1, -1.5])
                    rbDebug("Created world anchor at 1, 1, -1.5")
                }
                scene.addChild(state_anchor!)
                
                if let apple = scene.findEntity(named: "Apple_I") as Entity? {
                    apple1 = apple
                    apple1?.isEnabled = true
                    state_anchor?.addChild(apple1!)
                    
                    if let model = apple.children[0] as? ModelEntity {
                        model.model?.materials = [SimpleMaterial(color: UIColor.brown, isMetallic: false)]
                    }
                }

                let box = MeshResource.generateBox(size: 1.0)
                let material = SimpleMaterial(color: UIColor.white, isMetallic: false)
                let boxEntity = ModelEntity(mesh: box, materials: [material])
                boxEntity.scale = [0.4, 0.4, 0.01]
                boxEntity.position = [-0.01, 0.35, -0.2]
                
                Renderer.entity = boxEntity
                state_anchor!.addChild(boxEntity)
                
                EmulatorInit()
                
                let interval = 1.0/60.0
                Timer.scheduledTimer(withTimeInterval: interval, repeats: true) { timer in
                    EmulatorFrame()
                }

                let str = "e000r\n" // 10 PRINT \"HELLO WORLD \";\n20 GOTO 10\nRUN\n"
                let interval2 = 1.0/10.0
                var count = 0;
                
                Timer.scheduledTimer(withTimeInterval: interval2, repeats: true) { timer in
                    let char = str[str.index(str.startIndex, offsetBy: count)]
                    sendText(String(char))
                    count += 1
                    
                    if count >= str.count {
                        timer.invalidate()
                    }
                }
            }
        }
    }
    
    func sendText(_ str: String) {
        for ch in str {
            if let ascii = ch.asciiValue {
                EmulatorKeyPress(Int32(ascii));
            }
        }
    }
    
}
