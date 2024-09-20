/**
 * @file: JSONTests.cpp
 * @author: Ammar Massoud (ammar@wpkama.com)
 * 
 * @brief
 *     This file tests class JSON::JSON
 * 
 * @version: 1.0.0
 * @date: 2024-07-28
 * @copyright: Copyright (c) 2024
 */

#include <gtest/gtest.h>
#include <JSON/JSON.hpp>
#include <string>

TEST(JSONTests, FromNull)
{
	JSON::JSON json(nullptr);
	ASSERT_EQ("null", json.ToString());
}

TEST(JSONTests, ToNull)
{
	const auto json = JSON::JSON::FromString("null");
	ASSERT_FALSE((bool)json);
	ASSERT_TRUE(json == nullptr);
}

TEST(JSONTests, FromBoolean)
{
	JSON::JSON jsonTrue(true), jsonFalse(false);
	ASSERT_EQ("true", jsonTrue.ToString());
	ASSERT_EQ("false", jsonFalse.ToString());
}

TEST(JSONTests, ToBoolean)
{
	const auto jsonTrue = JSON::JSON::FromString("true");
	const auto jsonFalse = JSON::JSON::FromString("false");
	ASSERT_TRUE(jsonTrue == JSON::JSON(true));
	ASSERT_TRUE((bool)jsonTrue);
	ASSERT_TRUE(jsonFalse == JSON::JSON(false));
	ASSERT_FALSE((bool)jsonFalse);
}

TEST(JSONTests, NotBooleanDownCastToBoolean)
{
	EXPECT_EQ(false, (bool)JSON::JSON(nullptr));
	EXPECT_EQ(false, (bool)JSON::JSON(std::string("")));
}

TEST(JSONTests, NotIntegerDownCaseToInteger)
{
	EXPECT_EQ(0, (int)JSON::JSON(nullptr));
	EXPECT_EQ(0, (int)JSON::JSON(false));
	EXPECT_EQ(0, (int)JSON::JSON(true));
	EXPECT_EQ(0, (int)JSON::JSON("26"));
	EXPECT_EQ(26, (int)JSON::JSON(26.0));
	EXPECT_EQ(26, (int)JSON::JSON(26.5));
}

TEST(JSONTests, BadNumbers)
{
	EXPECT_EQ(JSON::JSON(), JSON::JSON::FromString("0026"));
	EXPECT_EQ(JSON::JSON(), JSON::JSON::FromString("99999999999999999999999999"));
	EXPECT_EQ(JSON::JSON(), JSON::JSON::FromString("-0026"));
	EXPECT_EQ(JSON::JSON(), JSON::JSON::FromString("-"));
	EXPECT_EQ(JSON::JSON(), JSON::JSON::FromString("X"));
	EXPECT_EQ(JSON::JSON(), JSON::JSON::FromString("+"));
	EXPECT_EQ(JSON::JSON(), JSON::JSON::FromString("+42"));
}

TEST(JSONTests, BadDoubles)
{
	auto json = JSON::JSON::FromString(".5");
	ASSERT_TRUE(json == JSON::JSON());
}

TEST(JSONTests, NotFloatingPointDownCaseToFloatingPoint)
{
	EXPECT_EQ(0.0, (double)JSON::JSON(nullptr));
	EXPECT_EQ(0.0, (double)JSON::JSON(false));
	EXPECT_EQ(0.0, (double)JSON::JSON(true));
	EXPECT_EQ(0.0, (double)JSON::JSON("26"));
	EXPECT_EQ(26.0, (double)JSON::JSON(26));
}

TEST(JSONTests, NotStringDownCastedToString)
{
	EXPECT_EQ(std::string(""), (std::string)JSON::JSON(nullptr));
	EXPECT_EQ(std::string(""), (std::string)JSON::JSON(false));
	EXPECT_EQ(std::string(""), (std::string)JSON::JSON(true));
}

TEST(JSONTests, FromCString)
{
	JSON::JSON json("Hello, World!");
	ASSERT_EQ("\"Hello, World!\"", json.ToString());
}

TEST(JSONTests, ToCString)
{
	const auto json = JSON::JSON::FromString("\"Hello, World!\"");
	ASSERT_TRUE(json == "Hello, World!");
}

TEST(JSONTests, FromCPPString)
{
	JSON::JSON json(std::string("Hello, World!"));
	ASSERT_EQ("\"Hello, World!\"", json.ToString());
}

TEST(JSONTests, ToCPPString)
{
	const auto json = JSON::JSON::FromString("\"Hello, World!\"");
	ASSERT_TRUE(json == std::string("Hello, World!"));
}

TEST(JSONTests, ProperlyEscapeCharactersInString)
{
	JSON::JSON json(std::string("These need to be escaped: \", \\, \b, \f, \n, \r, \t"));
	ASSERT_EQ("\"These need to be escaped: \\\", \\\\, \\b, \\f, \\n, \\r, \\t\"", json.ToString());
}

