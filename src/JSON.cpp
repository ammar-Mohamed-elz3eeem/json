/**
 * @file: JSON.cpp
 * @author: Ammar Massoud (ammar@wpkama.com)
 * 
 * @brief
 *     This file implements class JSON::JSON
 * 
 * @version: 1.0.0
 * @date: 2024-07-28
 * @copyright: Copyright (c) 2024
 */

#include <JSON/JSON.hpp>

/**
 * TODO: covert FromString to accept UTF-8 encoded codepoints.
 * HACK: ADD test case for 1e as json value and it should give
 *       invalid json value.
 */

namespace
{
	/**
	 * @brief
	 *     This represents all whitespace characters but as
	 *     utf8 codepoints.
	 */
	const std::set<Utf8::UnicodeCodePoint> WHITESPACES = {
		0x20,	// Space ' '
		0x09,	// Tab '\t'
		0x0D,	// Carriage return '\r'
		0x0A	// Line feed '\n'
	};

	const std::set<Utf8::UnicodeCodePoint> FLOAT_INDICATORS = {
		0x2E,
		0x65,
		0x45
	};

	/**
	 * @brief
	 *     This maps escaped representations of special characters back
	 *     to the actual characters they represent.
	 */
	std::map<Utf8::UnicodeCodePoint, Utf8::UnicodeCodePoint> SPECIAL_ESCAPE_DECODINGS{
		{ 0x22, 0x22  }, // """
		{ 0x5C, 0x5C  }, // "\"
		{ 0x2F, 0x2F  }, // "\"
		{ 0x62, 0x08  }, // "\b"
		{ 0x66, 0x0C  }, // "\f"
		{ 0x6E, 0x0A  }, // "\n"
		{ 0x72, 0x0D  }, // "\r"
		{ 0x74, 0x09  }  // "\t"
	};

	/**
	 * @brief
	 *     This maps special characters to their escaped representations.
	 */
	std::map<Utf8::UnicodeCodePoint, Utf8::UnicodeCodePoint> SPECIAL_ESCAPE_ENCODINGS{
		{ 0x22, 0x22  }, // """
		{ 0x5C, 0x5C  }, // "\"
		{ 0x2F, 0x2F  }, // "\"
		{ 0x08, 0x62  }, // "\b"
		{ 0x0C, 0x66  }, // "\f"
		{ 0x0A, 0x6E  }, // "\n"
		{ 0x0D, 0x72  }, // "\r"
		{ 0x09, 0x74  }  // "\t"
	};

	/**
	 * @brief
	 *     This function finds the first character on the given 
	 *     codepoints that is not listed on the given search set.
	 *
	 * @param[in] codepoints
	 *     These are the codepoints to search in.
	 * 
	 * @param[in] searchSet
	 *     These are the characters that we need to compare
	 *     with our codepoints.
	 * 
	 * @param[in] forwardDirection
	 *     flag to indicate whether start from beginning or
	 *     start from end.
	 * 
	 * @return
	 *     the position of the first character that doesn't exist
	 *     on the given search set.
	 * 
	 * @retval codepoints.size()
	 *     if all codepoints exist on the given search set. 
	 */
	size_t findFirstNotOf(
		const std::vector<Utf8::UnicodeCodePoint>& codepoints,
		const std::set<Utf8::UnicodeCodePoint>& searchSet,
		bool forwardDirection
	) {
		size_t codepointsSize = codepoints.size();
		for (size_t i = 0; i < codepointsSize; i++)
			if (searchSet.find(codepoints[forwardDirection ? i : codepointsSize - i - 1]) == searchSet.end())
				return forwardDirection ? i : codepointsSize - i;
		return codepointsSize;
	}

	/**
	 * @brief
	 *     This function finds the first character on the given 
	 *     codepoints that is listed on the given search set.
	 *
	 * @param[in] codepoints
	 *     These are the codepoints to search in.
	 * 
	 * @param[in] searchSet
	 *     These are the characters that we need to compare
	 *     with our codepoints.
	 * 
	 * @param[in] forwardDirection
	 *     flag to indicate whether start from beginning or
	 *     start from end.
	 * 
	 * @return
	 *     the position of the first character that exist
	 *     on the given search set.
	 * 
	 * @retval codepoints.size()
	 *     if all codepoints exist on the given search set. 
	 */
	size_t findFirstOf(
		const std::vector<Utf8::UnicodeCodePoint>& codepoints,
		const std::set<Utf8::UnicodeCodePoint>& searchSet,
		bool forwardDirection
	) {
		size_t codepointsSize = codepoints.size();
		for (size_t i = 0; i < codepointsSize; i++)
			if (searchSet.find(codepoints[forwardDirection ? i : codepointsSize - i - 1]) != searchSet.end())
				return forwardDirection ? i : codepointsSize - i;
		return codepointsSize;
	}

	/**
	 * @brief
	 *     convert a code point to four hexadecimal digits string.
	 * 
	 * @param codepoint 
	 *     This is the code point to render as four hex digits.
	 * 
	 * @returns
	 *     String consisting of four hex digits which renders
	 *     the code point.
	 */
	std::string codepointToFourHexDigits(Utf8::UnicodeCodePoint codepoint)
	{
		std::string fourHexDigits;
		for (size_t i = 0; i < 4; ++i)
		{
			const auto nibble = (codepoint >> (4 * (3 - i)) & 0x0F);
			if (nibble < 10)
				fourHexDigits += (char)(nibble + '0');
			else
				fourHexDigits += (char)(nibble - 10) + 'A';
		}
		return fourHexDigits;
	}

