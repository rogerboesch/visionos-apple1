import Foundation
import ARKit
import RealityKit
import QuartzCore

// Display size in meters (4:3 ratio matching main window)
let DISPLAY_WIDTH: Float = 7.5
let DISPLAY_HEIGHT: Float = 5.2

let DISPLAY_DEPTH: Float = 0.005

// Minimum distance from floor to bottom of display (in meters)
let DISPLAY_MIN_FLOOR_GAP: Float = 0.2

// Offset from head: placed slightly in front of the user
let DISPLAY_PLACE_DISTANCE: Float = 0.5

// Panel placement (fixed position, billboard rotation only)
let PANEL_DISTANCE: Float = 0.8
let PANEL_VERTICAL_OFFSET: Float = -0.1
let PANEL_SMOOTHING: Float = 0.1
let PANEL_TILT_DEGREES: Float = 30.0

// Circle layout
let CIRCLE_DISPLAY_COUNT = 8
let CIRCLE_RADIUS: Float = 10.0

// Carousel rotation speed (radians per second)
let CAROUSEL_SPEED: Float = 0.15

// Maximum number of displays
let DISPLAY_MAX_COUNT = 26

@Observable
@MainActor
class DisplayManager {
    static let shared = DisplayManager()

    let session = ARKitSession()
    let worldTracking = WorldTrackingProvider()

    var rootEntity: Entity? = nil
    var panelEntity: Entity? = nil
    var displayEntities: [ModelEntity] = []

    var carouselRotating = false

    private var textureIndex = 0
    private var trackingStarted = false
    private var carouselAngle: Float = 0
    private var circleCenter = simd_float2(0, 0)
    private var circleY: Float = 0

    var displayCount: Int { displayEntities.count }
    var canPlaceMore: Bool { displayEntities.count < DISPLAY_MAX_COUNT }

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

    func placePanel() {
        guard let panel = panelEntity else { return }

        guard worldTracking.state == .running,
              let deviceAnchor = worldTracking.queryDeviceAnchor(atTimestamp: CACurrentMediaTime())
        else {
            // Fallback: place at fixed position in front of origin
            panel.position = simd_float3(0, DISPLAY_MIN_FLOOR_GAP, -PANEL_DISTANCE)
            return
        }

        let headMatrix = deviceAnchor.originFromAnchorTransform

        // Place in front of the head, slightly below eye level
        let offset = simd_float4(0, PANEL_VERTICAL_OFFSET, -PANEL_DISTANCE, 1)
        let targetPos = headMatrix * offset
        panel.position = simd_float3(targetPos.x, targetPos.y, targetPos.z)
    }

    func updatePanelBillboard() {
        guard let panel = panelEntity else { return }

        guard worldTracking.state == .running,
              let deviceAnchor = worldTracking.queryDeviceAnchor(atTimestamp: CACurrentMediaTime())
        else {
            return
        }

        let headMatrix = deviceAnchor.originFromAnchorTransform

        // Billboard: face user using yaw only (ignore head pitch/roll)
        let headPos = simd_float3(headMatrix.columns.3.x,
                                  headMatrix.columns.3.y,
                                  headMatrix.columns.3.z)
        let dir = headPos - panel.position
        let yaw = atan2(dir.x, dir.z)
        let tilt = PANEL_TILT_DEGREES * (.pi / 180.0)

        // Yaw rotation (face user) then tilt like a table
        let yawQuat = simd_quatf(angle: yaw, axis: simd_float3(0, 1, 0))
        let tiltQuat = simd_quatf(angle: -tilt, axis: simd_float3(1, 0, 0))
        let targetRot = yawQuat * tiltQuat

        let currentRot = panel.orientation
        panel.orientation = simd_slerp(currentRot, targetRot, PANEL_SMOOTHING)
    }

    func placeDisplay() {
        guard let root = rootEntity, canPlaceMore else { return }

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

        let display = createDisplayEntity()

        // Clamp Y so bottom edge stays above floor
        let minY = DISPLAY_MIN_FLOOR_GAP + DISPLAY_HEIGHT / 2.0
        let clampedY = max(worldPos.y, minY)
        display.position = simd_float3(worldPos.x, clampedY, worldPos.z)

        // Use head orientation so display faces the user
        let headTransform = Transform(matrix: headMatrix)
        display.orientation = headTransform.rotation

        root.addChild(display)
        displayEntities.append(display)
    }

