import Foundation

class KeyboardHandler {
    private static let KEY_INTERVAL = 1.0/10.0

    private var _text = ""
    private var _count = 0

    private func _sendText() {
        Timer.scheduledTimer(withTimeInterval: KeyboardHandler.KEY_INTERVAL, repeats: true) { timer in
            let char = self._text[self._text.index(self._text.startIndex, offsetBy: self._count)]
            if let ascii = char.asciiValue {
                EmulatorKeyPress(Int32(ascii));
            }
            
            self._count += 1
            
            if self._count >= self._text.count {
                timer.invalidate()
            }
        }
    }
    
    func sendText(_ text: String, wait: TimeInterval = 0.0) {
        if text.count == 0 {
            return
        }
        
        _text = text
        _count = 0

        if wait > 0 {
            Timer.scheduledTimer(withTimeInterval: wait, repeats: false) { timer in
                self._sendText()
            }
        }
        else {
            _sendText()
        }
    }
}