	/**
	 * @brief
	 *     This function produces the escaped version of the
	 *     given string.
	 * 
	 * @param[in] s
	 *     This is the string which we need to be escaped.
	 * 
	 * @param[in] options
	 *     This is used to configure the various options
	 *     having to do with encoding a JSON object into
	 *     its string format.
	 * 
	 * @return
	 *     The escaped version of the given string.
	 */
	std::string escape(
		const std::string &s,
		const JSON::EncodingOptions &options
	) {
		Utf8::Utf8 decoder, encoder;
		std::string output;
		for (const auto codepoint: decoder.decode(s))
		{
			if (
				(codepoint == 0x22) ||
				(codepoint == 0x5C) ||
				(codepoint < 0x20)
			) {
				output += '\\';
				const auto entry = SPECIAL_ESCAPE_ENCODINGS.find(codepoint);
				if (entry == SPECIAL_ESCAPE_ENCODINGS.end())
				{
					output += 'u';
					output += codepointToFourHexDigits(codepoint);
				}
				else
				{
					output += (char)entry->second;
				}
				// output += (char)codepoint;
			}
			else if (
				options.escapeNonAscii &&
				(codepoint > 0x7F)
			) {
				if (codepoint > 0xFFFF)
				{
					output += "\\u";
					output += codepointToFourHexDigits(0xD800 + (((codepoint - 0x10000) >> 10) & 0x3FF));
					output += "\\u";
					output += codepointToFourHexDigits(0xDC00 + ((codepoint - 0x10000) & 0x3FF));
				}
				else
				{
					output += "\\u";
					output += codepointToFourHexDigits(codepoint);
				}
			}
			else
			{
				const auto encoded = encoder.encode({ codepoint });
				output += std::string(encoded.begin(), encoded.end());
			}
		}
		return output;
	}

	/**
	 * @brief
	 *     This function produces the unescaped version of the
	 *     given string.
	 * 
	 * @param[in] s
	 *     This is the string which we need to be unescaped.
	 * 
	 * @param[out] output
	 *     This is the string in which we put the unescaped
	 *     version on the given string.
	 * 
	 * @return
	 *     The unescaped version of the given string.
	 */
	bool unescape(
		const std::string &s,
		std::string &output
	) {
		Utf8::Utf8 decoder, encoder;
		size_t state = 0;
		Utf8::UnicodeCodePoint cpFromHexDigit = 0;
		Utf8::UnicodeCodePoint firstHalfOfSurrogatePair = 0;
		std::vector<Utf8::UnicodeCodePoint> hexDigitsOriginal;
		for (const auto codepoint: decoder.decode(s))
		{
			switch (state)
			{
			case 0: /// Initial State
				{
					if (codepoint == 0x5C)
					{
						state = 1;
					}
					else if (firstHalfOfSurrogatePair == 0)
					{
						const auto encoded = encoder.encode({ codepoint });
						output += std::string(encoded.begin(), encoded.end());
					}
					else
					{
						return false;
					}
				}
				break;

			case 1: /// Escape character
				{
					if (codepoint == 0x75)
					{
						state = 2;
						cpFromHexDigit = 0;
						hexDigitsOriginal = {0x5C, 0x75};
					}
					else if (firstHalfOfSurrogatePair == 0)
					{
						Utf8::UnicodeCodePoint alternative = codepoint;
						const auto entry = SPECIAL_ESCAPE_DECODINGS.find(codepoint);
						if (entry == SPECIAL_ESCAPE_DECODINGS.end())
						{
							return false;
						}
						const auto encoded = encoder.encode({ entry->second });
						output += std::string(encoded.begin(), encoded.end());
						state = 0;
					}
					else
					{
						return false;
					}
				}
				break;
			
			case 2: // First HexDigit
			case 3: // Second HexDigit
			case 4: // Third HexDigit
			case 5: // Fourth HexDigit
				{
					hexDigitsOriginal.push_back(codepoint);
					cpFromHexDigit <<= 4;
					if (
						(codepoint >= (Utf8::UnicodeCodePoint)'0') &&
						(codepoint <= (Utf8::UnicodeCodePoint)'9')
					) {
						cpFromHexDigit += (codepoint - (Utf8::UnicodeCodePoint)'0');
					} else if (
						(codepoint >= (Utf8::UnicodeCodePoint)'A') &&
						(codepoint <= (Utf8::UnicodeCodePoint)'F')
					) {
						cpFromHexDigit += (codepoint - (Utf8::UnicodeCodePoint)'A' + 10);
					} else if (
						(codepoint >= (Utf8::UnicodeCodePoint)'a') &&
						(codepoint <= (Utf8::UnicodeCodePoint)'f')
					) {
						cpFromHexDigit += (codepoint - (Utf8::UnicodeCodePoint)'a' + 10);
					}
					else
					{
						return false;
					}
					if (++state == 6)
					{
						state = 0;
						if (cpFromHexDigit >= 0xD800 && cpFromHexDigit <= 0xDFFF)
						{
							if (firstHalfOfSurrogatePair == 0)
							{
								firstHalfOfSurrogatePair = cpFromHexDigit;
							}
							else
							{
								const auto secondHalfOfSurrogatePair = cpFromHexDigit;
								const auto encoded = encoder.encode({
									((firstHalfOfSurrogatePair - 0xD800) << 10) +
									(secondHalfOfSurrogatePair - 0xDC00) + 0x10000
								});
								output += std::string(encoded.begin(), encoded.end());
								firstHalfOfSurrogatePair = 0;
							}
						}
						else if (firstHalfOfSurrogatePair == 0)
						{
							const auto encoded = encoder.encode({ cpFromHexDigit });
							output += std::string(encoded.begin(), encoded.end());
						}
						else
						{
							return false;
						}
					}
				}
				break;
			}
		}
		switch (state)
		{
		case 1:
			{
				const auto encoded = encoder.encode({ 0x75 });
				output += std::string(encoded.begin(), encoded.end());
			}
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			{
				const auto encoded = encoder.encode(hexDigitsOriginal);
				output += std::string(encoded.begin(), encoded.end());
			}
			break;
		}
		return (state != 1) && ((state < 2) || (state > 5));
	}

