import Foundation
import UIKit
import SwiftUI

@Observable
@MainActor
class ScreenRenderer {
    static let shared = ScreenRenderer()

    var screenImage: UIImage? = nil
}

@objc
@MainActor
class Renderer : NSObject {
    @objc
    public static func render(_ image: UIImage) {
        ScreenRenderer.shared.screenImage = image
    }
}
