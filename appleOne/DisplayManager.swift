import Foundation
import ARKit
import RealityKit
import QuartzCore

// Display size in meters
let DISPLAY_WIDTH: Float = 1.5
let DISPLAY_HEIGHT: Float = DISPLAY_WIDTH * (208.0 / 336.0)
let DISPLAY_DEPTH: Float = 0.005

// Offset from head: placed slightly in front of the user
let DISPLAY_PLACE_DISTANCE: Float = 0.5

@Observable
@MainActor
class DisplayManager {
    static let shared = DisplayManager()

    private let session = ARKitSession()
    private let worldTracking = WorldTrackingProvider()

    var rootEntity: Entity? = nil
    var displayEntity: ModelEntity? = nil

    private var textureIndex = 0
    private var trackingStarted = false

    func startTracking() async {
        guard !trackingStarted else { return }
        trackingStarted = true

        do {
            try await session.run([worldTracking])
        }
        catch {
            rbDebug("ARKit session failed: \(error.localizedDescription)")
        }
    }

    func placeDisplay() {
        guard let root = rootEntity else { return }

        // Remove existing display
        if let existing = displayEntity {
            existing.removeFromParent()
            displayEntity = nil
        }

        // Get current head pose
        guard worldTracking.state == .running,
              let deviceAnchor = worldTracking.queryDeviceAnchor(atTimestamp: CACurrentMediaTime())
        else {
            rbDebug("Head tracking not available, using fallback position")
            placeDisplayAtFallback()
            return
        }

        let headMatrix = deviceAnchor.originFromAnchorTransform

        // Calculate position: move forward from head along gaze direction
        let forward = simd_float4(0, 0, -DISPLAY_PLACE_DISTANCE, 1)
        let worldPos = headMatrix * forward

        // Create display entity
        let display = createDisplayEntity()
        display.position = simd_float3(worldPos.x, worldPos.y, worldPos.z)

        // Use head orientation but only keep the Y rotation (yaw)
        // so the display hangs flat like on a wall
        let headTransform = Transform(matrix: headMatrix)
        display.orientation = headTransform.rotation

        root.addChild(display)
        displayEntity = display
    }

    func placeDisplayAtFallback() {
        guard let root = rootEntity else { return }

        // Remove existing display
        if let existing = displayEntity {
            existing.removeFromParent()
            displayEntity = nil
        }

        let display = createDisplayEntity()
        display.position = [0, 1.6, -3.7]
        root.addChild(display)
        displayEntity = display
    }

    func updateTexture(_ cgImage: CGImage) {
        guard let entity = displayEntity else { return }

        textureIndex += 1
        let name = "display-\(textureIndex)"

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
                rbError("Display texture: \(error.localizedDescription)")
            }
        }
    }

    private func createDisplayEntity() -> ModelEntity {
        let mesh = MeshResource.generateBox(size: 1.0)
        let material = SimpleMaterial(color: .black, isMetallic: false)
        let display = ModelEntity(mesh: mesh, materials: [material])
        display.scale = [DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_DEPTH]
        return display
    }
}
