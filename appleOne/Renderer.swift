import Foundation
import UIKit
import SwiftUI

@Observable
@MainActor
class ScreenRenderer {
    static let shared = ScreenRenderer()

    var screenImage: UIImage? = nil

    // Single portrait (Jobs or Woz on all circle displays)
    var portraitImage: UIImage? = nil
    var portraitSingleCount: Int = 0

    // Alternating portrait pair (Jobs on even displays, Wozniak on odd)
    var portraitImageA: UIImage? = nil
    var portraitImageB: UIImage? = nil
    var portraitPairCount: Int = 0
}

@objc
@MainActor
class Renderer : NSObject {
    @objc
    public static func render(_ image: UIImage) {
        ScreenRenderer.shared.screenImage = image
    }

    @objc
    public static func renderPortrait(_ image: UIImage) {
        let renderer = ScreenRenderer.shared
        renderer.portraitImage = image
        renderer.portraitSingleCount += 1
    }

    @objc
    public static func renderPortraitPair(_ imageA: UIImage, imageB: UIImage) {
        let renderer = ScreenRenderer.shared
        renderer.portraitImageA = imageA
        renderer.portraitImageB = imageB
        renderer.portraitPairCount += 1
    }
}
