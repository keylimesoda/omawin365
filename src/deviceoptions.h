#pragma once

// What one Cloud PC may use on this computer. Saved per Cloud PC in profiles.json,
// never in the connection file. Audio and microphone are opt-in; clipboard is opt-out.
struct DeviceOptions {
    bool audioPlayback = false;
    bool audioCapture = false;
    bool clipboard = true;
    bool operator==(const DeviceOptions&) const = default;
};