	bool decodeString(
		const std::vector<Utf8::UnicodeCodePoint>& codepoints,
		std::vector<Utf8::UnicodeCodePoint> &output
	) {
		size_t state = 0;
		Utf8::UnicodeCodePoint cpFromHexDigit = 0;
		Utf8::UnicodeCodePoint firstHalfOfSurrogatePair = 0;
		std::vector<Utf8::UnicodeCodePoint> hexDigitsOriginal;
		for (const auto codepoint: codepoints)
		{
			switch (state)
			{
			case 0: /// Initial State
				{
					if (codepoint == 0x5C)
						state = 1;
					else if (firstHalfOfSurrogatePair == 0)
						output.push_back(codepoint);
					else
						return false;
				}
				break;

			case 1: /// Escape character
				{
					if (codepoint == 0x75)
					{
						state = 2;
						cpFromHexDigit = 0;
						hexDigitsOriginal = {0x5C, 0x75};
					}
					else if (firstHalfOfSurrogatePair == 0)
					{
						const auto entry = SPECIAL_ESCAPE_DECODINGS.find(codepoint);
						if (entry == SPECIAL_ESCAPE_DECODINGS.end())
							return false;
						output.push_back(entry->second);
						state = 0;
					}
					else
					{
						return false;
					}
				}
				break;
			
			case 2: // First HexDigit
			case 3: // Second HexDigit
			case 4: // Third HexDigit
			case 5: // Fourth HexDigit
				{
					hexDigitsOriginal.push_back(codepoint);
					cpFromHexDigit <<= 4;
					if (
						(codepoint >= (Utf8::UnicodeCodePoint)'0') &&
						(codepoint <= (Utf8::UnicodeCodePoint)'9')
					) {
						cpFromHexDigit += (codepoint - (Utf8::UnicodeCodePoint)'0');
					} else if (
						(codepoint >= (Utf8::UnicodeCodePoint)'A') &&
						(codepoint <= (Utf8::UnicodeCodePoint)'F')
					) {
						cpFromHexDigit += (codepoint - (Utf8::UnicodeCodePoint)'A' + 10);
					} else if (
						(codepoint >= (Utf8::UnicodeCodePoint)'a') &&
						(codepoint <= (Utf8::UnicodeCodePoint)'f')
					) {
						cpFromHexDigit += (codepoint - (Utf8::UnicodeCodePoint)'a' + 10);
					}
					else
					{
						return false;
					}
					if (++state == 6)
					{
						state = 0;
						if (cpFromHexDigit >= 0xD800 && cpFromHexDigit <= 0xDFFF)
						{
							if (firstHalfOfSurrogatePair == 0)
							{
								firstHalfOfSurrogatePair = cpFromHexDigit;
							}
							else
							{
								const auto secondHalfOfSurrogatePair = cpFromHexDigit;
								output.push_back(((firstHalfOfSurrogatePair - 0xD800) << 10) + (secondHalfOfSurrogatePair - 0xDC00) + 0x10000);
								firstHalfOfSurrogatePair = 0;
							}
						}
						else if (firstHalfOfSurrogatePair == 0)
						{
							output.push_back(cpFromHexDigit);
						}
						else
						{
							return false;
						}
					}
				}
				break;
			}

		}
		switch (state)
		{
		case 1:
			{
				output.push_back(0x75);
			}
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			{
				for (const auto hexDigit: hexDigitsOriginal)
					output.push_back(hexDigit);
			}
			break;
		}
		return (state != 1) && ((state < 2) || (state > 5));
	}

	/**
	 * @brief
	 *     compare two arrays holding pointers for JSON values
	 *     if they are equal or not
	 * 
	 * @param[in] lhs
	 *     This the left array containing json values
	 * 
	 * @param[in] rhs
	 *     This the right array containing json values
	 * 
	 * @return
	 *     true if both arrays have the same size and all
	 *     elements on both of them are also equal (deep equal)
	 *     otherwise false.
	 */
	bool compareArrays(
		const std::vector<std::shared_ptr<JSON::JSON>> &lhs,
		const std::vector<std::shared_ptr<JSON::JSON>> &rhs
	) {
		if (lhs.size() != rhs.size())
			return false;
		for (size_t i = 0; i < lhs.size(); ++i)
			if (*lhs[i] != *rhs[i])
				return false;
		return true;
	}

