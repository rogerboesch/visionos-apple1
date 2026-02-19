import SwiftUI

struct PlacementPanel: View {
    @State private var displayManager = DisplayManager.shared

    var body: some View {
        HStack(spacing: 12) {
            Button(action: {
                displayManager.placeDisplay()
            }) {
                Label("PLACE DISPLAY", systemImage: "display")
                    .font(.system(size: 16, weight: .semibold))
            }
            .buttonStyle(.borderedProminent)
            .disabled(!displayManager.canPlaceMore)

            Text("\(displayManager.displayCount)/\(DISPLAY_MAX_COUNT)")
                .font(.system(size: 14, design: .monospaced))
                .foregroundColor(.secondary)
        }
        .padding()
    }
}
