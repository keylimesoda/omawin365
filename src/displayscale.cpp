#include "displayscale.h"

#include <QDir>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>
#include <QRegularExpression>
#include <QScreen>
#include <QWidget>
#include <algorithm>
#include <cmath>

namespace {
constexpr qint64 maxReply = 256 * 1024;
constexpr int queryBudgetMs = 300;

// Bounded and synchronous: Hyprland answers immediately, and a slow or absent
// compositor only costs Auto its first choice.
std::optional<double> hyprlandFocusedScale()
{
    const QString signature = qEnvironmentVariable("HYPRLAND_INSTANCE_SIGNATURE");
    const QString runtime = qEnvironmentVariable("XDG_RUNTIME_DIR");
    static const QRegularExpression safe(QStringLiteral("^[A-Za-z0-9_]{1,128}$"));
    if (!safe.match(signature).hasMatch() || !QDir::isAbsolutePath(runtime))
        return std::nullopt;
    QLocalSocket socket;
    socket.setReadBufferSize(maxReply + 1);
    socket.connectToServer(runtime + QStringLiteral("/hypr/") + signature + QStringLiteral("/.socket.sock"));
    QElapsedTimer elapsed;
    elapsed.start();
    if (!socket.waitForConnected(queryBudgetMs))
        return std::nullopt;
    socket.write("j/monitors");
    QByteArray reply;
    while (elapsed.elapsed() < queryBudgetMs && reply.size() <= maxReply) {
        const int remaining = int(queryBudgetMs - elapsed.elapsed());
        if (!socket.waitForReadyRead(std::max(1, remaining))) break;
        reply += socket.read(maxReply + 1 - reply.size());
    }
    reply += socket.read(maxReply + 1 - reply.size());
    if (reply.size() > maxReply)
        return std::nullopt;
    return DisplayScale::focusedScaleFromJson(reply);
}
}

const QList<int>& DisplayScale::presets()
{
    static const QList<int> values{100, 125, 150, 175, 200, 250, 300};
    return values;
}

int DisplayScale::fromFactor(double factor)
{
    if (!std::isfinite(factor) || factor <= 0)
        return 100;
    const double steps = std::floor(factor * 4 + 0.5);
    return int(std::clamp(steps * 25, 100.0, 500.0));
}

int DisplayScale::normalizeChoice(const QVariant& stored)
{
    bool ok = false;
    const int value = stored.toInt(&ok);
    return ok && presets().contains(value) ? value : Auto;
}

std::optional<double> DisplayScale::focusedScaleFromJson(const QByteArray& json)
{
    QJsonParseError error{};
    const auto document = QJsonDocument::fromJson(json, &error);
    if (error.error != QJsonParseError::NoError || !document.isArray())
        return std::nullopt;
    std::optional<double> found;
    for (const auto& entry : document.array()) {
        const auto monitor = entry.toObject();
        if (monitor.value(QStringLiteral("focused")).toBool() != true)
            continue;
        if (found)
            return std::nullopt; // Ambiguous: more than one focused monitor.
        const auto scale = monitor.value(QStringLiteral("scale"));
        if (!scale.isDouble() || !std::isfinite(scale.toDouble()) || scale.toDouble() <= 0)
            return std::nullopt;
        found = scale.toDouble();
    }
    return found;
}

int DisplayScale::resolveAuto(const QWidget* reference)
{
    if (const auto scale = hyprlandFocusedScale())
        return fromFactor(*scale);
    if (reference && reference->screen())
        return fromFactor(reference->screen()->devicePixelRatio());
    return 100;
}