	/**
	 * @brief
	 *     compare two objects holding pointers for JSON values
	 *     if they are equal or not
	 * 
	 * @param[in] lhs
	 *     This the left object containing json values
	 * 
	 * @param[in] rhs
	 *     This the right object containing json values
	 * 
	 * @return
	 *     true if both objects have the same size and all
	 *     elements on both of them are also equal (deep equal)
	 *     otherwise false.
	 */
	bool compareObjects(
		const std::map<std::string, std::shared_ptr<JSON::JSON>> &lhs,
		const std::map<std::string, std::shared_ptr<JSON::JSON>> &rhs
	) {
		std::set<std::string> keys;
		for (const auto &entry: lhs)
			(void)keys.insert(entry.first);
		for (const auto &entry: rhs)
		{
			const auto otherEntry = keys.find(entry.first);
			if (otherEntry == keys.end())
				return false;
			(void)keys.erase(entry.first);
		}
		if (!keys.empty())
			return false;
		for (auto it = lhs.begin(); it != lhs.end(); it++)
		{
			const auto otherEntry = rhs.find(it->first);
			if (*it->second != *otherEntry->second)
				return false;
		}
		return true;
	}
}

namespace JSON
{
	/**
	 * @brief
	 *     This contains all private properties of the
	 *     JSON class.
	 */
	struct JSON::Impl
	{
		// Types

		/**
		 * @brief
		 *     This holds the actual value represented by the
		 *     JSON object. Use the member that matches the type.
		 */
		union
		{
			bool booleanValue;
			std::string *stringValue;
			int integerValue;
			double floatingPointValue;
			std::vector<std::shared_ptr<JSON>> *arrayValues;
			std::map<std::string, std::shared_ptr<JSON>> *objectValues;
		};

		// Properties
		
		/**
		 * @brief
		 *     This indicates the type of the value represented
		 *     by the JSON object.
		 */
		Type type = Type::Invalid;

		/**
		 * @brief
		 *     This is a cache of the encoding of the value.
		 */
		std::string encoding;

		// Methods

		// Lifecycle Management

		~Impl() noexcept
		{
			switch (type)
			{
				case Type::String:
				{
					delete stringValue;
				}
				break;

				case Type::Array:
				{
					delete arrayValues;
				}
				break;

				case Type::Object:
				{
					delete objectValues;
				}
				break;

				default:
					break;
			}
		}
		Impl(const Impl &) noexcept = delete;
		Impl(Impl &&) noexcept;
		Impl &operator=(const Impl &) noexcept = delete;
		Impl &operator=(Impl &&) noexcept;

		/**
		 * @brief
		 *     This is the default constructor
		 */
		Impl() = default;

		/**
		 * @brief
		 *     This method build the json value up as a copy
		 *     of another json value 
		 * 
		 * @param[in] otherElement
		 *     This is the other json value to copy
		 */
		void copyFrom(const std::unique_ptr<Impl>& otherElement)
		{
			type = otherElement->type;
			switch (type)
			{
			case Type::Boolean:
				{
					booleanValue = otherElement->booleanValue;
				}
				break;

			case Type::Integer:
				{
					integerValue = otherElement->integerValue;
				}
				break;

			case Type::FloatingPoint:
				{
					floatingPointValue = otherElement->floatingPointValue;
				}
				break;

			case Type::String:
				{
					stringValue = new std::string(*otherElement->stringValue);
				}
				break;

			case Type::Array:
				{
					arrayValues = new std::vector<std::shared_ptr<JSON>>;
					arrayValues->reserve(otherElement->arrayValues->size());
					for (const auto &val: *otherElement->arrayValues)
					{
						const auto copy = std::make_shared<JSON>(*val);
						arrayValues->push_back(copy);
					}
				}
				break;

			case Type::Object:
				{
					objectValues = new std::map<std::string, std::shared_ptr<JSON>>;
					for (const auto &val: *otherElement->objectValues)
					{
						const auto copy = std::make_shared<JSON>(*val.second);
						(*objectValues)[val.first] = copy;
					}
				}
				break;
			
			default:
				break;
			}
		}

		/**
		 * @brief
		 *     This parses the given string as an integer JSON value.
		 *
		 * @param[in] s
		 *     This is the string we want to parse to integer.
		 */
		void decodeAsInt(const std::vector<Utf8::UnicodeCodePoint> &codepoints) {
			size_t index = 0;
			size_t state = 0;
			bool negative = false;
			int value = 0;
			while (index < codepoints.size())
			{
				switch (state)
				{
					case 0: // [ minus ]
						{
							if (codepoints[index] == 0x2D)
							{
								negative = true;
								++index;
							}
							state = 1;
						}
						break;

					case 1: // integral part
						{
							if (codepoints[index] == 0x30)
							{
								state = 2;
							}
							else if (codepoints[index] >= 0x31 && codepoints[index] <= 0x39)
							{
								state = 3;
								value += (int)(codepoints[index] - 0x30);
							}
							else
								return;
							++index;
						}
						break;

					case 2: // Invalid value for number
						return;
						break;

					case 3: // *DIGIT (1-9)
						{
							const int previousValue = value;
							if (codepoints[index] >= 0x31 && codepoints[index] <= 0x39)
							{
								value *= 10;
								value += (int)(codepoints[index] - 0x30);
								if (value / 10 != previousValue)
								{
									return;
								}
							}
							else
							{
								return;
							}
							index++;
						}
						break;
				}
			}
			if (state >= 2)
			{
				type = Type::Integer;
				integerValue = (value * (negative ? -1 : 1));
			}
		}

