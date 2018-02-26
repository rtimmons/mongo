/**
 * Copyright (C) 2017 MongoDB Inc.
 *
 * This program is free software: you can redistribute it and/or  modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, the copyright holders give permission to link the
 * code of portions of this program with the OpenSSL library under certain
 * conditions as described in each individual source file and distribute
 * linked combinations including the program with the OpenSSL library. You
 * must comply with the GNU Affero General Public License in all respects
 * for all of the code used other than as permitted herein. If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so. If you do not
 * wish to do so, delete this exception statement from your version. If you
 * delete this exception statement from all source files in the program,
 * then also delete it in the license file.
 */

#include "mongo/platform/basic.h"

#include "mongo/bson/oid.h"
#include "mongo/db/pipeline/aggregation_context_fixture.h"
#include "mongo/db/pipeline/document_value_test_util.h"
#include "mongo/db/pipeline/value_comparator.h"
#include "mongo/unittest/unittest.h"

namespace mongo {

namespace ExpressionConvertTest {

static const long long kIntMax = std::numeric_limits<int>::max();
static const long long kIntMin = std::numeric_limits<int>::lowest();
static const long long kLongMax = std::numeric_limits<long long>::max();
static const double kLongMin = static_cast<double>(std::numeric_limits<long long>::lowest());
static const double kLongNegativeOverflow =
    std::nextafter(static_cast<double>(kLongMin), std::numeric_limits<double>::lowest());
static const Decimal128 kDoubleOverflow = Decimal128("1e309");
static const Decimal128 kDoubleNegativeOverflow = Decimal128("-1e309");

using ExpressionConvertTest = AggregationContextFixture;

TEST_F(ExpressionConvertTest, ParseAndSerializeWithoutOptionalArguments) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    ASSERT_VALUE_EQ(Value(fromjson("{$convert: {input: '$path1', to: {$const: 'int'}}}")),
                    convertExp->serialize(false));

    ASSERT_VALUE_EQ(Value(fromjson("{$convert: {input: '$path1', to: {$const: 'int'}}}")),
                    convertExp->serialize(true));
}

TEST_F(ExpressionConvertTest, ParseAndSerializeWithOnError) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"
                                        << "onError"
                                        << 0));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    ASSERT_VALUE_EQ(
        Value(fromjson("{$convert: {input: '$path1', to: {$const: 'int'}, onError: {$const: 0}}}")),
        convertExp->serialize(false));

    ASSERT_VALUE_EQ(
        Value(fromjson("{$convert: {input: '$path1', to: {$const: 'int'}, onError: {$const: 0}}}")),
        convertExp->serialize(true));
}

TEST_F(ExpressionConvertTest, ParseAndSerializeWithOnNull) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"
                                        << "onNull"
                                        << 0));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    ASSERT_VALUE_EQ(
        Value(fromjson("{$convert: {input: '$path1', to: {$const: 'int'}, onNull: {$const: 0}}}")),
        convertExp->serialize(false));

    ASSERT_VALUE_EQ(
        Value(fromjson("{$convert: {input: '$path1', to: {$const: 'int'}, onNull: {$const: 0}}}")),
        convertExp->serialize(true));
}

TEST_F(ExpressionConvertTest, ConvertWithoutInputFailsToParse) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("to"
                                        << "int"
                                        << "onError"
                                        << 0));
    ASSERT_THROWS_WITH_CHECK(Expression::parseExpression(expCtx, spec, expCtx->variablesParseState),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::FailedToParse);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Missing 'input' parameter to $convert");
                             });
}

TEST_F(ExpressionConvertTest, ConvertWithoutToFailsToParse) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "onError"
                                        << 0));
    ASSERT_THROWS_WITH_CHECK(Expression::parseExpression(expCtx, spec, expCtx->variablesParseState),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::FailedToParse);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Missing 'to' parameter to $convert");
                             });
}

TEST_F(ExpressionConvertTest, InvalidTypeNameFails) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "dinosaur"
                                        << "onError"
                                        << 0));

    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(Document()),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::BadValue);
                                 ASSERT_STRING_CONTAINS(exception.reason(), "Unknown type name");
                             });
}

TEST_F(ExpressionConvertTest, NonIntegralTypeFails) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << 3.6
                                        << "onError"
                                        << 0));

    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(Document()),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::FailedToParse);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "In $convert, numeric 'to' argument is not an integer");
                             });
}

TEST_F(ExpressionConvertTest, NonStringNonNumericalTypeFails) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << BSON("dinosaur"
                                                << "Tyrannosaurus rex")
                                        << "onError"
                                        << 0));

    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(Document()),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::FailedToParse);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "$convert's 'to' argument must be a string or number");
                             });
}

TEST_F(ExpressionConvertTest, IllegalTargetTypeFails) {
    auto expCtx = getExpCtx();

    std::vector<std::string> illegalTargetTypes{"minKey",
                                                "object",
                                                "array",
                                                "binData",
                                                "undefined",
                                                "null",
                                                "regex",
                                                "dbPointer",
                                                "javascript",
                                                "symbol",
                                                "javascriptWithScope",
                                                "timestamp",
                                                "maxKey"};

    // Attempt a conversion with each illegal type.
    for (auto&& typeName : illegalTargetTypes) {
        auto spec = BSON("$convert" << BSON("input"
                                            << "$path1"
                                            << "to"
                                            << Value(typeName)
                                            << "onError"
                                            << 0));

        auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

        ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(Document()),
                                 AssertionException,
                                 [](const AssertionException& exception) {
                                     ASSERT_EQ(exception.code(), ErrorCodes::FailedToParse);
                                     ASSERT_STRING_CONTAINS(exception.reason(),
                                                            "$convert with unsupported 'to' type");
                                 });
    }
}

TEST_F(ExpressionConvertTest, InvalidNumericTargetTypeFails) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << 100
                                        << "onError"
                                        << 0));

    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate(Document()),
        AssertionException,
        [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::FailedToParse);
            ASSERT_STRING_CONTAINS(
                exception.reason(),
                "In $convert, numeric value for 'to' does not correspond to a BSON type");
        });
}

TEST_F(ExpressionConvertTest, NegativeNumericTargetTypeFails) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << -2
                                        << "onError"
                                        << 0));

    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate(Document()),
        AssertionException,
        [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::FailedToParse);
            ASSERT_STRING_CONTAINS(
                exception.reason(),
                "In $convert, numeric value for 'to' does not correspond to a BSON type");
        });
}

TEST_F(ExpressionConvertTest, UnsupportedConversionFails) {
    auto expCtx = getExpCtx();

    std::vector<std::pair<Value, std::string>> unsupportedConversions{
        {Value(OID()), "double"},
        {Value(OID()), "int"},
        {Value(OID()), "long"},
        {Value(OID()), "decimal"},
        {Value(Date_t{}), "objectId"},
        {Value(Date_t{}), "int"},
        {Value(int{1}), "date"},
        {Value(true), "date"},
    };

    // Attempt every possible unsupported conversion.
    for (auto conversion : unsupportedConversions) {
        auto inputValue = conversion.first;
        auto targetTypeName = conversion.second;

        auto spec = BSON("$convert" << BSON("input"
                                            << "$path1"
                                            << "to"
                                            << Value(targetTypeName)));

        Document intInput{{"path1", inputValue}};

        auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

        ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(intInput),
                                 AssertionException,
                                 [](const AssertionException& exception) {
                                     ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                     ASSERT_STRING_CONTAINS(exception.reason(),
                                                            "Unsupported conversion");
                                 });
    }
}

TEST_F(ExpressionConvertTest, ConvertNullishInput) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document nullInput{{"path1", Value(BSONNULL)}};
    Document undefinedInput{{"path1", Value(BSONUndefined)}};
    Document missingInput{{"path1", Value()}};

    ASSERT_VALUE_EQ(convertExp->evaluate(nullInput), Value(BSONNULL));
    ASSERT_VALUE_EQ(convertExp->evaluate(undefinedInput), Value(BSONNULL));
    ASSERT_VALUE_EQ(convertExp->evaluate(missingInput), Value(BSONNULL));
}

