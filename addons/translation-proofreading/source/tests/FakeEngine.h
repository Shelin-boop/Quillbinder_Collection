#pragma once
// A stand-in for the engine in the checks (no models needed): it answers from a function the check sets and records
// every call, so that a check can see which model was asked and with what.
#include "EngineApi.h"

#include <QHash>
#include <QMutex>
#include <QMutexLocker>

#include <functional>

class FakeEngine : public tp::EngineInterface
{
public:
    struct Call {
        QString what;       // "run" or "variants"
        QString slot, model;
        QStringList texts;
    };
    std::function<QString(const QString &model, const QString &text)> answer;
    QStringList variantList;

    QList<Call> calls() const
    {
        QMutexLocker l(&m_mutex);
        return m_calls;
    }
    void clearCalls()
    {
        QMutexLocker l(&m_mutex);
        m_calls.clear();
    }

    bool load(const QString &slot, const tp::ModelFiles &model, int, QString *) override
    {
        QMutexLocker l(&m_mutex);
        m_loaded[slot] = model.id;
        return true;
    }
    void unload(const QString &slot) override
    {
        QMutexLocker l(&m_mutex);
        m_loaded.remove(slot);
    }
    QString loaded(const QString &slot) const override
    {
        QMutexLocker l(&m_mutex);
        return m_loaded.value(slot);
    }
    QStringList run(const QString &slot, const QStringList &texts, const tp::RunOptions &,
                    const std::function<bool(int, int)> &, QString *) override
    {
        QMutexLocker l(&m_mutex);
        m_calls << Call{QStringLiteral("run"), slot, m_loaded.value(slot), texts};
        QStringList out;
        for (const QString &t : texts)
            out << (answer ? answer(m_loaded.value(slot), t) : t);
        return out;
    }
    QList<tp::Alternative> alternatives(const QString &, const QString &, const QString &, int, const tp::RunOptions &,
                                        int, QString *) override
    {
        return {};
    }
    QStringList variants(const QString &slot, const QString &text, const tp::RunOptions &, int, QString *) override
    {
        QMutexLocker l(&m_mutex);
        m_calls << Call{QStringLiteral("variants"), slot, m_loaded.value(slot), {text}};
        return variantList;
    }
    QString version() const override { return QStringLiteral("stand-in"); }

private:
    mutable QMutex m_mutex;
    QHash<QString, QString> m_loaded;
    QList<Call> m_calls;
};