		/**
		 * @brief
		 *     This parses the given string as a double JSON value.
		 *
		 * @param[in] s
		 *     This is the string we want to parse to float.
		 */
		void decodeAsDouble(const std::vector<Utf8::UnicodeCodePoint> &codepoints)
		{
			size_t index = 0;
			size_t state = 0;
			bool negative = false;
			bool negativeMagnitude = false;
			bool negativeExponent = false;
			double magnitude = 0.0;
			double fraction = 0.0;
			double exponent = 0.0;
			size_t fractionDigits = 0;
			while (index < codepoints.size())
			{
				switch (state)
				{
					case 0: // [ minus ]
						{
							if (codepoints[index] == 0x2D)
							{
								negativeMagnitude = true;
								++index;
							}
							state = 1;
						}
						break;

					case 1: // integral part
						{
							if (codepoints[index] == 0)
							{
								state = 2;
							}
							else if (codepoints[index] >= 0x30 && codepoints[index] <= 0x39)
							{
								state = 3;
								magnitude += (double)(codepoints[index] - 0x30);
							}
							else
								return;
							++index;
						}
						break;

					case 2: // Invalid value for number
						return;
						break;

					case 3: // *DIGIT (1-9)
						{
							if (codepoints[index] >= 0x30 && codepoints[index] <= 0x39)
							{
								const auto previousMagnitude = (intmax_t)magnitude;
								magnitude *= 10.0;
								magnitude += (double)(codepoints[index] - '0');
								if (previousMagnitude != (intmax_t)magnitude / 10)
									return;
							}
							else if (codepoints[index] == 0x2E)
							{
								state = 4;
							}
							else if (codepoints[index] == 0x65 || codepoints[index] == 0x45)
							{
								state = 6;
							}
							else
							{
								return;
							}
							index++;
						}
						break;
					case 4: // Fractional part: consists of one digit (0-9)
						{
							if (codepoints[index] >= 0x30 && codepoints[index] <= 0x39)
							{
								++fractionDigits;
								fraction += (double)(codepoints[index] - 0x30) * pow(10.0, -(double)fractionDigits);
							}
							else
							{
								return;
							}
							state = 5;
							++index;
						}
						break;
					case 5: // Fractional part: consists of optional digits (0-9) or e or E
						{
							if (codepoints[index] >= 0x30 && codepoints[index] <= 0x39)
							{
								++fractionDigits;
								fraction += (double)(codepoints[index] - 0x30) * pow(10.0, -(double)fractionDigits);
							}
							else if (codepoints[index] == 0x65 || codepoints[index] == 0x45)
							{
								state = 6;
							}
							else
							{
								return;
							}
							++index;
						}
						break;
					case 6: // Exponential part: consists of [plus|minus] 1*DIGIT (0-9)
						{
							if (codepoints[index] == 0x2D)
							{
								negativeExponent = true;
								++index;
							}
							else if (codepoints[index] == 0x2B)
								++index;
							state = 7;
						}
						break;
					case 7: // Exponential part: required DIGIT (0-9)
						{
							state = 8;
						}
						break;
					case 8: // Exponential part: required DIGIT (0-9)
						{
							if (codepoints[index] >= 0x30 && codepoints[index] <= 0x39)
							{
								const auto previousExponent = (intmax_t)exponent;
								exponent *= 10.0;
								exponent += (double)(codepoints[index] - 0x30);
								if (previousExponent != (intmax_t)exponent / 10)
									return;
							}
							else
							{
								return;
							}
							++index;
						}
						break;
				}
			}
			if ((state >= 2) && (state != 4) && (state != 6) && (state != 7))
			{
				type = Type::FloatingPoint;
				floatingPointValue = (
					(magnitude + fraction) *
					pow(10.0, (negativeExponent ? -1.0 : 1.0) * exponent) *
					(negativeMagnitude ? -1.0 : 1.0)
				);
			}
		}

		/**
		 * @brief
		 *     This function extracts the encoding of the next json value
		 *     in the given codepoints, updating the offset to indicate
		 *     where the end of the value is, or the encoding of the value
		 *     was invalid and thus the position is npos.
		 * 
		 * @param[in] codepoints
		 *     The codepoints to extract json value from.
		 * 
		 * @param[in,out] offset
		 *     the position of start of parsing and this will be edited
		 *     to indicate the end of value.
		 * 
		 * @param[in] delimeter
		 *     This is the delimeter that separates json values.
		 * 
		 * @return
		 *     the encoding of the next json value in the given codepoints.
		 * 
		 * @retval {}
		 *     is returned if the encoded value was invalid.
		 */
		std::vector<Utf8::UnicodeCodePoint> parseValue(
			const std::vector<Utf8::UnicodeCodePoint> &codepoints,
			size_t &offset,
			char delimeter
		) {
			std::stack<Utf8::UnicodeCodePoint> expectedDelims;
			std::vector<Utf8::UnicodeCodePoint> encodedValueCodepoints;
			const auto encodingCodepoints = std::vector<Utf8::UnicodeCodePoint>(
				codepoints.begin() + offset,
				codepoints.end()
			);
			if (codepoints.empty())
				return {};
			bool insideString = false;
			for (const auto codepoint: encodingCodepoints)
			{
				encodedValueCodepoints.push_back(codepoint);
				if (
					(!expectedDelims.empty()) &&
					(codepoint == expectedDelims.top())
				) {
					insideString = false;
					expectedDelims.pop();
					continue;
				}
				if (!insideString)
				{
					if (codepoint == (Utf8::UnicodeCodePoint)'\"')
					{
						expectedDelims.push((Utf8::UnicodeCodePoint)'\"');
						insideString = true;
					}
					else if (codepoint == (Utf8::UnicodeCodePoint)'[')
					{
						expectedDelims.push((Utf8::UnicodeCodePoint)']');
					}
					else if (codepoint == (Utf8::UnicodeCodePoint)'{')
					{
						expectedDelims.push((Utf8::UnicodeCodePoint)'}');
					}
					else if (
						codepoint == (Utf8::UnicodeCodePoint)delimeter &&
						expectedDelims.empty()
					) {
						break;
					}
				}
			}
			if (expectedDelims.empty())
			{
				offset += encodedValueCodepoints.size();
				if (encodedValueCodepoints.back() == (Utf8::UnicodeCodePoint)delimeter)
					encodedValueCodepoints.pop_back();
				return encodedValueCodepoints;
			}
			return {};
		}