TEST_F(ExpressionConvertTest, ConvertNullishInputWithOnNull) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"
                                        << "onNull"
                                        << "B)"
                                        << "onError"
                                        << "Should not be used here"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document nullInput{{"path1", Value(BSONNULL)}};
    Document undefinedInput{{"path1", Value(BSONUndefined)}};
    Document missingInput{{"path1", Value()}};

    ASSERT_VALUE_EQ(convertExp->evaluate(nullInput), Value("B)"_sd));
    ASSERT_VALUE_EQ(convertExp->evaluate(undefinedInput), Value("B)"_sd));
    ASSERT_VALUE_EQ(convertExp->evaluate(missingInput), Value("B)"_sd));
}

TEST_F(ExpressionConvertTest, NullishToReturnsNull) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "inputString"
                                        << "to"
                                        << "$path1"
                                        << "onNull"
                                        << "Should not be used here"
                                        << "onError"
                                        << "Also should not be used"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document nullInput{{"path1", Value(BSONNULL)}};
    Document undefinedInput{{"path1", Value(BSONUndefined)}};
    Document missingInput{{"path1", Value()}};

    ASSERT_VALUE_EQ(convertExp->evaluate(nullInput), Value(BSONNULL));
    ASSERT_VALUE_EQ(convertExp->evaluate(undefinedInput), Value(BSONNULL));
    ASSERT_VALUE_EQ(convertExp->evaluate(missingInput), Value(BSONNULL));
}

#define ASSERT_VALUE_CONTENTS_AND_TYPE(v, contents, type)  \
    do {                                                   \
        Value evaluatedResult = v;                         \
        ASSERT_VALUE_EQ(evaluatedResult, Value(contents)); \
        ASSERT_EQ(evaluatedResult.getType(), type);        \
    } while (false);

TEST_F(ExpressionConvertTest, NullInputOverridesNullTo) {
    auto expCtx = getExpCtx();

    auto spec =
        BSON("$convert" << BSON("input" << Value(BSONNULL) << "to" << Value(BSONNULL) << "onNull"
                                        << "X"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(Document{}), "X"_sd, BSONType::String);
}

TEST_F(ExpressionConvertTest, ConvertOptimizesToExpressionConstant) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input" << 0 << "to"
                                                << "double"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    convertExp = convertExp->optimize();

    auto constResult = dynamic_cast<ExpressionConstant*>(convertExp.get());
    ASSERT(constResult);
    ASSERT_VALUE_CONTENTS_AND_TYPE(constResult->getValue(), 0.0, BSONType::NumberDouble);
}

TEST_F(ExpressionConvertTest, ConvertWithOnErrorOptimizesToExpressionConstant) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input" << 0 << "to"
                                                << "objectId"
                                                << "onError"
                                                << "X"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    convertExp = convertExp->optimize();

    auto constResult = dynamic_cast<ExpressionConstant*>(convertExp.get());
    ASSERT(constResult);
    ASSERT_VALUE_CONTENTS_AND_TYPE(constResult->getValue(), "X"_sd, BSONType::String);
}

TEST_F(ExpressionConvertTest, DoubleIdentityConversion) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "double"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document doubleInput{{"path1", Value(2.4)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(doubleInput), 2.4, BSONType::NumberDouble);

    Document doubleNaN{{"path1", std::numeric_limits<double>::quiet_NaN()}};
    auto result = convertExp->evaluate(doubleNaN);
    ASSERT(std::isnan(result.getDouble()));

    Document doubleInfinity{{"path1", std::numeric_limits<double>::infinity()}};
    result = convertExp->evaluate(doubleInfinity);
    ASSERT_EQ(result.getType(), BSONType::NumberDouble);
    ASSERT_GT(result.getDouble(), 0.0);
    ASSERT(std::isinf(result.getDouble()));

    Document doubleNegativeInfinity{{"path1", -std::numeric_limits<double>::infinity()}};
    result = convertExp->evaluate(doubleNegativeInfinity);
    ASSERT_EQ(result.getType(), BSONType::NumberDouble);
    ASSERT_LT(result.getDouble(), 0.0);
    ASSERT(std::isinf(result.getDouble()));
}

TEST_F(ExpressionConvertTest, BoolIdentityConversion) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "bool"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document trueBoolInput{{"path1", Value(true)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(trueBoolInput), true, BSONType::Bool);

    Document falseBoolInput{{"path1", Value(false)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(falseBoolInput), false, BSONType::Bool);
}

TEST_F(ExpressionConvertTest, DateIdentityConversion) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "date"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document dateInput{{"path1", Value(Date_t{})}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(dateInput), Date_t{}, BSONType::Date);
}

TEST_F(ExpressionConvertTest, IntIdentityConversion) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document intInput{{"path1", Value(int{123})}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(intInput), int{123}, BSONType::NumberInt);
}

TEST_F(ExpressionConvertTest, LongIdentityConversion) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "long"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document longInput{{"path1", Value(123LL)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(longInput), 123LL, BSONType::NumberLong);
}

TEST_F(ExpressionConvertTest, DecimalIdentityConversion) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "decimal"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document decimalInput{{"path1", Value(Decimal128("2.4"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(decimalInput), Decimal128("2.4"), BSONType::NumberDecimal);

    Document decimalNaN{{"path1", Decimal128::kPositiveNaN}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(decimalNaN), Decimal128::kPositiveNaN, BSONType::NumberDecimal);

    Document decimalInfinity{{"path1", Decimal128::kPositiveInfinity}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(decimalInfinity),
                                   Decimal128::kPositiveInfinity,
                                   BSONType::NumberDecimal);

    Document decimalNegativeInfinity{{"path1", Decimal128::kNegativeInfinity}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(decimalNegativeInfinity),
                                   Decimal128::kNegativeInfinity,
                                   BSONType::NumberDecimal);
}

TEST_F(ExpressionConvertTest, ConvertDateToBool) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "bool"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    // All date inputs evaluate as true.
    Document dateInput{{"path1", Value(Date_t{})}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(dateInput), true, BSONType::Bool);
}

TEST_F(ExpressionConvertTest, ConvertIntToBool) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "bool"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document trueIntInput{{"path1", Value(int{1})}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(trueIntInput), true, BSONType::Bool);

    Document falseIntInput{{"path1", Value(int{0})}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(falseIntInput), false, BSONType::Bool);
}

