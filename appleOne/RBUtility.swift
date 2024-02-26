//
//  RBUtility.swift
//  Some useful utility functions
//
//  Created by Roger Boesch on 01/01/2024.
//  Copyright © 2024 Roger Boesch. All rights reserved.
//

import Foundation

func isVisionOSDevice() -> Bool {
    if let _ = ProcessInfo().environment["SIMULATOR_MODEL_IDENTIFIER"] {
        return false
    }

    return true
}
