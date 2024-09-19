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

}
