import SwiftUI

struct PlacementPanel: View {
    @State private var displayManager = DisplayManager.shared

    var body: some View {
        Button(action: {
            displayManager.placeDisplay()
        }) {
            Label("PLACE DISPLAY", systemImage: "display")
                .font(.system(size: 16, weight: .semibold))
        }
        .buttonStyle(.borderedProminent)
        .padding()
    }
}
