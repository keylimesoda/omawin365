#pragma once

// Local opt-ins, saved separately from the original connection file.
struct AudioOptions {
    bool playback = false;
    bool capture = false;
    bool operator==(const AudioOptions&) const = default;
};
