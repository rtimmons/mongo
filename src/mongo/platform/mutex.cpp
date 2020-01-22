/**
 *    Copyright (C) 2018-present MongoDB, Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the Server Side Public License, version 1,
 *    as published by MongoDB, Inc.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    Server Side Public License for more details.
 *
 *    You should have received a copy of the Server Side Public License
 *    along with this program. If not, see
 *    <http://www.mongodb.com/licensing/server-side-public-license>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the Server Side Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */

#include "mongo/platform/mutex.h"

#include "mongo/base/init.h"

namespace mongo {

Mutex::Mutex(std::shared_ptr<latch_detail::Data> data) : _data{std::move(data)} {
    invariant(_data);

    _data->counts().created.fetchAndAdd(1);
}

Mutex::~Mutex() {
    invariant(!_isLocked);

    _data->counts().destroyed.fetchAndAdd(1);
}

void Mutex::lock() {
    if (_mutex.try_lock()) {
        _isLocked = true;
        _onQuickLock();
        return;
    }

    _onContendedLock();
    _mutex.lock();
    _isLocked = true;
    _onSlowLock();
}

void Mutex::unlock() {
    _onUnlock();
    _isLocked = false;
    _mutex.unlock();
}
bool Mutex::try_lock() {
    if (!_mutex.try_lock()) {
        return false;
    }

    _isLocked = true;
    _onQuickLock();
    return true;
}

StringData Mutex::getName() const {
    return StringData(_data->identity().name());
}

void Mutex::addLockListener(LockListener* listener) {
    auto& state = _getListenerState();

    invariant(!state.isFinalized.load());
    state.listeners.push_back(listener);
}

void Mutex::finalizeLockListeners() {
    auto& state = _getListenerState();
    state.isFinalized.store(true);
}

void Mutex::_onContendedLock() noexcept {
    _data->counts().contended.fetchAndAdd(1);

    auto& state = _getListenerState();
    if (!state.isFinalized.load()) {
        return;
    }

    for (auto listener : state.listeners) {
        listener->onContendedLock(_data->identity());
    }
}

void Mutex::_onQuickLock() noexcept {
    _data->counts().acquired.fetchAndAdd(1);

    auto& state = _getListenerState();
    if (!state.isFinalized.load()) {
        return;
    }

    for (auto listener : state.listeners) {
        listener->onQuickLock(_data->identity());
    }
}

void Mutex::_onSlowLock() noexcept {
    _data->counts().acquired.fetchAndAdd(1);

    auto& state = _getListenerState();
    if (!state.isFinalized.load()) {
        return;
    }

    for (auto listener : state.listeners) {
        listener->onSlowLock(_data->identity());
    }
}

void Mutex::_onUnlock() noexcept {
    _data->counts().released.fetchAndAdd(1);

    auto& state = _getListenerState();
    if (!state.isFinalized.load()) {
        return;
    }

    for (auto listener : state.listeners) {
        listener->onUnlock(_data->identity());
    }
}

/**
 * Any MONGO_INITIALIZER that adds a LockListener will want to list FinalizeLockListeners as
 * a dependent initializer. This means that all LockListeners are certified to be added before main
 * and no LockListeners are ever invoked before main.
 */
MONGO_INITIALIZER(FinalizeLockListeners)(InitializerContext* context) {
    Mutex::finalizeLockListeners();

    return Status::OK();
}

}  // namespace mongo
