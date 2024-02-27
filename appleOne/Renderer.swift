import Foundation
import UIKit
import RealityKit

var texture_count = 0

@objc
@MainActor
class Renderer : NSObject {
    public static var entity: ModelEntity?
    
    @objc
    public static func render(_ image: UIImage) {
        Task.init {
            do {
                texture_count += 1
                
                var material = SimpleMaterial(color: UIColor.red, roughness: 0, isMetallic: false)
                let resource = try await TextureResource.generate(from: image.cgImage!, named: "asset-\(texture_count)", options: TextureResource.CreateOptions.init(semantic: .raw))
                material.color = .init(tint: .white, texture: .init(resource))
                
                if Renderer.entity != nil {
                    Renderer.entity?.model?.materials = [material];
                }
            }
            catch {
                rbError(error.localizedDescription)
            }
        }
    }
    
    public static func render2(_ image: UIImage) {
        Task.init {
            do {
                var material = SimpleMaterial(color: UIColor.red, roughness: 0, isMetallic: false)
                let resource = try await TextureResource.generate(from: image.cgImage!, options: TextureResource.CreateOptions.init(semantic: .raw))
                material.color = .init(tint: .white, texture: .init(resource))
                
                if Renderer.entity != nil {
                    Renderer.entity?.model?.materials = [material];
                }
            }
            catch {
                rbError(error.localizedDescription)
            }
        }
    }
}