		void parseAsObject(const std::vector<Utf8::UnicodeCodePoint> &codepoints)
		{
			size_t offset = 0;
			std::map<std::string, std::shared_ptr<JSON>> newObjectValues;
			while (offset < codepoints.size())
			{
				const auto encodedKey = parseValue(codepoints, offset, ':');
				if (encodedKey.empty())
					return;
				const auto key = std::make_shared<JSON>(FromString(encodedKey));
				if (key->getType() != Type::String)
					return;
				const auto encodedValue = parseValue(codepoints, offset, ',');
				if (encodedKey.empty())
					return;
				const auto value = std::make_shared<JSON>(FromString(encodedValue));
				newObjectValues[(std::string)*key] = value;
			}
			type = Type::Object;
			objectValues = new decltype(newObjectValues)(newObjectValues);
		}

		void parseAsArray(const std::vector<Utf8::UnicodeCodePoint> &codepoints)
		{
			size_t offset = 0;
			std::vector<std::shared_ptr<JSON>> newArrValues;
			while (offset < codepoints.size())
			{
				const auto encodedValue = parseValue(codepoints, offset, ',');
				if (encodedValue.empty())
					return;
				const auto value = std::make_shared<JSON>(FromString(encodedValue));
				newArrValues.push_back(value);
			}
			type = Type::Array;
			arrayValues = new decltype(newArrValues)(newArrValues);
		}
	};

	JSON::~JSON() = default;

	JSON::JSON(const JSON &other) noexcept
		: impl_(new Impl())
	{
		if (this != &other)
		{
			impl_->copyFrom(other.impl_);
		}
	}
	
	JSON::JSON(JSON &&) noexcept = default;
	
	JSON& JSON::operator=(JSON &&) noexcept = default;
	
	JSON &JSON::operator=(const JSON &other) noexcept
	{
		if (this != &other)
		{
			impl_.reset(new Impl());
			impl_->copyFrom(other.impl_);
		}
		return *this;
	}

	bool JSON::operator==(const JSON &other) const
	{
		if (impl_->type != other.impl_->type)
			return false;
		else switch (impl_->type)
		{
			case Type::Invalid:
				return true;
			case Type::Null:
				return true;
			case Type::Boolean:
				return impl_->booleanValue == other.impl_->booleanValue;
			case Type::String:
				return *impl_->stringValue == *other.impl_->stringValue;
			case Type::Integer:
				return impl_->integerValue == other.impl_->integerValue;
			case Type::FloatingPoint:
				return impl_->floatingPointValue == other.impl_->floatingPointValue;
			case Type::Array:
				return compareArrays(*impl_->arrayValues, *other.impl_->arrayValues);
			case Type::Object:
				return compareObjects(*impl_->objectValues, *other.impl_->objectValues);
			default:
				return false;
		}
	}

	bool JSON::operator!=(const JSON &other) const
	{
		return !(*this == other);
	}

	JSON::operator bool() const
	{
		if (impl_->type == Type::Boolean)
			return impl_->booleanValue;
		else
			return false;
	}

	JSON::operator std::string() const
	{
		if (impl_->type == Type::String)
			return (*impl_->stringValue);
		else
			return ("");
	}

	JSON::operator int() const
	{
		if (impl_->type == Type::Integer)
			return impl_->integerValue;
		else if (impl_->type == Type::FloatingPoint)
			return (int)impl_->floatingPointValue;
		else
			return 0;
	}

	JSON::operator double() const
	{
		if (impl_->type == Type::FloatingPoint)
			return (impl_->floatingPointValue);
		else if (impl_->type == Type::Integer)
			return (double)impl_->integerValue;
		else
			return (0.0);
	}

	std::shared_ptr<JSON> JSON::operator[](size_t index) const
	{
		if (impl_->type == Type::Array && index < impl_->arrayValues->size())
			return (*impl_->arrayValues)[index];
		return nullptr;
	}

	std::shared_ptr<JSON> JSON::operator[](int index) const
	{
		return (*this)[(size_t)index];
	}

	std::shared_ptr<JSON> JSON::operator[](const std::string &key) const
	{
		if (impl_->type == Type::Object)
		{
			const auto entry = impl_->objectValues->find(key);
			if (entry == impl_->objectValues->end())
			{
				return nullptr;
			}
			return entry->second;
		}
		return nullptr;
	}

	std::shared_ptr<JSON> JSON::operator[](const char *s) const
	{
		return (*this)[(std::string)s];
	}

	JSON::JSON(Type type)
		: impl_(new Impl())
	{
		impl_->type = type;
		switch (type)
		{
			case Type::String:
				impl_->stringValue = new std::string();
				break;

			case Type::Array:
				impl_->arrayValues = new std::vector<std::shared_ptr<JSON>>();
				break;

			case Type::Object:
				impl_->objectValues = new std::map<std::string, std::shared_ptr<JSON>>();
				break;

			default:
				break;
		}
	}

