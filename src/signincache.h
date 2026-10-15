#pragma once

#include <QObject>
#include <QByteArray>
#include <QJsonArray>
#include <QQueue>
#include <QThread>
#include <QTimer>
#include <memory>

namespace SignInCookies {
// Only persistent, secure cookies for the exact Entra sign-in host; no OAuth tokens,
// session-only cookies, portal storage, arbitrary IdP state or browser profiles.
QJsonArray persistent(const QJsonArray& cookies, double now);
QByteArray encode(const QJsonArray& cookies, double now);
QJsonArray decode(const QByteArray& data, double now);
}

// Secret Service is the only storage backend. Jobs run serially off the UI thread;
// forgetting drops queued saves and follows any in-flight save with deletion.
class SignInCache final : public QObject {
    Q_OBJECT
public:
    explicit SignInCache(QObject* parent = nullptr);
    ~SignInCache() override;
    void load();
    void save(const QByteArray& data);
    void clear();
signals:
    void loaded(const QByteArray& data, bool ok);
    void saved(bool ok, bool hasPersistentCookies);
    void cleared(bool ok);
private:
    enum class Kind { Load, Save, Clear };
    struct Job { Kind kind; QByteArray data; };
    struct Result;
    void enqueue(Job job);
    void next();
    QQueue<Job> jobs;
    QThread* thread = nullptr;
    QTimer timeout;
    std::shared_ptr<Result> current;
};
