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
#include <Utf8/Utf8.hpp>
#include <map>
#include <vector>
#include <math.h>

namespace
{
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
	 *     This parses the given string as an integer value.
	 *
	 * @param[in] s
	 *     This is the string to be parsed.
	 *
	 * @return
	 *     The integer value that is equivalent to given string.
	 */
	JSON::JSON parseInt(const std::string &s)
	{
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
							return JSON::JSON();
						++index;
					}
					break;

				case 2: // Invalid value for number
					return JSON::JSON();
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
								return JSON::JSON();
							}
						}
						else
						{
							return JSON::JSON();
						}
						index++;
					}
					break;
			}
		}
		if (state < 2)
			return JSON::JSON();
		else
			return (JSON::JSON(value * (negative ? -1 : 1)));
	}

	/**
	 * @brief
	 *     This parses the given string as a double value.
	 *
	 * @param[in] s
	 *     This is the string to be parsed.
	 *
	 * @return
	 *     The double value that is equivalent to given string.
	 */
	JSON::JSON parseFloat(const std::string &s)
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
							return JSON::JSON();
						++index;
					}
					break;

				case 2: // Invalid value for number
					return JSON::JSON();
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
							return JSON::JSON();
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
							return JSON::JSON();
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
							return JSON::JSON();
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
							return JSON::JSON();
						}
						++index;
					}
					break;
			}
		}
		if ((state < 2) || (state == 4) || (state == 6) || (state == 7))
			return JSON::JSON();
		else
			return JSON::JSON(
				(magnitude + fraction) *
				pow(10.0, (negativeExponent ? -1.0 : 1.0) * exponent) *
				(negativeMagnitude ? -1.0 : 1.0)
			);
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
				output += "\\u";
				output += codepointToFourHexDigits(codepoint);
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
	 * @return
	 *     The unescaped version of the given string.
	 */
	std::string unescape(
		const std::string &s
	) {
		Utf8::Utf8 decoder, encoder;
		std::string output;
		size_t state = 0;
		Utf8::UnicodeCodePoint cpFromHexDigit = 0;
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
					else
					{
						const auto encoded = encoder.encode({ codepoint });
						output += std::string(encoded.begin(), encoded.end());
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
					else
					{
						Utf8::UnicodeCodePoint alternative = codepoint;
						const auto entry = SPECIAL_ESCAPE_DECODINGS.find(codepoint);
						if (entry == SPECIAL_ESCAPE_DECODINGS.end())
						{
							const auto encoded = encoder.encode({ 0x5C, codepoint });
							output += std::string(encoded.begin(), encoded.end());
						}
						else
						{
							const auto encoded = encoder.encode({ entry->second });
							output += std::string(encoded.begin(), encoded.end());
						}
						state = 0;
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
						state = 0;
						const auto encoded = encoder.encode(hexDigitsOriginal);
						output += std::string(encoded.begin(), encoded.end());
						break;
					}
					if (++state == 6)
					{
						state = 0;
						const auto encoded = encoder.encode({ cpFromHexDigit });
						output += std::string(encoded.begin(), encoded.end());
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
		return output;
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
		 *     These are the different kinds of values that
		 *     a JSON object can be.
		 */
		enum class Type
		{
			Null,
			Boolean,
			String,
			Integer,
			FloatingPoint
		};

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
		};
		

		// Properties
		
		/**
		 * @brief
		 *     This indicates the type of the value represented
		 *     by the JSON object.
		 */
		Type type = Type::Null;

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
			case Impl::Type::Null:
				return true;
			case Impl::Type::Boolean:
				return impl_->booleanValue == other.impl_->booleanValue;
			case Impl::Type::String:
				return *impl_->stringValue == *other.impl_->stringValue;
			case Impl::Type::Integer:
				return impl_->integerValue == other.impl_->integerValue;
			case Impl::Type::FloatingPoint:
				return impl_->floatingPointValue == other.impl_->floatingPointValue;
			default:
				return false;
		}
	}

	JSON::operator bool() const
	{
		if (impl_->type == Impl::Type::Boolean)
			return impl_->booleanValue;
		else
			return false;
	}

	JSON::operator std::string() const
	{
		if (impl_->type == Impl::Type::String)
			return (*impl_->stringValue);
		else
			return ("");
	}

	JSON::operator int() const
	{
		if (impl_->type == Impl::Type::Integer)
			return impl_->integerValue;
		else if (impl_->type == Impl::Type::FloatingPoint)
			return (int)impl_->floatingPointValue;
		else
			return 0;
	}

	JSON::operator double() const
	{
		if (impl_->type == Impl::Type::FloatingPoint)
			return (impl_->floatingPointValue);
		else if (impl_->type == Impl::Type::Integer)
			return (double)impl_->integerValue;
		else
			return (0.0);
	}

	JSON::JSON()
		: impl_(new Impl())
	{}

	JSON::JSON(nullptr_t)
		: impl_(new Impl())
	{
		impl_->type = Impl::Type::Null;
	}

	JSON::JSON(bool value)
		: impl_(new Impl())
	{
		impl_->type = Impl::Type::Boolean;
		impl_->booleanValue = value;
	}

	JSON::JSON(const std::string &value)
		: impl_(new Impl())
	{
		impl_->type = Impl::Type::String;
		impl_->stringValue = new std::string(value);
	}

	JSON::JSON(const char* value)
		: impl_(new Impl())
	{
		impl_->type = Impl::Type::String;
		impl_->stringValue = new std::string(value);
	}

	JSON::JSON(int value)
		: impl_(new Impl())
	{
		impl_->type = Impl::Type::Integer;
		impl_->integerValue = value;
	}

	JSON::JSON(double value)
		: impl_(new Impl())
	{
		impl_->type = Impl::Type::FloatingPoint;
		impl_->floatingPointValue = value;
	}

	std::string JSON::ToString(const EncodingOptions &options) const
	{
		switch (impl_->type)
		{
			case Impl::Type::Null:
				return "null";
			case Impl::Type::Boolean:
				return impl_->booleanValue ? "true" : "false";
			case Impl::Type::String:
				return (
					"\""
					+ escape(*impl_->stringValue, options)
					+ "\""
				);
			case Impl::Type::Integer:
				return StringExtensions::sprintf("%d", impl_->integerValue);
			case Impl::Type::FloatingPoint:
				return StringExtensions::sprintf("%lg", impl_->floatingPointValue);
			default:
				return "";
		}
	}

	JSON JSON::FromString(const std::string &format)
	{
		if (format.empty())
			return JSON();
		else if (format[0] == '{')
			return JSON();
		else if (format[0] == '[')
			return JSON();
		else if (format[0] == '"' && format[format.length() - 1] == '"')
			return unescape(format.substr(1, format.length() - 2));
		else if (format == "null")
			return nullptr;
		else if (format == "true")
			return true;
		else if (format == "false")
			return false;
		else
		{
			if (format.find_first_of(".eE") != std::string::npos)
			{
				// TODO: Parse as floating point number
				return JSON(parseFloat(format));
			}
			else
			{
				// TODO: Parse as integer number
				return JSON(parseInt(format));
			}
		}
	}
} // namespace JSON
