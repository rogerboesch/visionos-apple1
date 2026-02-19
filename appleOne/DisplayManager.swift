import Foundation
import ARKit
import RealityKit
import QuartzCore

// Display size in meters
let DISPLAY_WIDTH: Float = 1.5 * 5.0
let DISPLAY_HEIGHT: Float = (1.5 * (208.0 / 336.0)) * 4.0

let DISPLAY_DEPTH: Float = 0.005

// Minimum distance from floor to bottom of display (in meters)
let DISPLAY_MIN_FLOOR_GAP: Float = 0.5

// Offset from head: placed slightly in front of the user
let DISPLAY_PLACE_DISTANCE: Float = 0.5

// Head-follow panel distance and position
let PANEL_DISTANCE: Float = 0.8
let PANEL_VERTICAL_OFFSET: Float = -0.3
let PANEL_SMOOTHING: Float = 0.1

@Observable
@MainActor
class DisplayManager {
    static let shared = DisplayManager()

    let session = ARKitSession()
    let worldTracking = WorldTrackingProvider()

    var rootEntity: Entity? = nil
    var panelEntity: Entity? = nil
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

    func updatePanelPosition() {
        guard let panel = panelEntity else { return }

        guard worldTracking.state == .running,
              let deviceAnchor = worldTracking.queryDeviceAnchor(atTimestamp: CACurrentMediaTime())
        else {
            return
        }

        let headMatrix = deviceAnchor.originFromAnchorTransform

        // Target position: in front of the head, slightly below eye level
        let offset = simd_float4(0, PANEL_VERTICAL_OFFSET, -PANEL_DISTANCE, 1)
        let targetPos = headMatrix * offset
        let target = simd_float3(targetPos.x, targetPos.y, targetPos.z)

        // Smooth follow (lerp)
        let current = panel.position
        panel.position = current + (target - current) * PANEL_SMOOTHING

        // Face the user
        let headTransform = Transform(matrix: headMatrix)
        let currentRot = panel.orientation
        panel.orientation = simd_slerp(currentRot, headTransform.rotation, PANEL_SMOOTHING)
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

        // Clamp Y so bottom edge stays above floor
        let minY = DISPLAY_MIN_FLOOR_GAP + DISPLAY_HEIGHT / 2.0
        let clampedY = max(worldPos.y, minY)
        display.position = simd_float3(worldPos.x, clampedY, worldPos.z)

        // Use head orientation so display faces the user
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
        let fallbackY = DISPLAY_MIN_FLOOR_GAP + DISPLAY_HEIGHT / 2.0
        display.position = [0, fallbackY, -3.7]
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