	JSON::JSON(nullptr_t)
		: impl_(new Impl())
	{
		impl_->type = Type::Null;
	}

	JSON::JSON(bool value)
		: impl_(new Impl())
	{
		impl_->type = Type::Boolean;
		impl_->booleanValue = value;
	}

	JSON::JSON(const std::string &value)
		: impl_(new Impl())
	{
		impl_->type = Type::String;
		impl_->stringValue = new std::string(value);
	}

	JSON::JSON(const char* value)
		: impl_(new Impl())
	{
		impl_->type = Type::String;
		impl_->stringValue = new std::string(value);
	}

	JSON::JSON(int value)
		: impl_(new Impl())
	{
		impl_->type = Type::Integer;
		impl_->integerValue = value;
	}

	JSON::JSON(double value)
		: impl_(new Impl())
	{
		impl_->type = Type::FloatingPoint;
		impl_->floatingPointValue = value;
	}

	JSON::JSON(std::initializer_list<const JSON> args)
		: impl_(new Impl())
	{
		impl_->type = Type::Array;
		impl_->arrayValues = new std::vector<std::shared_ptr<JSON>>(args.size());
		size_t index = 0;
		for (auto arg = args.begin(); arg != args.end(); arg++, index++)
		{
			(*impl_->arrayValues)[index] = std::make_shared<JSON>(*arg);
		}
	}

	auto JSON::getType() const -> Type
	{
		return impl_->type;
	}

	size_t JSON::getSize() const
	{
		if (impl_->type == Type::Array)
			return impl_->arrayValues->size();
		else if (impl_->type == Type::Object)
			return impl_->objectValues->size();
		return 0;
	}

	bool JSON::has(const std::string &key) const
	{
		if (impl_->type == Type::Object)
			return impl_->objectValues->find(key) != impl_->objectValues->end();
		return false;
	}

	std::string JSON::ToString(const EncodingOptions &options) const
	{
		if (impl_->type == Type::Invalid)
		{
			return StringExtensions::sprintf(
				"(Invalid JSON: %s)",
				impl_->encoding.c_str());
		}
		if (options.deleteCache)
		{
			impl_->encoding.clear();
		}
		if (impl_->encoding.empty())
		{
			switch (impl_->type)
			{
				case Type::Null:
					{
						impl_->encoding = "null";
					}
					break;
				case Type::Boolean:
					{
						impl_->encoding = impl_->booleanValue ? "true" : "false";
					}
					break;
				case Type::String:
					{
						impl_->encoding = (
							"\""
							+ escape(*impl_->stringValue, options)
							+ "\""
						);
					}
					break;
				case Type::Integer:
					{
						impl_->encoding = StringExtensions::sprintf("%d", impl_->integerValue);
					}
					break;
				case Type::FloatingPoint:
					{
						impl_->encoding = StringExtensions::sprintf("%lg", impl_->floatingPointValue);
					}
					break;
				case Type::Array:
					{
						impl_->encoding = '[';
						bool isFirst = true;
						auto nestedOptions = options;
						nestedOptions.depth++;
						std::string nestedIndentation(
							(
								nestedOptions.depth *
								nestedOptions.spacePerIndentation),
							' '
						);
						std::string wrappedEncoding = "[\r\n";
						for (const auto value: *impl_->arrayValues)
						{
							if (isFirst)
							{
								isFirst = false;
							}
							else
							{
								impl_->encoding += ',';
								wrappedEncoding += ",\r\n";
							}
							const auto encodedValue = value->ToString(nestedOptions);
							impl_->encoding += encodedValue;
							wrappedEncoding += nestedIndentation;
							wrappedEncoding += encodedValue;
						}
						impl_->encoding += ']';
						wrappedEncoding += "\r\n";
						std::string indentation(options.depth * options.spacePerIndentation, ' ');
						wrappedEncoding += indentation;
						wrappedEncoding += "]";
						if (options.prettyPrint)
							impl_->encoding = wrappedEncoding;
					}
					break;
				case Type::Object:
					{
						impl_->encoding = '{';
						bool isFirst = true;
						auto nestedOptions = options;
						nestedOptions.depth++;
						std::string nestedIndentation(
							(
								nestedOptions.depth *
								nestedOptions.spacePerIndentation),
							' '
						);
						std::string wrappedEncoding = "{\r\n";
						for (const auto& entry: *impl_->objectValues)
						{
							if (isFirst)
							{
								isFirst = false;
							}
							else
							{
								impl_->encoding += ',';
								wrappedEncoding += ",\r\n";
							}
							const JSON jsonKey(entry.first);
							const auto encodedValue = (
								jsonKey.ToString(nestedOptions)
								+ (nestedOptions.prettyPrint ? ": " : ":")
								+ entry.second->ToString(nestedOptions)
							);

							impl_->encoding += encodedValue;
							wrappedEncoding += nestedIndentation;
							wrappedEncoding += encodedValue;
						}
						impl_->encoding += '}';
						wrappedEncoding += "\r\n";
						std::string indentation(options.depth * options.spacePerIndentation, ' ');
						wrappedEncoding += indentation;
						wrappedEncoding += "}";
						if (options.prettyPrint)
							impl_->encoding = wrappedEncoding;
					}
					break;
				default:
					impl_->encoding = "????";
			}
		}
		return impl_->encoding;
	}

