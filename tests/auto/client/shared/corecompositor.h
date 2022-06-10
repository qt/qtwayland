// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MOCKCOMPOSITOR_CORECOMPOSITOR_H
#define MOCKCOMPOSITOR_CORECOMPOSITOR_H

#include <QtTest/QtTest>

#include <wayland-server-core.h>

struct wl_resource;

namespace MockCompositor {

class Global : public QObject
{
    Q_OBJECT
public:
    virtual bool isClean() { return true; }
    virtual QString dirtyMessage() { return isClean() ? "clean" : "dirty"; }
};

class CoreCompositor
{
public:
    enum CompositorType {
        Default,
        Legacy // wl-shell
    };

    CompositorType m_type = Default;
    explicit CoreCompositor(CompositorType t = Default);
    ~CoreCompositor();
    bool isClean();
    QString dirtyMessage();
    void dispatch(int timeout = 0);

    template<typename function_type, typename... arg_types>
    auto exec(function_type func, arg_types&&... args) -> decltype(func())
    {
        Lock lock(this);
        return func(std::forward<arg_types>(args)...);
    }

    template<typename function_type, typename... arg_types>
    auto call(function_type func, arg_types&&... args) -> decltype(func())
    {
        Lock lock(this);
        auto boundFunc = std::bind(func, this);
        return boundFunc(this, std::forward<arg_types>(args)...);
    }

    // Unsafe section below, YOU are responsible that the compositor is locked or
    // this is run through the mutex() method!

    void add(Global *global);
    void remove(Global *global);

    /*!
     * \brief Constructs and adds a new global with the given parameters
     *
     * Convenience function. i.e.
     *
     *     compositor->add(new MyGlobal(compositor, version);
     *
     * can be written as:
     *
     *     compositor->add<MyGlobal>(version);
     *
     * Returns the new global
     */
    template<typename global_type, typename... arg_types>
    global_type *add(arg_types&&... args)
    {
        warnIfNotLockedByThread(Q_FUNC_INFO);
        auto *global = new global_type(this, std::forward<arg_types>(args)...);
        m_globals.append(global);
        return global;
    }

    /*!
     * \brief Removes all globals of the given type
     *
     * Convenience function
     */
    template<typename global_type, typename... arg_types>
    void removeAll()
    {
        const auto globals = getAll<global_type>();
        for (auto global : globals)
            remove(global);
    }

    /*!
     * \brief Returns a global with the given type, if any
     */
    template<typename global_type>
    global_type *get()
    {
        warnIfNotLockedByThread(Q_FUNC_INFO);
        for (auto *global : qAsConst(m_globals)) {
            if (auto *casted = qobject_cast<global_type *>(global))
                return casted;
        }
        return nullptr;
    }

    /*!
     * \brief Returns the nth global with the given type, if any
     */
    template<typename global_type>
    global_type *get(int index)
    {
        warnIfNotLockedByThread(Q_FUNC_INFO);
        for (auto *global : qAsConst(m_globals)) {
            if (auto *casted = qobject_cast<global_type *>(global)) {
                if (index--)
                    continue;
                return casted;
            }
        }
        return nullptr;
    }

    /*!
     * \brief Returns all globals with the given type, if any
     */
    template<typename global_type>
    QList<global_type *> getAll()
    {
        warnIfNotLockedByThread(Q_FUNC_INFO);
        QList<global_type *> matching;
        for (auto *global : qAsConst(m_globals)) {
            if (auto *casted = qobject_cast<global_type *>(global))
                matching.append(casted);
        }
        return matching;
    }

    uint nextSerial();
    uint currentTimeMilliseconds();
    wl_client *client(int index = 0);
    void warnIfNotLockedByThread(const char* caller = "warnIfNotLockedbyThread");

public:
    // Only use this carefully from the test thread (i.e. lock first)
    wl_display *m_display = nullptr;

protected:
    class Lock {
    public:
        explicit Lock(CoreCompositor *compositor)
            : m_compositor(compositor)
            , m_threadId(std::this_thread::get_id())
        {
            // Can't use a QMutexLocker here, as it's not movable
            compositor->m_mutex.lock();
            Q_ASSERT(compositor->m_lock == nullptr);
            compositor->m_lock = this;
        }
        ~Lock()
        {
            Q_ASSERT(m_compositor->m_lock == this);
            m_compositor->m_lock = nullptr;
            m_compositor->m_mutex.unlock();
        }

        // Move semantics
        Lock(Lock &&) = default;
        Lock &operator=(Lock &&) = default;

        // Disable copying
        Lock(const Lock &) = delete;
        Lock &operator=(const Lock &) = delete;

        bool isOwnedByCurrentThread() const { return m_threadId == std::this_thread::get_id(); }
    private:
        CoreCompositor *m_compositor = nullptr;
        std::thread::id m_threadId;
    };
    QByteArray m_socketName;
    wl_event_loop *m_eventLoop = nullptr;
    bool m_running = true;
    QList<Global *> m_globals;
    QElapsedTimer m_timer;

private:
    Lock *m_lock = nullptr;
    QMutex m_mutex;
    std::thread m_dispatchThread;
};

template<typename container_type>
QByteArray toByteArray(container_type container)
{
    return QByteArray(reinterpret_cast<const char *>(container.data()), sizeof (container[0]) * container.size());
}

template<typename return_type>
return_type *fromResource(::wl_resource *resource) {
    if (auto *r = return_type::Resource::fromResource(resource))
        return static_cast<return_type *>(r->object());
    return nullptr;
}

} // namespace MockCompositor

#endif // MOCKCOMPOSITOR_CORECOMPOSITOR_H
