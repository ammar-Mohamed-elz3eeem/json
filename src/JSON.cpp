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

namespace
{
	constexpr const char *WHITESPACES = " \t\r\n";

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
		 *     This parses the given string as an integer JSON value.
		 *
		 * @param[in] s
		 *     This is the string we want to parse to integer.
		 */
		void parseAsInt(const std::string &s) {
			size_t index = 0;
			size_t state = 0;
			bool negative = false;
			int value = 0;
			while (index < s.length())
			{
				switch (state)
				{
					case 0: // [ minus ]
						{
							if (s[index] == '-')
							{
								negative = true;
								++index;
							}
							state = 1;
						}
						break;

					case 1: // integral part
						{
							if (s[index] == 0)
							{
								state = 2;
							}
							else if (s[index] >= '1' && s[index] <= '9')
							{
								state = 3;
								value += (int)(s[index] - '0');
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
							if (s[index] >= '1' && s[index] <= '9')
							{
								value *= 10;
								value += (int)(s[index] - '0');
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
		void parseAsFloat(const std::string &s)
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
			while (index < s.length())
			{
				switch (state)
				{
					case 0: // [ minus ]
						{
							if (s[index] == '-')
							{
								negativeMagnitude = true;
								++index;
							}
							state = 1;
						}
						break;

					case 1: // integral part
						{
							if (s[index] == 0)
							{
								state = 2;
							}
							else if (s[index] >= '0' && s[index] <= '9')
							{
								state = 3;
								magnitude += (double)(s[index] - '0');
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
							if (s[index] >= '0' && s[index] <= '9')
							{
								magnitude *= 10.0;
								magnitude += (double)(s[index] - '0');
							}
							else if (s[index] == '.')
							{
								state = 4;
							}
							else if (s[index] == 'e' || s[index] == 'E')
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
							if (s[index] >= '0' && s[index] <= '9')
							{
								++fractionDigits;
								fraction += (double)(s[index] - '0') * pow(10.0, -(double)fractionDigits);
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
							if (s[index] >= '0' && s[index] <= '9')
							{
								++fractionDigits;
								fraction += (double)(s[index] - '0') * pow(10.0, -(double)fractionDigits);
							}
							else if (s[index] == 'e' || s[index] == 'E')
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
							if (s[index] == '-')
							{
								negativeExponent = true;
								++index;
							}
							else if (s[index] == '+')
								++index;
							state = 7;
						}
						break;
					case 7: // Exponential part: required DIGIT (0-9)
						{
							state = 8;
						}
						break;
					case 8: // Exponential part: Extra DIGITs (0-9)
						{
							if (s[index] >= '0' && s[index] <= '9')
							{
								exponent *= 10.0;
								exponent += (double)(s[index] - '0');
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
			// else
				// return JSON::JSON(
				// 	(magnitude + fraction) *
				// 	pow(10.0, (negativeExponent ? -1.0 : 1.0) * exponent) *
				// 	(negativeMagnitude ? -1.0 : 1.0)
				// );
		}

		/**
		 * @brief
		 *     This function extracts the encoding of the next JSON
		 *     value in the given string, updating the given offset
		 *     where the end of the value was found, or whether the
		 *     encoding was invalid.
		 *
		 * @param[in] s
		 *     This is the string to parse to extract json value from.
		 *
		 * @param[in,out] offset
		 *     On input, this is the position of the first character
		 *     of the encoded in the given string,
		 *     On output, this is the position of the first character
		 *     past the end the encoded in the given string or
		 *     std::string::npos if the encoded value was invalid.
		 *
		 * @return
		 *     The actual value that will be assigned to json value.
		 */
		std::string parseValue(
			const std::string &s,
			size_t &offset,
			char delimeter
		) {
			Utf8::Utf8 decoder, encoder;
			std::stack<char> expectedDelims;
			std::vector<Utf8::UnicodeCodePoint> encodedValueCodepoints;
			const auto encodingCodepoints = decoder.decode(s.substr(offset));
			if (encodingCodepoints.empty())
			{
				offset = std::string::npos;
				return "";
			}
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
						expectedDelims.push('\"');
						insideString = true;
					}
					else if (codepoint == (Utf8::UnicodeCodePoint)'[')
					{
						expectedDelims.push(']');
					}
					else if (codepoint == (Utf8::UnicodeCodePoint)'{')
					{
						expectedDelims.push('}');
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
				auto encodedValue = encoder.encode(encodedValueCodepoints);
				if (encodedValue.back() == (Utf8::UnicodeCodePoint)delimeter)
				{
					encodedValue.pop_back();
				}
				offset += encodedValueCodepoints.size();
				return std::string(
					encodedValue.begin(),
					encodedValue.end());
			}
			else
			{
				offset = std::string::npos;
				return "";
			}
		}

		/**
		 * @brief
		 *     This parses the given string as an array JSON value.
		 *
		 * @param[in] s
		 *     This is the string we want to parse to array.
		 */
		void parseAsArray(const std::string &s)
		{
			size_t offset = 0;
			std::vector<std::shared_ptr<JSON>> newArrValues;
			while (offset < s.length())
			{
				const auto encodedValue = parseValue(s, offset, ',');
				if (offset == std::string::npos)
					return;
				const auto value = std::make_shared<JSON>(FromString(encodedValue));
				newArrValues.push_back(value);
			}
			type = Type::Array;
			arrayValues = new decltype(newArrValues)(newArrValues);
		}

		/**
		 * @brief
		 *     This parses the given string as an object JSON value.
		 *
		 * @param[in] s
		 *     This is the string we want to parse to object.
		 */
		void parseAsObject(const std::string &s)
		{
			size_t offset = 0;
			std::map<std::string, std::shared_ptr<JSON>> newObjectValues;
			while (offset < s.length())
			{
				const auto encodedKey = parseValue(s, offset, ':');
				if (offset == std::string::npos)
					return;
				const auto key = std::make_shared<JSON>(FromString(encodedKey));
				if (key->getType() != Type::String)
					return;
				const auto encodedValue = parseValue(s, offset, ',');
				if (offset == std::string::npos)
					return;
				const auto value = std::make_shared<JSON>(FromString(encodedValue));
				newObjectValues[(std::string)*key] = value;
			}
			type = Type::Object;
			objectValues = new decltype(newObjectValues)(newObjectValues);
		}
	};

	JSON::~JSON() = default;
	JSON::JSON(JSON &&) noexcept = default;
	JSON& JSON::operator=(JSON &&) noexcept = default;

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
			default:
				return false;
		}
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
					impl_->encoding = "null";
					break;
				case Type::Boolean:
					impl_->encoding = impl_->booleanValue ? "true" : "false";
					break;
				case Type::String:
					impl_->encoding = (
						"\""
						+ escape(*impl_->stringValue, options)
						+ "\""
					);
					break;
				case Type::Integer:
					impl_->encoding = StringExtensions::sprintf("%d", impl_->integerValue);
					break;
				case Type::FloatingPoint:
					impl_->encoding = StringExtensions::sprintf("%lg", impl_->floatingPointValue);
					break;
				default:
					impl_->encoding = "????";
			}
		}
		return impl_->encoding;
	}

	JSON JSON::FromString(const std::string &formatBeforeTrim)
	{
		JSON json;
		const auto firstNonWhitespaceChar = formatBeforeTrim.find_first_not_of(WHITESPACES);
		if (firstNonWhitespaceChar == std::string::npos)
			return json;
		const auto lastNonWhitespaceChar = formatBeforeTrim.find_last_not_of(WHITESPACES);
		const auto format = formatBeforeTrim.substr(
			firstNonWhitespaceChar,
			lastNonWhitespaceChar - firstNonWhitespaceChar + 1
		);
		json.impl_->encoding = format;
		if (format.empty())
		{
			return json;
		}
		else if (format[0] == '{' && format[format.length() - 1] == '}')
			json.impl_->parseAsObject(format.substr(1, format.length() - 2));
		else if (format[0] == '[' && format[format.length() - 1] == ']')
			json.impl_->parseAsArray(format.substr(1, format.length() - 2));
		else if (format[0] == '"' && format[format.length() - 1] == '"')
		{
			std::string output;
			if (unescape(format.substr(1, format.length() - 2), output))
			{
				json.impl_->type = Type::String;
				json.impl_->stringValue = new std::string(output);
			}
		}
		else if (format == "null")
		{
			json.impl_->type = Type::Null;
		}
		else if (format == "true")
		{
			json.impl_->type = Type::Boolean;
			json.impl_->booleanValue = true;
		}
		else if (format == "false")
		{
			json.impl_->type = Type::Boolean;
			json.impl_->booleanValue = false;
		}
		else
		{
			if (format.find_first_of(".eE") != std::string::npos)
			{
				json.impl_->parseAsFloat(format);
			}
			else
			{
				json.impl_->parseAsInt(format);
			}
		}
		return json;
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
