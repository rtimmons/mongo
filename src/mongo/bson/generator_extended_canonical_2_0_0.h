/**
 *    Copyright (C) 2019-present MongoDB, Inc.
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

#pragma once

#include "mongo/bson/bsonobj.h"
#include "mongo/platform/decimal128.h"
#include "mongo/util/base64.h"
#include "mongo/util/str_escape.h"

#include <fmt/compile.h>

namespace mongo {
class ExtendedCanonicalV200Generator {
public:
    void writeNull(fmt::memory_buffer& buffer) const {
        appendTo(buffer, "null"_sd);
    }
    void writeUndefined(fmt::memory_buffer& buffer) const {
        appendTo(buffer, R"({"$undefined":true})"_sd);
    }

    void writeString(fmt::memory_buffer& buffer, StringData str) const {
        buffer.push_back('"');
        str::escapeForJSON(buffer, str);
        buffer.push_back('"');
    }

    void writeBool(fmt::memory_buffer& buffer, bool val) const {
        if (val)
            appendTo(buffer, "true"_sd);
        else
            appendTo(buffer, "false"_sd);
    }

    void writeInt32(fmt::memory_buffer& buffer, int32_t val) const {
        static const auto fmt_str = fmt::compile<int32_t>(R"({{"$numberInt":"{}"}})");
        compiled_format_to(buffer, fmt_str, val);
    }

    void writeInt64(fmt::memory_buffer& buffer, int64_t val) const {
        static const auto fmt_str = fmt::compile<int64_t>(R"({{"$numberLong":"{}"}})");
        compiled_format_to(buffer, fmt_str, val);
    }

    void writeDouble(fmt::memory_buffer& buffer, double val) const {
        static const auto fmt_str = fmt::compile<double>(R"({{"$numberDouble":"{}"}})");
        if (val >= std::numeric_limits<double>::lowest() &&
            val <= std::numeric_limits<double>::max())
            compiled_format_to(buffer, fmt_str, val);
        else if (std::isnan(val))
            appendTo(buffer, R"({"$numberDouble":"NaN"})"_sd);
        else if (std::isinf(val)) {
            if (val > 0)
                appendTo(buffer, R"({"$numberDouble":"Infinity"})"_sd);
            else
                appendTo(buffer, R"({"$numberDouble":"-Infinity"})"_sd);
        } else {
            StringBuilder ss;
            ss << "Number " << val << " cannot be represented in JSON";
            uassert(51757, ss.str(), false);
        }
    }

    void writeDecimal128(fmt::memory_buffer& buffer, Decimal128 val) const {
        static const auto fmt_str_infinite =
            fmt::compile<StringData>(R"({{"$numberDecimal":"{}"}})");
        static const auto fmt_str_decimal =
            fmt::compile<std::string>(R"({{"$numberDecimal":"{}"}})");
        if (val.isNaN())
            appendTo(buffer, R"({"$numberDecimal":"NaN"})"_sd);
        else if (val.isInfinite())
            compiled_format_to(
                buffer, fmt_str_infinite, val.isNegative() ? "-Infinity"_sd : "Infinity"_sd);
        else {
            compiled_format_to(buffer, fmt_str_decimal, val.toString());
        }
    }

    void writeDate(fmt::memory_buffer& buffer, Date_t val) const {
        static const auto fmt_str =
            fmt::compile<long long>(R"({{"$date":{{"$numberLong":"{}"}}}})");
        compiled_format_to(buffer, fmt_str, val.toMillisSinceEpoch());
    }

    void writeDBRef(fmt::memory_buffer& buffer, StringData ref, OID id) const {
        // Collection names can unfortunately contain control characters that need to be escaped
        appendTo(buffer, R"({"$ref":")"_sd);
        str::escapeForJSON(buffer, ref);

        // OID is a hex string and does not need to be escaped
        static const auto fmt_str = fmt::compile<std::string>(R"(","$id":"{}"}})");
        compiled_format_to(buffer, fmt_str, id.toString());
    }

    void writeOID(fmt::memory_buffer& buffer, OID val) const {
        // OID is a hex string and does not need to be escaped
        static const auto fmt_str = fmt::compile<uint8_t,
                                                 uint8_t,
                                                 uint8_t,
                                                 uint8_t,
                                                 uint8_t,
                                                 uint8_t,
                                                 uint8_t,
                                                 uint8_t,
                                                 uint8_t,
                                                 uint8_t,
                                                 uint8_t,
                                                 uint8_t>(
            R"({{"$oid":"{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}"}})");
        static_assert(OID::kOIDSize == 12);
        const uint8_t* data = reinterpret_cast<const uint8_t*>(val.view().view());
        compiled_format_to(buffer,
                           fmt_str,
                           data[0],
                           data[1],
                           data[2],
                           data[3],
                           data[4],
                           data[5],
                           data[6],
                           data[7],
                           data[8],
                           data[9],
                           data[10],
                           data[11]);
    }

    void writeTimestamp(fmt::memory_buffer& buffer, Timestamp val) const {
        static const auto fmt_str =
            fmt::compile<unsigned int, unsigned int>(R"({{"$timestamp":{{"t":{},"i":{}}}}})");
        compiled_format_to(buffer, fmt_str, val.getSecs(), val.getInc());
    }

    void writeBinData(fmt::memory_buffer& buffer, StringData data, BinDataType type) const {
        static const auto fmt_str_uuid = fmt::compile<uint8_t,
                                                      uint8_t,
                                                      uint8_t,
                                                      uint8_t,
                                                      uint8_t,
                                                      uint8_t,
                                                      uint8_t,
                                                      uint8_t,
                                                      uint8_t,
                                                      uint8_t,
                                                      uint8_t,
                                                      uint8_t,
                                                      uint8_t,
                                                      uint8_t,
                                                      uint8_t,
                                                      uint8_t>(
            R"({{"$uuid":"{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}"}})");
        static const auto fmt_str_subtype = fmt::compile<BinDataType>(R"(","subType":"{:x}"}}}})");
        if (type == newUUID && data.size() == 16) {
            compiled_format_to(buffer,
                               fmt_str_uuid,
                               static_cast<uint8_t>(data[0]),
                               static_cast<uint8_t>(data[1]),
                               static_cast<uint8_t>(data[2]),
                               static_cast<uint8_t>(data[3]),
                               static_cast<uint8_t>(data[4]),
                               static_cast<uint8_t>(data[5]),
                               static_cast<uint8_t>(data[6]),
                               static_cast<uint8_t>(data[7]),
                               static_cast<uint8_t>(data[8]),
                               static_cast<uint8_t>(data[9]),
                               static_cast<uint8_t>(data[10]),
                               static_cast<uint8_t>(data[11]),
                               static_cast<uint8_t>(data[12]),
                               static_cast<uint8_t>(data[13]),
                               static_cast<uint8_t>(data[14]),
                               static_cast<uint8_t>(data[15]));
        } else {
            appendTo(buffer, R"({"$binary":{"base64":")"_sd);
            base64::encode(buffer, data);
            compiled_format_to(buffer, fmt_str_subtype, type);
        }
    }

    void writeRegex(fmt::memory_buffer& buffer, StringData pattern, StringData options) const {
        appendTo(buffer, R"({"$regularExpression":{"pattern":")"_sd);
        str::escapeForJSON(buffer, pattern);
        appendTo(buffer, R"(","options":")"_sd);
        str::escapeForJSON(buffer, options);
        appendTo(buffer, R"("}})"_sd);
    }

    void writeSymbol(fmt::memory_buffer& buffer, StringData symbol) const {
        appendTo(buffer, R"({"$symbol":")"_sd);
        str::escapeForJSON(buffer, symbol);
        appendTo(buffer, R"("})"_sd);
    }

    void writeCode(fmt::memory_buffer& buffer, StringData code) const {
        appendTo(buffer, R"({"$code":")"_sd);
        str::escapeForJSON(buffer, code);
        appendTo(buffer, R"("})"_sd);
    }
    void writeCodeWithScope(fmt::memory_buffer& buffer,
                            StringData code,
                            BSONObj const& scope) const {
        appendTo(buffer, R"({"$code":")"_sd);
        str::escapeForJSON(buffer, code);
        appendTo(buffer, R"(","$scope":)"_sd);
        scope.jsonStringGenerator(*this, 0, false, buffer);
        appendTo(buffer, R"(})"_sd);
    }
    void writeMinKey(fmt::memory_buffer& buffer) const {
        appendTo(buffer, R"({"$minKey":1})"_sd);
    }
    void writeMaxKey(fmt::memory_buffer& buffer) const {
        appendTo(buffer, R"({"$maxKey":1})"_sd);
    }
    void writePadding(fmt::memory_buffer& buffer) const {}

protected:
    static void appendTo(fmt::memory_buffer& buffer, StringData data) {
        buffer.append(data.begin(), data.end());
    }

    static void appendTo(fmt::memory_buffer& buffer, const fmt::format_int& data) {
        buffer.append(data.data(), data.data() + data.size());
    }

    template <typename CompiledFormatStr, typename... Args>
    static void compiled_format_to(fmt::memory_buffer& buffer,
                                   const CompiledFormatStr& fmt_str,
                                   const Args&... args) {
        fmt::internal::cf::vformat_to<fmt::buffer_context<char>>(
            fmt::buffer_range(buffer),
            fmt_str,
            {fmt::make_format_args<fmt::buffer_context<char>>(args...)});
    }
};
}  // namespace mongo
