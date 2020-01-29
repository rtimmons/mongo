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
#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kReplication

#include "mongo/platform/basic.h"

#include "mongo/db/repl/repl_set_heartbeat_args_v1.h"

#include "mongo/bson/util/bson_check.h"
#include "mongo/bson/util/bson_extract.h"
#include "mongo/db/jsobj.h"
#include "mongo/util/net/hostandport.h"
#include "mongo/util/log.h"

namespace mongo {
namespace repl {

namespace {

const std::string kCheckEmptyFieldName = "checkEmpty";
const std::string kConfigVersionFieldName = "configVersion";
const std::string kHeartbeatVersionFieldName = "hbv";
const std::string kSenderHostFieldName = "from";
const std::string kSenderIdFieldName = "fromId";
const std::string kSetNameFieldName = "replSetHeartbeat";
const std::string kTermFieldName = "term";
const std::string kPrimaryIdFieldName = "primaryId";

const std::string kLegalHeartbeatFieldNames[] = {kCheckEmptyFieldName,
                                                 kConfigVersionFieldName,
                                                 kHeartbeatVersionFieldName,
                                                 kSenderHostFieldName,
                                                 kSenderIdFieldName,
                                                 kSetNameFieldName,
                                                 kTermFieldName,
                                                 kPrimaryIdFieldName};

}  // namespace

Status ReplSetHeartbeatArgsV1::initialize(const BSONObj& argsObj) {
    Status status = bsonCheckOnlyHasFieldsForCommand(
        "ReplSetHeartbeatArgs", argsObj, kLegalHeartbeatFieldNames);
    if (!status.isOK())
        return status;

    status = bsonExtractBooleanFieldWithDefault(argsObj, kCheckEmptyFieldName, false, &_checkEmpty);
    if (!status.isOK())
        return status;

    status = bsonExtractIntegerField(argsObj, kConfigVersionFieldName, &_configVersion);
    if (!status.isOK())
        return status;

    long long tempHeartbeatVersion;
    status = bsonExtractIntegerField(argsObj, kHeartbeatVersionFieldName, &tempHeartbeatVersion);
    if (status.isOK()) {
        if (tempHeartbeatVersion != 1) {
            return Status(ErrorCodes::Error(40666),
                          str::stream()
                              << "Found invalid value for field " << kHeartbeatVersionFieldName
                              << ": " << tempHeartbeatVersion);
        }
        _heartbeatVersion = tempHeartbeatVersion;
        _hasHeartbeatVersion = true;
    } else if (status.code() != ErrorCodes::NoSuchKey) {
        return status;
    }

    // If sender is version < 4.4, argsObj won't have the primaryId field,
    // but we still parse and allow it whenever it is present.
    status = bsonExtractIntegerFieldWithDefault(argsObj, kSenderIdFieldName, -1, &_senderId);
    if (!status.isOK())
        return status;

    std::string hostAndPortString;
    status = bsonExtractStringField(argsObj, kSenderHostFieldName, &hostAndPortString);
    if (!status.isOK())
        return status;
    if (!hostAndPortString.empty()) {
        status = _senderHost.initialize(hostAndPortString);
        if (!status.isOK())
            return status;
        _hasSender = true;
    }

    status = bsonExtractIntegerFieldWithDefault(argsObj, kPrimaryIdFieldName, -1, &_primaryId);
    if (!status.isOK())
        return status;

    status = bsonExtractIntegerField(argsObj, kTermFieldName, &_term);
    if (!status.isOK())
        return status;

    status = bsonExtractStringField(argsObj, kSetNameFieldName, &_setName);
    if (!status.isOK())
        return status;

    return Status::OK();
}

bool ReplSetHeartbeatArgsV1::isInitialized() const {
    return _configVersion != -1 && _term != -1 && !_setName.empty();
}

void ReplSetHeartbeatArgsV1::setConfigVersion(long long newVal) {
    _configVersion = newVal;
}

void ReplSetHeartbeatArgsV1::setHeartbeatVersion(long long newVal) {
    _heartbeatVersion = newVal;
    _hasHeartbeatVersion = true;
}

void ReplSetHeartbeatArgsV1::setSenderHost(const HostAndPort& newVal) {
    _senderHost = newVal;
    _hasSender = true;
}

void ReplSetHeartbeatArgsV1::setSenderId(long long newVal) {
    _senderId = newVal;
}

void ReplSetHeartbeatArgsV1::setSetName(const std::string& newVal) {
    _setName = newVal;
}

void ReplSetHeartbeatArgsV1::setTerm(long long newVal) {
    _term = newVal;
}

void ReplSetHeartbeatArgsV1::setCheckEmpty() {
    _checkEmpty = true;
}

void ReplSetHeartbeatArgsV1::setPrimaryId(long long primaryId) {
    _primaryId = primaryId;
}


BSONObj ReplSetHeartbeatArgsV1::toBSON() const {
    invariant(isInitialized());
    BSONObjBuilder builder;
    addToBSON(&builder);
    return builder.obj();
}

void ReplSetHeartbeatArgsV1::addToBSON(BSONObjBuilder* builder) const {
    builder->append(kSetNameFieldName, _setName);
    if (_checkEmpty) {
        builder->append(kCheckEmptyFieldName, _checkEmpty);
    }
    builder->appendIntOrLL(kConfigVersionFieldName, _configVersion);
    if (_hasHeartbeatVersion) {
        builder->appendIntOrLL(kHeartbeatVersionFieldName, _hasHeartbeatVersion);
    }
    builder->append(kSenderHostFieldName, _hasSender ? _senderHost.toString() : "");
    builder->appendIntOrLL(kSenderIdFieldName, _senderId);
    builder->appendIntOrLL(kTermFieldName, _term);

    severe() << "RRRR Checking compat version to include _primaryId " << _primaryId << "(" <<
                                                                                   static_cast<int>(serverGlobalParams.featureCompatibility.getVersion()) << ")";

    // Can't send _primaryId field unless we're all on latest version since
    // parsing code above has bsonCheckOnlyHasFieldsForCommand (and all < 4.4 branches have this)
    if (serverGlobalParams.featureCompatibility.getVersion() ==
         ServerGlobalParams::FeatureCompatibility::Version::kFullyUpgradedTo44) {
        severe() << "RRR Included _primaryId";
        builder->append(kPrimaryIdFieldName, _primaryId);
    }
}

}  // namespace repl
}  // namespace mongo
