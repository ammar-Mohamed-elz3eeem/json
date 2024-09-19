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

}