	JSON JSON::FromString(const std::vector<Utf8::UnicodeCodePoint> &codepoints)
	{
		JSON json;
		Utf8::Utf8 encoder;

		const auto firstNonWhitespaceChar = findFirstNotOf(codepoints, WHITESPACES, true);
		if (firstNonWhitespaceChar == codepoints.size())
			return json;
		const auto lastNonWhitespaceChar = findFirstNotOf(codepoints, WHITESPACES, false);

		const auto remainingCodepoints = std::vector<Utf8::UnicodeCodePoint>(
			codepoints.begin() + firstNonWhitespaceChar,
			codepoints.begin() + lastNonWhitespaceChar
		);

		std::vector<uint8_t> encodedUtf8 = encoder.encode(remainingCodepoints);

		json.impl_->encoding = std::string(
			encodedUtf8.begin(),
			encodedUtf8.end()
		);

		if (encodedUtf8.empty())
			return json;
		else if (
			encodedUtf8[0] == (Utf8::UnicodeCodePoint)'{' &&
			encodedUtf8[encodedUtf8.size() - 1] == (Utf8::UnicodeCodePoint)'}'
		) {
			json.impl_->parseAsObject(std::vector<Utf8::UnicodeCodePoint>(
				remainingCodepoints.begin() + 1,
				remainingCodepoints.begin() + remainingCodepoints.size() - 1
			));
		}
		else if (
			encodedUtf8[0] == (Utf8::UnicodeCodePoint)'[' && 
			encodedUtf8[encodedUtf8.size() - 1] == (Utf8::UnicodeCodePoint)']'
		) {
			json.impl_->parseAsArray(std::vector<Utf8::UnicodeCodePoint>(
				remainingCodepoints.begin() + 1,
				remainingCodepoints.begin() + remainingCodepoints.size() - 1
			));
		}
		else if (
			encodedUtf8[0] == (Utf8::UnicodeCodePoint)'"' &&
			encodedUtf8[encodedUtf8.size() - 1] == (Utf8::UnicodeCodePoint)'"'
		) {
			std::vector<Utf8::UnicodeCodePoint> output;
			if (
				decodeString(
					std::vector<Utf8::UnicodeCodePoint>(
						remainingCodepoints.begin() + 1,
						remainingCodepoints.begin() + remainingCodepoints.size() - 1
					),
					output
				)
			) {
				json.impl_->type = Type::String;
				const auto encodedOutput = encoder.encode(output);
				json.impl_->stringValue = new std::string(encodedOutput.begin(), encodedOutput.end());
			}
		}
		else if (json.impl_->encoding == "null")
			json.impl_->type = Type::Null;
		else if (json.impl_->encoding == "true")
		{
			json.impl_->type = Type::Boolean;
			json.impl_->booleanValue = true;
		}
		else if (json.impl_->encoding == "false")
		{
			json.impl_->type = Type::Boolean;
			json.impl_->booleanValue = false;
		}
		else
		{
			if (findFirstOf(remainingCodepoints, FLOAT_INDICATORS, true) != remainingCodepoints.size())
				json.impl_->decodeAsDouble(remainingCodepoints);
			else
				json.impl_->decodeAsInt(remainingCodepoints);
		}
		return json;
	}

	JSON JSON::FromString(const std::string &formatBeforeTrim)
	{
		Utf8::Utf8 decoder;
		return FromString(decoder.decode(formatBeforeTrim));
	}

	void JSON::add(const JSON &value)
	{
		if (impl_->type != Type::Array)
			return;
		insert(std::move(value), impl_->arrayValues->size());
		impl_->encoding.clear();
	}

	void JSON::insert(const JSON &value, size_t index)
	{
		if (impl_->type != Type::Array)
			return;
		(void)impl_->arrayValues->insert(
			impl_->arrayValues->begin() + std::min(
				index,
				impl_->arrayValues->size()
			),
			std::make_shared<JSON>(std::move(value))
		);
		impl_->encoding.clear();
	}

	void JSON::remove(size_t index)
	{
		if (impl_->type != Type::Array)
			return;
		if (index < impl_->arrayValues->size())
		{
			impl_->arrayValues->erase(
				impl_->arrayValues->begin() + index
			);
			impl_->encoding.clear();
		}
	}

	void JSON::remove(const std::string &key)
	{
		if (impl_->type != Type::Object)
			return;
		(void)impl_->objectValues->erase(key);
		impl_->encoding.clear();
	}

	void JSON::set(const std::string &key, const JSON &value)
	{
		if (impl_->type != Type::Object)
			return;
		(*impl_->objectValues)[key] = std::make_shared<JSON>(std::move(value));
		impl_->encoding.clear();
	}

	void PrintTo(
		const JSON &json,
		std::ostream *os
	) {
		*os << json.ToString();
	}

	void PrintTo(
		const JSON::Type &type,
		std::ostream *os)
	{
		switch (type)
		{
			case JSON::Type::Array:
				*os << "ARRAY";
				break;
			case JSON::Type::Boolean:
				*os << "BOOLEAN";
				break;
			case JSON::Type::FloatingPoint:
				*os << "DOUBLE";
				break;
			case JSON::Type::Integer:
				*os << "INTEGER";
				break;
			case JSON::Type::Invalid :
				*os << "INVALID";
				break;
			case JSON::Type::Null:
				*os << "NULL";
				break;
			case JSON::Type::String :
				*os << "STRING";
				break;
			default:
				*os << "???";
		}
	}
} // namespace JSON
