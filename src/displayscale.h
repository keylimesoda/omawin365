#pragma once

#include <QByteArray>
#include <QList>
#include <QVariant>
#include <optional>

class QWidget;

// Chooses the Windows desktop scale sent to FreeRDP (/scale-desktop). Read-only:
// it asks the compositor for the focused monitor's scale and never changes it.
namespace DisplayScale {
constexpr int Auto = 0;
// Percentages offered in the launcher; Auto resolves to any step of 25 in 100..500.
const QList<int>& presets();
// Nearest 25%, ties upward, bounded to 100..500. Invalid factors give 100.
int fromFactor(double factor);
// A stored choice: Auto or one of presets(). Anything else is Auto.
int normalizeChoice(const QVariant& stored);
// The focused monitor's scale from Hyprland's `j/monitors` reply.
std::optional<double> focusedScaleFromJson(const QByteArray& json);
// Hyprland first (where new windows open), then the reference widget's screen, else 100.
int resolveAuto(const QWidget* reference);
}
