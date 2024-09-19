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
	EXPECT_EQ("This is bad: \\u123X", (std::string)json);
	json = JSON::JSON::FromString("\"This is bad: \\x\"");
	EXPECT_EQ("This is bad: \\x", (std::string)json);
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
	JSON::JSON json(3.14159);
	ASSERT_EQ("3.14159", json.ToString());
}

TEST(JSONTests, ToFloatingPoint)
{
	const auto json = JSON::JSON::FromString("3.14159");
	ASSERT_TRUE((double)json == 3.14159);
}