TEST_F(ExpressionConvertTest, ConvertLongToBool) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "bool"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document trueLongInput{{"path1", Value(-1ll)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(trueLongInput), true, BSONType::Bool);

    Document falseLongInput{{"path1", Value(0ll)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(falseLongInput), false, BSONType::Bool);
}

TEST_F(ExpressionConvertTest, ConvertDoubleToBool) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "bool"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document trueDoubleInput{{"path1", Value(2.4)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(trueDoubleInput), true, BSONType::Bool);

    Document falseDoubleInput{{"path1", Value(-0.0)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(falseDoubleInput), false, BSONType::Bool);

    Document doubleNaN{{"path1", std::numeric_limits<double>::quiet_NaN()}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(doubleNaN), true, BSONType::Bool);

    Document doubleInfinity{{"path1", std::numeric_limits<double>::infinity()}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(doubleInfinity), true, BSONType::Bool);

    Document doubleNegativeInfinity{{"path1", -std::numeric_limits<double>::infinity()}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(doubleNegativeInfinity), true, BSONType::Bool);
}

TEST_F(ExpressionConvertTest, ConvertDecimalToBool) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "bool"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document trueDecimalInput{{"path1", Value(Decimal128(5))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(trueDecimalInput), true, BSONType::Bool);

    Document falseDecimalInput{{"path1", Value(Decimal128(0))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(falseDecimalInput), false, BSONType::Bool);

    Document preciseZero{{"path1", Value(Decimal128("0.00"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(preciseZero), false, BSONType::Bool);

    Document negativeZero{{"path1", Value(Decimal128("-0.00"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(negativeZero), false, BSONType::Bool);

    Document decimalNaN{{"path1", Decimal128::kPositiveNaN}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(decimalNaN), true, BSONType::Bool);

    Document decimalNegativeNaN{{"path1", Decimal128::kNegativeNaN}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(decimalNegativeNaN), true, BSONType::Bool);

    Document decimalInfinity{{"path1", Decimal128::kPositiveInfinity}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(decimalInfinity), true, BSONType::Bool);

    Document decimalNegativeInfinity{{"path1", Decimal128::kNegativeInfinity}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(decimalNegativeInfinity), true, BSONType::Bool);
}

TEST_F(ExpressionConvertTest, ConvertNumericToDouble) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "double"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document intInput{{"path1", Value(int{1})}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(intInput), 1.0, BSONType::NumberDouble);

    Document longInput{{"path1", Value(0xf00000000ll)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(longInput), 64424509440.0, BSONType::NumberDouble);

    Document decimalInput{{"path1", Value(Decimal128("5.5"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(decimalInput), 5.5, BSONType::NumberDouble);

    Document boolFalse{{"path1", Value(false)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(boolFalse), 0.0, BSONType::NumberDouble);

    Document boolTrue{{"path1", Value(true)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(boolTrue), 1.0, BSONType::NumberDouble);

    Document decimalNaN{{"path1", Decimal128::kPositiveNaN}};
    auto result = convertExp->evaluate(decimalNaN);
    ASSERT_EQ(result.getType(), BSONType::NumberDouble);
    ASSERT(std::isnan(result.getDouble()));

    Document decimalNegativeNaN{{"path1", Decimal128::kNegativeNaN}};
    result = convertExp->evaluate(decimalNegativeNaN);
    ASSERT_EQ(result.getType(), BSONType::NumberDouble);
    ASSERT(std::isnan(result.getDouble()));

    Document decimalInfinity{{"path1", Decimal128::kPositiveInfinity}};
    result = convertExp->evaluate(decimalInfinity);
    ASSERT_EQ(result.getType(), BSONType::NumberDouble);
    ASSERT_GT(result.getDouble(), 0.0);
    ASSERT(std::isinf(result.getDouble()));

    Document decimalNegativeInfinity{{"path1", Decimal128::kNegativeInfinity}};
    result = convertExp->evaluate(decimalNegativeInfinity);
    ASSERT_EQ(result.getType(), BSONType::NumberDouble);
    ASSERT_LT(result.getDouble(), 0.0);
    ASSERT(std::isinf(result.getDouble()));

    // Note that the least significant bits get lost, because the significand of a double is not
    // wide enough for the original long long value in its entirety.
    Document largeLongInput{{"path1", Value(0xf0000000000000fLL)}};
    result = convertExp->evaluate(largeLongInput);
    ASSERT_EQ(static_cast<long long>(result.getDouble()), 0xf00000000000000LL);

    // Again, some precision is lost in the conversion from Decimal128 to double.
    Document preciseDecimalInput{{"path1", Value(Decimal128("1.125000000000000000005"))}};
    result = convertExp->evaluate(preciseDecimalInput);
    ASSERT_EQ(result.getDouble(), 1.125);
}

TEST_F(ExpressionConvertTest, ConvertDateToDouble) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "double"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document dateInput{{"path1", Value(Date_t::fromMillisSinceEpoch(123))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(dateInput), 123.0, BSONType::NumberDouble);

    // Note that the least significant bits get lost, because the significand of a double is not
    // wide enough for the original 64-bit Date_t value in its entirety.
    Document largeDateInput{{"path1", Value(Date_t::fromMillisSinceEpoch(0xf0000000000000fLL))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(largeDateInput), 0xf00000000000000LL, BSONType::NumberDouble);
}

TEST_F(ExpressionConvertTest, ConvertOutOfBoundsDecimalToDouble) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "double"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document overflowInput{{"path1", Decimal128("1e309")}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(overflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });

    Document negativeOverflowInput{{"path1", Decimal128("-1e309")}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(negativeOverflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });
}

TEST_F(ExpressionConvertTest, ConvertOutOfBoundsDecimalToDoubleWithOnError) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "double"
                                        << "onError"
                                        << "X"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document overflowInput{{"path1", Decimal128("1e309")}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(overflowInput), "X"_sd, BSONType::String);

    Document negativeOverflowInput{{"path1", Decimal128("-1e309")}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(negativeOverflowInput), "X"_sd, BSONType::String);
}

TEST_F(ExpressionConvertTest, ConvertNumericToDecimal) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "decimal"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document intInput{{"path1", Value(int{1})}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(intInput), Decimal128(1), BSONType::NumberDecimal);

    Document longInput{{"path1", Value(0xf00000000ll)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(longInput),
                                   Decimal128(std::int64_t{0xf00000000LL}),
                                   BSONType::NumberDecimal);

    Document doubleInput{{"path1", Value(0.1)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(doubleInput), Decimal128("0.1"), BSONType::NumberDecimal);

    Document boolFalse{{"path1", Value(false)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(boolFalse), Decimal128(0), BSONType::NumberDecimal);

    Document boolTrue{{"path1", Value(true)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(boolTrue), Decimal128(1), BSONType::NumberDecimal);

    Document doubleNaN{{"path1", std::numeric_limits<double>::quiet_NaN()}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(doubleNaN), Decimal128::kPositiveNaN, BSONType::NumberDecimal);

    Document doubleInfinity{{"path1", std::numeric_limits<double>::infinity()}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(doubleInfinity),
                                   Decimal128::kPositiveInfinity,
                                   BSONType::NumberDecimal);

    Document doubleNegativeInfinity{{"path1", -std::numeric_limits<double>::infinity()}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(doubleNegativeInfinity),
                                   Decimal128::kNegativeInfinity,
                                   BSONType::NumberDecimal);

    // Unlike the similar conversion in ConvertNumericToDouble, there is more than enough precision
    // to store the exact orignal value in a Decimal128.
    Document largeLongInput{{"path1", Value(0xf0000000000000fLL)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(largeLongInput), Value(0xf0000000000000fLL), BSONType::NumberDecimal);
}

TEST_F(ExpressionConvertTest, ConvertDateToDecimal) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "decimal"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document dateInput{{"path1", Value(Date_t::fromMillisSinceEpoch(123))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(dateInput), Decimal128(123), BSONType::NumberDecimal);

    Document largeDateInput{{"path1", Value(Date_t::fromMillisSinceEpoch(0xf0000000000000fLL))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(largeDateInput), Value(0xf0000000000000fLL), BSONType::NumberDecimal);
}

TEST_F(ExpressionConvertTest, ConvertDoubleToInt) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document simpleInput{{"path1", Value(1.0)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(simpleInput), 1, BSONType::NumberInt);

    // Conversions to int should always truncate the fraction (i.e., round towards 0).
    Document nonIntegerInput1{{"path1", Value(2.1)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nonIntegerInput1), 2, BSONType::NumberInt);

    Document nonIntegerInput2{{"path1", Value(2.9)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nonIntegerInput2), 2, BSONType::NumberInt);

    Document nonIntegerInput3{{"path1", Value(-2.1)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nonIntegerInput3), -2, BSONType::NumberInt);

    Document nonIntegerInput4{{"path1", Value(-2.9)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nonIntegerInput4), -2, BSONType::NumberInt);

    int maxInt = std::numeric_limits<int>::max();
    Document maxInput{{"path1", Value(static_cast<double>(maxInt))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(maxInput), maxInt, BSONType::NumberInt);

    int minInt = std::numeric_limits<int>::lowest();
    Document minInput{{"path1", Value(static_cast<double>(minInt))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(minInput), minInt, BSONType::NumberInt);
}

TEST_F(ExpressionConvertTest, ConvertOutOfBoundsDoubleToInt) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    int maxInt = std::numeric_limits<int>::max();
    double overflowInt =
        std::nextafter(static_cast<double>(maxInt), std::numeric_limits<double>::max());
    Document overflowInput{{"path1", Value(overflowInt)}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(overflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });

    int minInt = std::numeric_limits<int>::lowest();
    double negativeOverflowInt =
        std::nextafter(static_cast<double>(minInt), std::numeric_limits<double>::lowest());
    Document negativeOverflowInput{{"path1", Value(negativeOverflowInt)}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(negativeOverflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });

    Document nanInput{{"path1", Value(std::numeric_limits<double>::quiet_NaN())}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(nanInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Attempt to convert NaN value to integer");
                             });

    Document doubleInfinity{{"path1", std::numeric_limits<double>::infinity()}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(doubleInfinity),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "Attempt to convert infinity value to integer");
                             });

    Document doubleNegativeInfinity{{"path1", -std::numeric_limits<double>::infinity()}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(doubleNegativeInfinity),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "Attempt to convert infinity value to integer");
                             });
}

TEST_F(ExpressionConvertTest, ConvertOutOfBoundsDoubleToIntWithOnError) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"
                                        << "onError"
                                        << "X"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    int maxInt = std::numeric_limits<int>::max();
    double overflowInt =
        std::nextafter(static_cast<double>(maxInt), std::numeric_limits<double>::max());
    Document overflowInput{{"path1", Value(overflowInt)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(overflowInput), "X"_sd, BSONType::String);

    int minInt = std::numeric_limits<int>::lowest();
    double negativeOverflowInt =
        std::nextafter(static_cast<double>(minInt), std::numeric_limits<double>::lowest());
    Document negativeOverflowInput{{"path1", Value(negativeOverflowInt)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(negativeOverflowInput), "X"_sd, BSONType::String);

    Document nanInput{{"path1", Value(std::numeric_limits<double>::quiet_NaN())}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nanInput), "X"_sd, BSONType::String);

    Document doubleInfinity{{"path1", std::numeric_limits<double>::infinity()}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(doubleInfinity), "X"_sd, BSONType::String);

    Document doubleNegativeInfinity{{"path1", -std::numeric_limits<double>::infinity()}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(doubleNegativeInfinity), "X"_sd, BSONType::String);
}

TEST_F(ExpressionConvertTest, ConvertDoubleToLong) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "long"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document simpleInput{{"path1", Value(1.0)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(simpleInput), 1, BSONType::NumberLong);

    // Conversions to int should always truncate the fraction (i.e., round towards 0).
    Document nonIntegerInput1{{"path1", Value(2.1)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nonIntegerInput1), 2, BSONType::NumberLong);

    Document nonIntegerInput2{{"path1", Value(2.9)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nonIntegerInput2), 2, BSONType::NumberLong);

    Document nonIntegerInput3{{"path1", Value(-2.1)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(nonIntegerInput3), -2, BSONType::NumberLong);

    Document nonIntegerInput4{{"path1", Value(-2.9)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(nonIntegerInput4), -2, BSONType::NumberLong);

    // maxVal is the highest double value that will not overflow long long.
    double maxVal = std::nextafter(ExpressionConvert::kLongLongMaxPlusOneAsDouble, 0.0);
    Document maxInput{{"path1", Value(maxVal)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(maxInput), static_cast<long long>(maxVal), BSONType::NumberLong);

    // minVal is the lowest double value that will not overflow long long.
    double minVal = static_cast<double>(std::numeric_limits<long long>::lowest());
    Document minInput{{"path1", Value(minVal)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(minInput), static_cast<long long>(minVal), BSONType::NumberLong);
}

TEST_F(ExpressionConvertTest, ConvertOutOfBoundsDoubleToLong) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "long"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    double overflowLong = ExpressionConvert::kLongLongMaxPlusOneAsDouble;
    Document overflowInput{{"path1", Value(overflowLong)}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(overflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });

    double minLong = static_cast<double>(std::numeric_limits<long long>::lowest());
    double negativeOverflowLong =
        std::nextafter(static_cast<double>(minLong), std::numeric_limits<double>::lowest());
    Document negativeOverflowInput{{"path1", Value(negativeOverflowLong)}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(negativeOverflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });

    Document nanInput{{"path1", Value(std::numeric_limits<double>::quiet_NaN())}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(nanInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Attempt to convert NaN value to integer");
                             });

    Document doubleInfinity{{"path1", std::numeric_limits<double>::infinity()}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(doubleInfinity),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "Attempt to convert infinity value to integer");
                             });

    Document doubleNegativeInfinity{{"path1", -std::numeric_limits<double>::infinity()}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(doubleNegativeInfinity),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "Attempt to convert infinity value to integer");
                             });
}

TEST_F(ExpressionConvertTest, ConvertOutOfBoundsDoubleToLongWithOnError) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "long"
                                        << "onError"
                                        << "X"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    double overflowLong = ExpressionConvert::kLongLongMaxPlusOneAsDouble;
    Document overflowInput{{"path1", Value(overflowLong)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(overflowInput), "X"_sd, BSONType::String);

    double minLong = static_cast<double>(std::numeric_limits<long long>::lowest());
    double negativeOverflowLong =
        std::nextafter(static_cast<double>(minLong), std::numeric_limits<double>::lowest());
    Document negativeOverflowInput{{"path1", Value(negativeOverflowLong)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(negativeOverflowInput), "X"_sd, BSONType::String);

    Document nanInput{{"path1", Value(std::numeric_limits<double>::quiet_NaN())}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nanInput), "X"_sd, BSONType::String);

    Document doubleInfinity{{"path1", std::numeric_limits<double>::infinity()}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(doubleInfinity), "X"_sd, BSONType::String);

    Document doubleNegativeInfinity{{"path1", -std::numeric_limits<double>::infinity()}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(doubleNegativeInfinity), "X"_sd, BSONType::String);
}

TEST_F(ExpressionConvertTest, ConvertDecimalToInt) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document simpleInput{{"path1", Value(Decimal128("1.0"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(simpleInput), 1, BSONType::NumberInt);

    // Conversions to int should always truncate the fraction (i.e., round towards 0).
    Document nonIntegerInput1{{"path1", Value(Decimal128("2.1"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nonIntegerInput1), 2, BSONType::NumberInt);

    Document nonIntegerInput2{{"path1", Value(Decimal128("2.9"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nonIntegerInput2), 2, BSONType::NumberInt);

    Document nonIntegerInput3{{"path1", Value(Decimal128("-2.1"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nonIntegerInput3), -2, BSONType::NumberInt);

    Document nonIntegerInput4{{"path1", Value(Decimal128("-2.9"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nonIntegerInput3), -2, BSONType::NumberInt);

    int maxInt = std::numeric_limits<int>::max();
    Document maxInput{{"path1", Value(Decimal128(maxInt))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(maxInput), maxInt, BSONType::NumberInt);

    int minInt = std::numeric_limits<int>::min();
    Document minInput{{"path1", Value(Decimal128(minInt))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(minInput), minInt, BSONType::NumberInt);
}

TEST_F(ExpressionConvertTest, ConvertOutOfBoundsDecimalToInt) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    int maxInt = std::numeric_limits<int>::max();
    Document overflowInput{{"path1", Decimal128(maxInt).add(Decimal128(1))}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(overflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });

    int minInt = std::numeric_limits<int>::lowest();
    Document negativeOverflowInput{{"path1", Decimal128(minInt).subtract(Decimal128(1))}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(negativeOverflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });

    Document nanInput{{"path1", Decimal128::kPositiveNaN}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(nanInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Attempt to convert NaN value to integer");
                             });

    Document negativeNaNInput{{"path1", Decimal128::kNegativeNaN}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(negativeNaNInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Attempt to convert NaN value to integer");
                             });

    Document decimalInfinity{{"path1", Decimal128::kPositiveInfinity}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(decimalInfinity),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "Attempt to convert infinity value to integer");
                             });

    Document decimalNegativeInfinity{{"path1", Decimal128::kNegativeInfinity}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(decimalNegativeInfinity),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "Attempt to convert infinity value to integer");
                             });
}

TEST_F(ExpressionConvertTest, ConvertOutOfBoundsDecimalToIntWithOnError) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"
                                        << "onError"
                                        << "X"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    int maxInt = std::numeric_limits<int>::max();
    Document overflowInput{{"path1", Decimal128(maxInt).add(Decimal128(1))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(overflowInput), "X"_sd, BSONType::String);

    int minInt = std::numeric_limits<int>::lowest();
    Document negativeOverflowInput{{"path1", Decimal128(minInt).subtract(Decimal128(1))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(negativeOverflowInput), "X"_sd, BSONType::String);

    Document nanInput{{"path1", Decimal128::kPositiveNaN}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nanInput), "X"_sd, BSONType::String);

    Document negativeNaNInput{{"path1", Decimal128::kNegativeNaN}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(negativeNaNInput), "X"_sd, BSONType::String);

    Document decimalInfinity{{"path1", Decimal128::kPositiveInfinity}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(decimalInfinity), "X"_sd, BSONType::String);

    Document decimalNegativeInfinity{{"path1", Decimal128::kNegativeInfinity}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(decimalNegativeInfinity), "X"_sd, BSONType::String);
}

TEST_F(ExpressionConvertTest, ConvertDecimalToLong) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "long"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document simpleInput{{"path1", Value(Decimal128("1.0"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(simpleInput), 1, BSONType::NumberLong);

    // Conversions to long should always truncate the fraction (i.e., round towards 0).
    Document nonIntegerInput1{{"path1", Value(Decimal128("2.1"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nonIntegerInput1), 2, BSONType::NumberLong);

    Document nonIntegerInput2{{"path1", Value(Decimal128("2.9"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nonIntegerInput2), 2, BSONType::NumberLong);

    Document nonIntegerInput3{{"path1", Value(Decimal128("-2.1"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(nonIntegerInput3), -2, BSONType::NumberLong);

    Document nonIntegerInput4{{"path1", Value(Decimal128("-2.9"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(nonIntegerInput4), -2, BSONType::NumberLong);

    long long maxVal = std::numeric_limits<long long>::max();
    Document maxInput{{"path1", Value(Decimal128(std::int64_t{maxVal}))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(maxInput), maxVal, BSONType::NumberLong);

    long long minVal = std::numeric_limits<long long>::min();
    Document minInput{{"path1", Value(Decimal128(std::int64_t{minVal}))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(minInput), minVal, BSONType::NumberLong);
}

TEST_F(ExpressionConvertTest, ConvertOutOfBoundsDecimalToLong) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "long"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    long long maxVal = std::numeric_limits<long long>::max();
    Document overflowInput{{"path1", Decimal128(std::int64_t{maxVal}).add(Decimal128(1))}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(overflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });

    long long minVal = std::numeric_limits<long long>::lowest();
    Document negativeOverflowInput{
        {"path1", Decimal128(std::int64_t{minVal}).subtract(Decimal128(1))}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(negativeOverflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });

    Document nanInput{{"path1", Decimal128::kPositiveNaN}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(nanInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Attempt to convert NaN value to integer");
                             });

    Document negativeNaNInput{{"path1", Decimal128::kNegativeNaN}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(negativeNaNInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Attempt to convert NaN value to integer");
                             });

    Document decimalInfinity{{"path1", Decimal128::kPositiveInfinity}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(decimalInfinity),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "Attempt to convert infinity value to integer");
                             });

    Document decimalNegativeInfinity{{"path1", Decimal128::kNegativeInfinity}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(decimalNegativeInfinity),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "Attempt to convert infinity value to integer");
                             });
}

TEST_F(ExpressionConvertTest, ConvertOutOfBoundsDecimalToLongWithOnError) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "long"
                                        << "onError"
                                        << "X"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    long long maxVal = std::numeric_limits<long long>::max();
    Document overflowInput{{"path1", Decimal128(std::int64_t{maxVal}).add(Decimal128(1))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(overflowInput), "X"_sd, BSONType::String);

    long long minVal = std::numeric_limits<long long>::lowest();
    Document negativeOverflowInput{
        {"path1", Decimal128(std::int64_t{minVal}).subtract(Decimal128(1))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(negativeOverflowInput), "X"_sd, BSONType::String);

    Document nanInput{{"path1", Decimal128::kPositiveNaN}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(nanInput), "X"_sd, BSONType::String);

    Document negativeNaNInput{{"path1", Decimal128::kNegativeNaN}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(negativeNaNInput), "X"_sd, BSONType::String);

    Document decimalInfinity{{"path1", Decimal128::kPositiveInfinity}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(decimalInfinity), "X"_sd, BSONType::String);

    Document decimalNegativeInfinity{{"path1", Decimal128::kNegativeInfinity}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(decimalNegativeInfinity), "X"_sd, BSONType::String);
}

TEST_F(ExpressionConvertTest, ConvertDateToLong) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "long"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document dateInput{{"path1", Value(Date_t::fromMillisSinceEpoch(123LL))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(dateInput), 123LL, BSONType::NumberLong);
}

TEST_F(ExpressionConvertTest, ConvertIntToLong) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "long"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document simpleInput{{"path1", Value(1)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(simpleInput), 1LL, BSONType::NumberLong);

    int maxInt = std::numeric_limits<int>::max();
    Document maxInput{{"path1", Value(maxInt)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(maxInput), maxInt, BSONType::NumberLong);

    int minInt = std::numeric_limits<int>::min();
    Document minInput{{"path1", Value(minInt)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(minInput), minInt, BSONType::NumberLong);
}

TEST_F(ExpressionConvertTest, ConvertLongToInt) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document simpleInput{{"path1", Value(1)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(simpleInput), 1, BSONType::NumberInt);

    long long maxInt = std::numeric_limits<int>::max();
    Document maxInput{{"path1", Value(maxInt)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(maxInput), maxInt, BSONType::NumberInt);

    long long minInt = std::numeric_limits<int>::min();
    Document minInput{{"path1", Value(minInt)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(minInput), minInt, BSONType::NumberInt);
}

TEST_F(ExpressionConvertTest, ConvertOutOfBoundsLongToInt) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    long long maxInt = std::numeric_limits<int>::max();
    Document overflowInput{{"path1", Value(maxInt + 1)}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(overflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });

    long long minInt = std::numeric_limits<int>::min();
    Document negativeOverflowInput{{"path1", Value(minInt - 1)}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(negativeOverflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });
}

TEST_F(ExpressionConvertTest, ConvertOutOfBoundsLongToIntWithOnError) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"
                                        << "onError"
                                        << "X"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    long long maxInt = std::numeric_limits<int>::max();
    Document overflowInput{{"path1", Value(maxInt + 1)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(overflowInput), "X"_sd, BSONType::String);

    long long minInt = std::numeric_limits<int>::min();
    Document negativeOverflowInput{{"path1", Value(minInt - 1)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(negativeOverflowInput), "X"_sd, BSONType::String);
}

TEST_F(ExpressionConvertTest, ConvertBoolToInt) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "int"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document boolFalse{{"path1", Value(false)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(boolFalse), 0, BSONType::NumberInt);

    Document boolTrue{{"path1", Value(true)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(boolTrue), 1, BSONType::NumberInt);
}

TEST_F(ExpressionConvertTest, ConvertBoolToLong) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "long"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document boolFalse{{"path1", Value(false)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(boolFalse), 0ll, BSONType::NumberLong);

    Document boolTrue{{"path1", Value(true)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(boolTrue), 1ll, BSONType::NumberLong);
}

TEST_F(ExpressionConvertTest, ConvertNumberToDate) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "date"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document longInput{{"path1", Value(0ll)}};
    ASSERT_EQ(dateToISOStringUTC(convertExp->evaluate(longInput).getDate()),
              "1970-01-01T00:00:00.000Z");

    Document doubleInput{{"path1", Value(431568000000.0)}};
    ASSERT_EQ(dateToISOStringUTC(convertExp->evaluate(doubleInput).getDate()),
              "1983-09-05T00:00:00.000Z");

    Document doubleInputWithFraction{{"path1", Value(431568000000.987)}};
    ASSERT_EQ(dateToISOStringUTC(convertExp->evaluate(doubleInputWithFraction).getDate()),
              "1983-09-05T00:00:00.000Z");

    Document decimalInput{{"path1", Value(Decimal128("872835240000"))}};
    ASSERT_EQ(dateToISOStringUTC(convertExp->evaluate(decimalInput).getDate()),
              "1997-08-29T06:14:00.000Z");

    Document decimalInputWithFraction{{"path1", Value(Decimal128("872835240000.987"))}};
    ASSERT_EQ(dateToISOStringUTC(convertExp->evaluate(decimalInputWithFraction).getDate()),
              "1997-08-29T06:14:00.000Z");
}

TEST_F(ExpressionConvertTest, ConvertOutOfBoundsNumberToDate) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "date"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    Document doubleOverflowInput{{"path1", Value(1.0e100)}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(doubleOverflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });

    Document doubleNegativeOverflowInput{{"path1", Value(-1.0e100)}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(doubleNegativeOverflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });

    Document doubleNaN{{"path1", Value(std::numeric_limits<double>::quiet_NaN())}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(doubleNaN),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "Attempt to convert NaN value to integer type");
                             });

    Document doubleInfinity{{"path1", std::numeric_limits<double>::infinity()}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(doubleInfinity),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "Attempt to convert infinity value to integer type");
                             });

    Document doubleNegativeInfinity{{"path1", -std::numeric_limits<double>::infinity()}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(doubleNegativeInfinity),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "Attempt to convert infinity value to integer type");
                             });

    Document decimalOverflowInput{{"path1", Value(Decimal128("1.0e100"))}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(decimalOverflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });

    Document decimalNegativeOverflowInput{{"path1", Value(Decimal128("1.0e100"))}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(decimalNegativeOverflowInput),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(exception.reason(),
                                                        "Conversion would overflow target type");
                             });

    Document decimalNaN{{"path1", Decimal128::kPositiveNaN}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(decimalNaN),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "Attempt to convert NaN value to integer type");
                             });

    Document decimalNegativeNaN{{"path1", Decimal128::kNegativeNaN}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(decimalNegativeNaN),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "Attempt to convert NaN value to integer type");
                             });

    Document decimalInfinity{{"path1", Decimal128::kPositiveInfinity}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(decimalInfinity),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "Attempt to convert infinity value to integer type");
                             });

    Document decimalNegativeInfinity{{"path1", Decimal128::kNegativeInfinity}};
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate(decimalNegativeInfinity),
                             AssertionException,
                             [](const AssertionException& exception) {
                                 ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
                                 ASSERT_STRING_CONTAINS(
                                     exception.reason(),
                                     "Attempt to convert infinity value to integer type");
                             });
}

TEST_F(ExpressionConvertTest, ConvertOutOfBoundsNumberToDateWithOnError) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "date"
                                        << "onError"
                                        << "X"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);

    // Int is explicitly disallowed for date conversions. Clients must use 64-bit long instead.
    Document intInput{{"path1", Value(int{0})}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(intInput), "X"_sd, BSONType::String);

    Document doubleOverflowInput{{"path1", Value(1.0e100)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(doubleOverflowInput), "X"_sd, BSONType::String);

    Document doubleNegativeOverflowInput{{"path1", Value(-1.0e100)}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(doubleNegativeOverflowInput), "X"_sd, BSONType::String);

    Document doubleNaN{{"path1", Value(std::numeric_limits<double>::quiet_NaN())}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(doubleNaN), "X"_sd, BSONType::String);

    Document doubleInfinity{{"path1", std::numeric_limits<double>::infinity()}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(doubleInfinity), "X"_sd, BSONType::String);

    Document doubleNegativeInfinity{{"path1", -std::numeric_limits<double>::infinity()}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(doubleNegativeInfinity), "X"_sd, BSONType::String);

    Document decimalOverflowInput{{"path1", Value(Decimal128("1.0e100"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(decimalOverflowInput), "X"_sd, BSONType::String);

    Document decimalNegativeOverflowInput{{"path1", Value(Decimal128("1.0e100"))}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(decimalNegativeOverflowInput), "X"_sd, BSONType::String);

    Document decimalNaN{{"path1", Decimal128::kPositiveNaN}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(decimalNaN), "X"_sd, BSONType::String);

    Document decimalNegativeNaN{{"path1", Decimal128::kNegativeNaN}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(decimalNegativeNaN), "X"_sd, BSONType::String);

    Document decimalInfinity{{"path1", Decimal128::kPositiveInfinity}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate(decimalInfinity), "X"_sd, BSONType::String);

    Document decimalNegativeInfinity{{"path1", Decimal128::kNegativeInfinity}};
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate(decimalNegativeInfinity), "X"_sd, BSONType::String);
}

TEST_F(ExpressionConvertTest, ConvertObjectIdToDate) {
    auto expCtx = getExpCtx();

    auto spec = BSON("$convert" << BSON("input"
                                        << "$path1"
                                        << "to"
                                        << "date"));
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    convertExp = convertExp->optimize();

    Document oidInput{{"path1", Value(OID("59E8A8D8FEDCBA9876543210"))}};

    ASSERT_EQ(dateToISOStringUTC(convertExp->evaluate(oidInput).getDate()),
              "2017-10-19T13:30:00.000Z");
}

TEST_F(ExpressionConvertTest, ConvertStringToInt) {
    auto expCtx = getExpCtx();

    auto spec = fromjson("{$convert: {input: '5', to: 'int'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), 5, BSONType::NumberInt);

    spec = fromjson("{$convert: {input: '" + std::to_string(kIntMax) + "', to: 'int'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), kIntMax, BSONType::NumberInt);
}

TEST_F(ExpressionConvertTest, ConvertStringToIntOverflow) {
    auto expCtx = getExpCtx();

    auto spec = fromjson("{$convert: {input: '" + std::to_string(kIntMax + 1) + "', to: 'int'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(), "Overflow");
        });

    spec = fromjson("{$convert: {input: '" + std::to_string(kIntMin - 1) + "', to: 'int'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(), "Overflow");
        });
}

TEST_F(ExpressionConvertTest, ConvertStringToIntOverflowWithOnError) {
    auto expCtx = getExpCtx();
    const auto onErrorValue = "><(((((>"_sd;

    auto spec = fromjson("{$convert: {input: '" + std::to_string(kIntMax + 1) +
                         "', to: 'int', onError: '" + onErrorValue + "'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), onErrorValue, BSONType::String);

    spec = fromjson("{$convert: {input: '" + std::to_string(kIntMin - 1) +
                    "', to: 'int', onError: '" + onErrorValue + "'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), onErrorValue, BSONType::String);
}

TEST_F(ExpressionConvertTest, ConvertStringToLong) {
    auto expCtx = getExpCtx();

    auto spec = fromjson("{$convert: {input: '5', to: 'long'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), 5LL, BSONType::NumberLong);

    spec = fromjson("{$convert: {input: '" + std::to_string(kLongMax) + "', to: 'long'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), kLongMax, BSONType::NumberLong);
}

TEST_F(ExpressionConvertTest, ConvertStringToLongOverflow) {
    auto expCtx = getExpCtx();
    auto longMaxPlusOneAsString = std::to_string(ExpressionConvert::kLongLongMaxPlusOneAsDouble);
    // Remove digits after the decimal to avoid parse failure.
    longMaxPlusOneAsString = longMaxPlusOneAsString.substr(0, longMaxPlusOneAsString.find('.'));

    auto spec = fromjson("{$convert: {input: '" + longMaxPlusOneAsString + "', to: 'long'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(), "Overflow");
        });

    auto longMinMinusOneAsString = std::to_string(kLongNegativeOverflow);
    longMinMinusOneAsString = longMinMinusOneAsString.substr(0, longMinMinusOneAsString.find('.'));

    spec = fromjson("{$convert: {input: '" + longMinMinusOneAsString + "', to: 'long'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(), "Overflow");
        });
}

TEST_F(ExpressionConvertTest, ConvertStringToLongFailsForFloats) {
    auto expCtx = getExpCtx();

    auto spec = fromjson("{$convert: {input: '5.5', to: 'long'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(), "Bad digit \".\"");
        });

    spec = fromjson("{$convert: {input: '5.0', to: 'long'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(), "Bad digit \".\"");
        });
}

TEST_F(ExpressionConvertTest, ConvertStringToLongWithOnError) {
    auto expCtx = getExpCtx();
    const auto onErrorValue = "><(((((>"_sd;
    auto longMaxPlusOneAsString = std::to_string(ExpressionConvert::kLongLongMaxPlusOneAsDouble);
    // Remove digits after the decimal to avoid parse failure.
    longMaxPlusOneAsString = longMaxPlusOneAsString.substr(0, longMaxPlusOneAsString.find('.'));

    auto spec = fromjson("{$convert: {input: '" + longMaxPlusOneAsString +
                         "', to: 'long', onError: '" + onErrorValue + "'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), onErrorValue, BSONType::String);

    auto longMinMinusOneAsString = std::to_string(kLongNegativeOverflow);
    longMinMinusOneAsString = longMinMinusOneAsString.substr(0, longMinMinusOneAsString.find('.'));

    spec = fromjson("{$convert: {input: '" + longMinMinusOneAsString + "', to: 'long', onError: '" +
                    onErrorValue + "'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), onErrorValue, BSONType::String);

    spec = fromjson("{$convert: {input: '5.5', to: 'long', onError: '" + onErrorValue + "'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), onErrorValue, BSONType::String);

    spec = fromjson("{$convert: {input: '5.0', to: 'long', onError: '" + onErrorValue + "'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), onErrorValue, BSONType::String);
}

TEST_F(ExpressionConvertTest, ConvertStringToDouble) {
    auto expCtx = getExpCtx();

    auto spec = fromjson("{$convert: {input: '5', to: 'double'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), 5.0, BSONType::NumberDouble);

    spec = fromjson("{$convert: {input: '5.5', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), 5.5, BSONType::NumberDouble);

    spec = fromjson("{$convert: {input: '.5', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), 0.5, BSONType::NumberDouble);

    spec = fromjson("{$convert: {input: '+5', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), 5.0, BSONType::NumberDouble);

    spec = fromjson("{$convert: {input: '+5.0e42', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), 5.0e42, BSONType::NumberDouble);
}

TEST_F(ExpressionConvertTest, ConvertStringToDoubleWithPrecisionLoss) {
    auto expCtx = getExpCtx();

    // Note that the least significant bits get lost, because the significand of a double is not
    // wide enough for the given input string in its entirety.
    auto spec = fromjson("{$convert: {input: '10000000000000000001', to: 'double'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), 1e19, BSONType::NumberDouble);

    // Again, some precision is lost in the conversion to double.
    spec = fromjson("{$convert: {input: '1.125000000000000000005', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), 1.125, BSONType::NumberDouble);
}

TEST_F(ExpressionConvertTest, ConvertStringToDoubleFailsForInvalidFloats) {
    auto expCtx = getExpCtx();

    auto spec = fromjson("{$convert: {input: '.5.', to: 'double'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(), "Did not consume whole number");
        });

    spec = fromjson("{$convert: {input: '5.5f', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(), "Did not consume whole number");
        });
}

TEST_F(ExpressionConvertTest, ConvertInfinityStringsToDouble) {
    auto expCtx = getExpCtx();
    auto infValue = std::numeric_limits<double>::infinity();

    auto spec = fromjson("{$convert: {input: 'Infinity', to: 'double'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), infValue, BSONType::NumberDouble);

    spec = fromjson("{$convert: {input: 'INF', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), infValue, BSONType::NumberDouble);

    spec = fromjson("{$convert: {input: 'infinity', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), infValue, BSONType::NumberDouble);

    spec = fromjson("{$convert: {input: '+InFiNiTy', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), infValue, BSONType::NumberDouble);

    spec = fromjson("{$convert: {input: '-Infinity', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), -infValue, BSONType::NumberDouble);

    spec = fromjson("{$convert: {input: '-INF', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), -infValue, BSONType::NumberDouble);

    spec = fromjson("{$convert: {input: '-InFiNiTy', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), -infValue, BSONType::NumberDouble);

    spec = fromjson("{$convert: {input: '-inf', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), -infValue, BSONType::NumberDouble);

    spec = fromjson("{$convert: {input: '-infinity', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), -infValue, BSONType::NumberDouble);
}

TEST_F(ExpressionConvertTest, ConvertZeroStringsToDouble) {
    auto expCtx = getExpCtx();

    auto spec = fromjson("{$convert: {input: '-0', to: 'double'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    auto result = convertExp->evaluate({});
    ASSERT_VALUE_CONTENTS_AND_TYPE(result, 0, BSONType::NumberDouble);
    ASSERT_TRUE(std::signbit(result.getDouble()));

    spec = fromjson("{$convert: {input: '-0.0', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    result = convertExp->evaluate({});
    ASSERT_VALUE_CONTENTS_AND_TYPE(result, 0, BSONType::NumberDouble);
    ASSERT_TRUE(std::signbit(result.getDouble()));

    spec = fromjson("{$convert: {input: '+0', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    result = convertExp->evaluate({});
    ASSERT_VALUE_CONTENTS_AND_TYPE(result, 0, BSONType::NumberDouble);
    ASSERT_FALSE(std::signbit(result.getDouble()));

    spec = fromjson("{$convert: {input: '+0.0', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    result = convertExp->evaluate({});
    ASSERT_VALUE_CONTENTS_AND_TYPE(result, 0, BSONType::NumberDouble);
    ASSERT_FALSE(std::signbit(result.getDouble()));
}

TEST_F(ExpressionConvertTest, ConvertNanStringsToDouble) {
    auto expCtx = getExpCtx();
    auto nanValue = std::numeric_limits<double>::quiet_NaN();

    auto spec = fromjson("{$convert: {input: 'nan', to: 'double'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    auto result = convertExp->evaluate({});
    ASSERT_TRUE(std::isnan(result.getDouble()));
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), nanValue, BSONType::NumberDouble);

    spec = fromjson("{$convert: {input: 'Nan', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    result = convertExp->evaluate({});
    ASSERT_TRUE(std::isnan(result.getDouble()));

    spec = fromjson("{$convert: {input: 'NaN', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    result = convertExp->evaluate({});
    ASSERT_TRUE(std::isnan(result.getDouble()));

    spec = fromjson("{$convert: {input: '-NAN', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    result = convertExp->evaluate({});
    ASSERT_TRUE(std::isnan(result.getDouble()));

    spec = fromjson("{$convert: {input: '+NaN', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    result = convertExp->evaluate({});
    ASSERT_TRUE(std::isnan(result.getDouble()));
}

TEST_F(ExpressionConvertTest, ConvertStringToDoubleOverflow) {
    auto expCtx = getExpCtx();

    auto spec = fromjson("{$convert: {input: '" + kDoubleOverflow.toString() + "', to: 'double'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(), "Out of range");
        });

    spec =
        fromjson("{$convert: {input: '" + kDoubleNegativeOverflow.toString() + "', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(), "Out of range");
        });
}

TEST_F(ExpressionConvertTest, ConvertStringToDoubleUnderflow) {
    auto expCtx = getExpCtx();

    auto spec = fromjson("{$convert: {input: '1E-1000', to: 'double'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(
                exception.reason(),
                "Failed to parse number '1E-1000' in $convert with no onError value: Out of range");
        });

    spec = fromjson("{$convert: {input: '-1E-1000', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(),
                                   "Failed to parse number '-1E-1000' in $convert with no onError "
                                   "value: Out of range");
        });
}

TEST_F(ExpressionConvertTest, ConvertStringToDoubleWithOnError) {
    auto expCtx = getExpCtx();
    const auto onErrorValue = "><(((((>"_sd;

    auto spec = fromjson("{$convert: {input: '" + kDoubleOverflow.toString() +
                         "', to: 'double', onError: '" + onErrorValue + "'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), onErrorValue, BSONType::String);

    spec = fromjson("{$convert: {input: '" + kDoubleNegativeOverflow.toString() +
                    "', to: 'double', onError: '" + onErrorValue + "'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), onErrorValue, BSONType::String);

    spec = fromjson("{$convert: {input: '.5.', to: 'double', onError: '" + onErrorValue + "'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), onErrorValue, BSONType::String);

    spec = fromjson("{$convert: {input: '5.5f', to: 'double', onError: '" + onErrorValue + "'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), onErrorValue, BSONType::String);
}

TEST_F(ExpressionConvertTest, ConvertStringToDecimal) {
    auto expCtx = getExpCtx();

    auto spec = fromjson("{$convert: {input: '5', to: 'decimal'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), 5, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: '2.02', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate({}), Decimal128("2.02"), BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: '2.02E200', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate({}), Decimal128("2.02E200"), BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: '" + Decimal128::kLargestPositive.toString() +
                    "', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate({}), Decimal128::kLargestPositive, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: '" + Decimal128::kLargestNegative.toString() +
                    "', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate({}), Decimal128::kLargestNegative, BSONType::NumberDecimal);
}

TEST_F(ExpressionConvertTest, ConvertInfinityStringsToDecimal) {
    auto expCtx = getExpCtx();
    auto infValue = Decimal128::kPositiveInfinity;
    auto negInfValue = Decimal128::kNegativeInfinity;

    auto spec = fromjson("{$convert: {input: 'Infinity', to: 'decimal'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), infValue, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: 'INF', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), infValue, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: 'infinity', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), infValue, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: '+InFiNiTy', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), infValue, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: '-Infinity', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), negInfValue, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: '-INF', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), negInfValue, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: '-InFiNiTy', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), negInfValue, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: '-inf', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), negInfValue, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: '-infinity', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), negInfValue, BSONType::NumberDecimal);
}

TEST_F(ExpressionConvertTest, ConvertNanStringsToDecimal) {
    auto expCtx = getExpCtx();
    auto positiveNan = Decimal128::kPositiveNaN;
    auto negativeNan = Decimal128::kNegativeNaN;

    auto spec = fromjson("{$convert: {input: 'nan', to: 'decimal'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), positiveNan, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: 'Nan', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), positiveNan, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: 'NaN', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), positiveNan, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: '+NaN', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), positiveNan, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: '-NAN', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), negativeNan, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: '-nan', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), negativeNan, BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: '-NaN', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), negativeNan, BSONType::NumberDecimal);
}

TEST_F(ExpressionConvertTest, ConvertZeroStringsToDecimal) {
    auto expCtx = getExpCtx();

    auto spec = fromjson("{$convert: {input: '-0', to: 'decimal'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    auto result = convertExp->evaluate({});
    ASSERT_VALUE_CONTENTS_AND_TYPE(result, 0, BSONType::NumberDecimal);
    ASSERT_TRUE(result.getDecimal().isZero());
    ASSERT_TRUE(result.getDecimal().isNegative());

    spec = fromjson("{$convert: {input: '-0.0', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    result = convertExp->evaluate({});
    ASSERT_VALUE_CONTENTS_AND_TYPE(result, 0, BSONType::NumberDecimal);
    ASSERT_TRUE(result.getDecimal().isZero());
    ASSERT_TRUE(result.getDecimal().isNegative());

    spec = fromjson("{$convert: {input: '+0', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    result = convertExp->evaluate({});
    ASSERT_VALUE_CONTENTS_AND_TYPE(result, 0, BSONType::NumberDecimal);
    ASSERT_TRUE(result.getDecimal().isZero());
    ASSERT_FALSE(result.getDecimal().isNegative());

    spec = fromjson("{$convert: {input: '+0.0', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    result = convertExp->evaluate({});
    ASSERT_VALUE_CONTENTS_AND_TYPE(result, 0, BSONType::NumberDecimal);
    ASSERT_TRUE(result.getDecimal().isZero());
    ASSERT_FALSE(result.getDecimal().isNegative());
}

TEST_F(ExpressionConvertTest, ConvertStringToDecimalOverflow) {
    auto expCtx = getExpCtx();

    auto spec = fromjson("{$convert: {input: '1E6145', to: 'decimal'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(),
                                   "Conversion from string to decimal would overflow");
        });

    spec = fromjson("{$convert: {input: '-1E6145', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(),
                                   "Conversion from string to decimal would overflow");
        });
}

TEST_F(ExpressionConvertTest, ConvertStringToDecimalUnderflow) {
    auto expCtx = getExpCtx();

    auto spec = fromjson("{$convert: {input: '1E-6178', to: 'decimal'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(),
                                   "Conversion from string to decimal would underflow");
        });

    spec = fromjson("{$convert: {input: '-1E-6177', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(),
                                   "Conversion from string to decimal would underflow");
        });
}

TEST_F(ExpressionConvertTest, ConvertStringToDecimalWithPrecisionLoss) {
    auto expCtx = getExpCtx();

    auto spec =
        fromjson("{$convert: {input: '10000000000000000000000000000000001', to: 'decimal'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate({}), Decimal128("1e34"), BSONType::NumberDecimal);

    spec = fromjson("{$convert: {input: '1.1250000000000000000000000000000001', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate({}), Decimal128("1.125"), BSONType::NumberDecimal);
}

TEST_F(ExpressionConvertTest, ConvertStringToDecimalWithOnError) {
    auto expCtx = getExpCtx();
    const auto onErrorValue = "><(((((>"_sd;

    auto spec =
        fromjson("{$convert: {input: '1E6145', to: 'decimal', onError: '" + onErrorValue + "'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), onErrorValue, BSONType::String);

    spec =
        fromjson("{$convert: {input: '-1E-6177', to: 'decimal', onError: '" + onErrorValue + "'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), onErrorValue, BSONType::String);
}

TEST_F(ExpressionConvertTest, ConvertStringToNumberFailsForHexStrings) {
    auto expCtx = getExpCtx();
    auto invalidHexFailure = [](const AssertionException& exception) {
        ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
        ASSERT_STRING_CONTAINS(exception.reason(), "Illegal hexadecimal input in $convert");
    };

    auto spec = fromjson("{$convert: {input: '0xFF', to: 'int'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate({}), AssertionException, invalidHexFailure);

    spec = fromjson("{$convert: {input: '0xFF', to: 'long'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate({}), AssertionException, invalidHexFailure);

    spec = fromjson("{$convert: {input: '0xFF', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate({}), AssertionException, invalidHexFailure);

    spec = fromjson("{$convert: {input: '0xFF', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate({}), AssertionException, invalidHexFailure);

    spec = fromjson("{$convert: {input: '0x00', to: 'int'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate({}), AssertionException, invalidHexFailure);

    spec = fromjson("{$convert: {input: '0x00', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(convertExp->evaluate({}), AssertionException, invalidHexFailure);

    spec = fromjson("{$convert: {input: 'FF', to: 'double'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(), "Did not consume whole number");
        });

    spec = fromjson("{$convert: {input: 'FF', to: 'decimal'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(), "Failed to parse string to decimal");
        });
}

TEST_F(ExpressionConvertTest, ConvertStringToOID) {
    auto expCtx = getExpCtx();
    auto oid = OID::gen();

    auto spec = fromjson("{$convert: {input: '" + oid.toString() + "', to: 'objectId'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), oid, BSONType::jstOID);

    spec = fromjson("{$convert: {input: '123456789abcdef123456789', to: 'objectId'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(
        convertExp->evaluate({}), OID("123456789abcdef123456789"), BSONType::jstOID);
}

TEST_F(ExpressionConvertTest, ConvertToOIDFailsForInvalidHexStrings) {
    auto expCtx = getExpCtx();

    auto spec = fromjson("{$convert: {input: 'InvalidHexButSizeCorrect', to: 'objectId'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(), "Invalid character found in hex string");
        });

    spec = fromjson("{$convert: {input: 'InvalidSize', to: 'objectId'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(), "Invalid string length for parsing to OID");
        });

    spec = fromjson("{$convert: {input: '0x123456789abcdef123456789', to: 'objectId'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_THROWS_WITH_CHECK(
        convertExp->evaluate({}), AssertionException, [](const AssertionException& exception) {
            ASSERT_EQ(exception.code(), ErrorCodes::ConversionFailure);
            ASSERT_STRING_CONTAINS(exception.reason(), "Invalid string length for parsing to OID");
        });
}

TEST_F(ExpressionConvertTest, ConvertToOIDWithOnError) {
    auto expCtx = getExpCtx();
    const auto onErrorValue = "><(((((>"_sd;

    auto spec =
        fromjson("{$convert: {input: 'InvalidHexButSizeCorrect', to: 'objectId', onError: '" +
                 onErrorValue + "'}}");
    auto convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), onErrorValue, BSONType::String);

    spec = fromjson("{$convert: {input: 'InvalidSize', to: 'objectId', onError: '" + onErrorValue +
                    "'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), onErrorValue, BSONType::String);

    spec = fromjson("{$convert: {input: '0x123456789abcdef123456789', to: 'objectId', onError: '" +
                    onErrorValue + "'}}");
    convertExp = Expression::parseExpression(expCtx, spec, expCtx->variablesParseState);
    ASSERT_VALUE_CONTENTS_AND_TYPE(convertExp->evaluate({}), onErrorValue, BSONType::String);
}

}  // namespace ExpressionConvertTest

}  // namespace mongo