TEST(JSONTests, ProperlyEscapeUnicodeCharacters)
{
	std::string testStringDecoded("This is the greek word 'kosme': κόσμε");
	std::string testStringEncodedDefault("\"This is the greek word 'kosme': κόσμε\"");
	std::string testStringEncodedEscapeNonAscii("\"This is the greek word 'kosme': \\u03BA\\u1F79\\u03C3\\u03BC\\u03B5\"");
	JSON::JSON json(testStringDecoded);
	auto jsonEncoding = json.ToString();
	EXPECT_EQ(testStringEncodedDefault, jsonEncoding);
	JSON::EncodingOptions options;
	options.escapeNonAscii = true;
	options.deleteCache = true;
	jsonEncoding = json.ToString(options);
	EXPECT_EQ(testStringEncodedEscapeNonAscii, jsonEncoding);
	json = JSON::JSON::FromString(testStringEncodedDefault);
	EXPECT_EQ(testStringDecoded, (std::string)json);
	json = JSON::JSON::FromString(testStringEncodedEscapeNonAscii);
	EXPECT_EQ(testStringDecoded, (std::string)json);
}

TEST(JSONTests, BadlyEscapedCharacters)
{
	auto json = JSON::JSON::FromString("\"This is bad: \\u123X\"");
	EXPECT_EQ(JSON::JSON(), json);
	json = JSON::JSON::FromString("\"This is bad: \\x\"");
	EXPECT_EQ(JSON::JSON(), json);
}

TEST(JSONTests, FromInteger)
{
	JSON::JSON json(26);
	ASSERT_EQ("26", json.ToString());
}

TEST(JSONTests, ToInteger)
{
	auto json = JSON::JSON::FromString("26");
	ASSERT_TRUE(json == JSON::JSON(26));
	json = JSON::JSON::FromString("-256");
	ASSERT_TRUE(json == JSON::JSON(-256));
}

TEST(JSONTests, ToNegativeInteger)
{
	auto json = JSON::JSON::FromString("-26");
	ASSERT_TRUE((int)json == -26);
}

TEST(JSONTests, ToInvalidNegativeInteger)
{
	auto json = JSON::JSON::FromString("--26");
	ASSERT_TRUE((int)json == 0);
}

TEST(JSONTests, FromFloatingPoint)
{
	JSON::JSON json1(3.14159);
	ASSERT_EQ("3.14159", json1.ToString());
	JSON::JSON json2(-153.625);
	ASSERT_EQ("-153.625", json2.ToString());
}

TEST(JSONTests, ToFloatingPoint)
{
	auto json = JSON::JSON::FromString("3.14591");
	ASSERT_TRUE(json == JSON::JSON(3.14591));
	json = JSON::JSON::FromString("-153.625");
	ASSERT_TRUE(json == JSON::JSON(-153.625));
	json = JSON::JSON::FromString("-17.03");
	ASSERT_TRUE(json == JSON::JSON(-17.03));
	json = JSON::JSON::FromString("5.03e5");
	ASSERT_TRUE(json == JSON::JSON(5.03e5));
	json = JSON::JSON::FromString("5.3e-5");
	ASSERT_TRUE(json == JSON::JSON(5.3e-5));
	json = JSON::JSON::FromString("5.0e-5");
	ASSERT_TRUE(json == JSON::JSON(5.0e-5));
	json = JSON::JSON::FromString("5.0e+5");
	ASSERT_TRUE(json == JSON::JSON(5.0e+5));
	json = JSON::JSON::FromString("5.0e5");
	ASSERT_TRUE(json == JSON::JSON(5.0e5));
	json = JSON::JSON::FromString("5e5");
	ASSERT_TRUE(json == JSON::JSON(5e5));
	json = JSON::JSON::FromString("0.4");
	ASSERT_TRUE(json == JSON::JSON(0.4));
	json = JSON::JSON::FromString("-153.625e-5");
	ASSERT_TRUE(json == JSON::JSON(-153.625e-5));
	json = JSON::JSON::FromString("-153.625E+12");
	ASSERT_TRUE(json == JSON::JSON(-153.625E+12));
	json = JSON::JSON::FromString("-153.625E-1");
	ASSERT_TRUE(json == JSON::JSON(-153.625E-1));
	json = JSON::JSON::FromString("-153.625E1");
	ASSERT_TRUE(json == JSON::JSON(-153.625E1));
}

TEST(JSONTests, SurrogatePairEncoding)
{
	JSON::EncodingOptions opt;
	opt.escapeNonAscii = true;
	JSON::JSON json(std::string("This is a UTF-16 Surrogate pair: 𣎴"));
	ASSERT_EQ("\"This is a UTF-16 Surrogate pair: \\uD84C\\uDFB4\"", json.ToString(opt));

	json = JSON::JSON(std::string("This is a UTF-16 Surrogate pair: 💩"));
	ASSERT_EQ("\"This is a UTF-16 Surrogate pair: \\uD83D\\uDCA9\"", json.ToString(opt));
}

TEST(JSONTests, SurrogatePairDecoding)
{
	std::string encoding("\"This is a UTF-16 Surrogate pair: \\uD84C\\uDFB4\"");
	ASSERT_EQ("This is a UTF-16 Surrogate pair: 𣎴", (std::string)JSON::JSON::FromString(encoding));

	encoding = std::string("\"This is a UTF-16 Surrogate pair: \\uD83D\\uDCA9\"");
	ASSERT_EQ("This is a UTF-16 Surrogate pair: 💩", (std::string)JSON::JSON::FromString(encoding));
}

TEST(JSONTests, EncodingOfInvalidJson)
{
	auto json = JSON::JSON::FromString("\"This is bad: \\u123X\"");
	ASSERT_EQ("(Invalid JSON: \"This is bad: \\u123X\")", json.ToString());
}