    func placeDisplayAtFallback() {
        guard let root = rootEntity, canPlaceMore else { return }

        let display = createDisplayEntity()
        let fallbackY = DISPLAY_MIN_FLOOR_GAP + DISPLAY_HEIGHT / 2.0
        display.position = [0, fallbackY, -3.7]
        root.addChild(display)
        displayEntities.append(display)
    }

    func placeDisplayCircle() {
        guard let root = rootEntity else { return }

        let displayY = DISPLAY_MIN_FLOOR_GAP + DISPLAY_HEIGHT / 2.0

        // Try to get head position as circle center, fall back to origin
        var centerX: Float = 0
        var centerZ: Float = 0

        if worldTracking.state == .running,
           let deviceAnchor = worldTracking.queryDeviceAnchor(atTimestamp: CACurrentMediaTime())
        {
            let headMatrix = deviceAnchor.originFromAnchorTransform
            centerX = headMatrix.columns.3.x
            centerZ = headMatrix.columns.3.z
        }

        // Store circle parameters for carousel rotation
        circleCenter = simd_float2(centerX, centerZ)
        circleY = displayY
        carouselAngle = 0

        let angleStep = (2.0 * Float.pi) / Float(CIRCLE_DISPLAY_COUNT)

        for i in 0..<CIRCLE_DISPLAY_COUNT {
            guard canPlaceMore else { break }

            let angle = Float(i) * angleStep

            let x = centerX + CIRCLE_RADIUS * sin(angle)
            let z = centerZ - CIRCLE_RADIUS * cos(angle)

            let display = createDisplayEntity()
            display.position = simd_float3(x, displayY, z)

            let center = simd_float3(centerX, displayY, centerZ)
            display.look(at: center, from: display.position, relativeTo: nil)

            root.addChild(display)
            displayEntities.append(display)
        }

        rbDebug("Placed \(CIRCLE_DISPLAY_COUNT) displays in circle (radius=\(CIRCLE_RADIUS)m)")
    }

    func updateCarousel(deltaTime: Float) {
        guard carouselRotating, !displayEntities.isEmpty else { return }

        carouselAngle += CAROUSEL_SPEED * deltaTime

        let angleStep = (2.0 * Float.pi) / Float(displayEntities.count)
        let cx = circleCenter.x
        let cz = circleCenter.y

        for (i, display) in displayEntities.enumerated() {
            let angle = Float(i) * angleStep + carouselAngle

            let x = cx + CIRCLE_RADIUS * sin(angle)
            let z = cz - CIRCLE_RADIUS * cos(angle)

            display.position = simd_float3(x, circleY, z)

            let center = simd_float3(cx, circleY, cz)
            display.look(at: center, from: display.position, relativeTo: nil)
        }
    }

    func removeAllDisplays() {
        for entity in displayEntities {
            entity.removeFromParent()
        }
        displayEntities.removeAll()
    }

    func updateTexture(_ cgImage: CGImage) {
        guard !displayEntities.isEmpty else { return }

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

                for entity in displayEntities {
                    entity.model?.materials = [material]
                }
            }
            catch {
                rbError("Display texture: \(error.localizedDescription)")
            }
        }
    }

    func updatePortraitPair(_ cgImageA: CGImage, _ cgImageB: CGImage) {
        guard !displayEntities.isEmpty else { return }

        textureIndex += 1
        let nameA = "portrait-a-\(textureIndex)"
        let nameB = "portrait-b-\(textureIndex)"

        Task {
            do {
                let resourceA = try await TextureResource(
                    image: cgImageA,
                    withName: nameA,
                    options: TextureResource.CreateOptions(semantic: .raw)
                )
                let resourceB = try await TextureResource(
                    image: cgImageB,
                    withName: nameB,
                    options: TextureResource.CreateOptions(semantic: .raw)
                )

                var materialA = SimpleMaterial(color: .white, isMetallic: false)
                materialA.color = .init(tint: .white, texture: .init(resourceA))

                var materialB = SimpleMaterial(color: .white, isMetallic: false)
                materialB.color = .init(tint: .white, texture: .init(resourceB))

                for (index, entity) in displayEntities.enumerated() {
                    entity.model?.materials = [index % 2 == 0 ? materialA : materialB]
                }
            }
            catch {
                rbError("Portrait pair texture: \(error.localizedDescription)")
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
