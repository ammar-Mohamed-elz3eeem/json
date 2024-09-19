#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <memory>
#include <string>
#include <stdint.h>
#include <StringExtensions/StringExtensions.hpp>

/**
 * @file: JSON.hpp
 * @author: Ammar Massoud (ammar@wpkama.com)
 * 
 * @brief
 *     This file declares class JSON::JSON
 * 
 * @version: 1.0.0
 * @date: 2024-07-28
 * @copyright: Copyright (c) 2024
 */

namespace JSON
{

	/**
	 * @brief
	 *     This is used to configure the various options
	 *     having to do with encoding a JSON object into
	 *     its string format.
	 */
	struct EncodingOptions
	{
		/**
		 * @brief
		 *     This flag indicates whether or not to escape non-ASCII
		 *     characters when encoding the JSON object into its string
		 *     format.
		 */
		bool escapeNonAscii = false;
	};

	/**
	 * @brief
	 *     This class represents a data structure that was parsed from
	 *     or can be rendered to, a string in the JavaScript Object
	 *     Notation (JSON) Data Interchange Format, as specified in
	 *     RFC 7159 (https://www.rfc-editor.org/rfc/rfc7159)
	 */
	class JSON
	{
	public:
		// Lifecycle Management

		~JSON() noexcept;
		JSON(const JSON &) noexcept = delete;
		JSON(JSON &&) noexcept;
		JSON &operator=(const JSON &) noexcept = delete;
		JSON &operator=(JSON &&) noexcept;

		// Public Methods

		/**
		 * @brief
		 *     This is the equality comparison operator.
		 * 
		 * @param[in] other
		 *     This is the other object we want to compare
		 *     with this object.
		 * 
		 * @return true if both JSON objects are equal.
		 */
		bool operator==(const JSON &other) const;

		/**
		 * @brief
		 *     This is the typecast to bool operator for
		 *     the class.
		 * 
		 * @return
		 *      true if the JSON object is a boolean and its value
		 *      is true, otherwise false.
		 * 
		 * @retval
		 *     false is the json object is not a boolean, or it's
		 *     a boolean and its value is false. 
		 */
		operator bool() const;

		/**
		 * @brief
		 *     This is the typecast to C++ string operator for
		 *     the class.
		 * 
		 * @return
		 *      The c++ string equivalent of the JSON object.
		 * 
		 * @retval std::string("")
		 *     is returned if the json object is not a string,
		 *     or it is a string and it's value is the empty
		 *     string
		 */
		operator std::string() const;

		/**
		 * @brief
		 *     This is the typecast to integer operator for
		 *     the class.
		 * 
		 * @return
		 *      The integer value equivalent of the JSON object.
		 * 
		 * @retval 0
		 *     is returned if the json object is not an integer,
		 *     or it is an integer and it's value is zero.
		 */
		operator int() const;

		/**
		 * @brief
		 *     This is the typecast to floating-point operator for
		 *     the class.
		 *
		 * @return
		 *      The floating-point value equivalent of the JSON object.
		 *
		 * @retval 0.0
		 *     is returned if the json object is not a floating-point,
		 *     or it's indeed a floating-point number with value 0.0.
		 */
		operator double() const;

		/**
		 * @brief
		 *     Construct a new JSON object
		 */
		JSON();

		/**
		 * @brief
		 *     This constructs a JSON object consisting of
		 *     the "null" literal.
		 * 
		 * @param[in] null
		 *     This is the object to wrap in JSON.
		 */
		JSON(nullptr_t);

		/**
		 * @brief
		 *     This constructs a JSON object consisting of
		 *     the given boolean value.
		 * 
		 * @param[in] value
		 *     This is the object to wrap in JSON.
		 */
		JSON(bool);

		/**
		 * @brief
		 *     This constructs a JSON object consisting of
		 *     the given string value.
		 * 
		 * @param[in] value
		 *     This is the object to wrap in JSON.
		 */
		JSON(const std::string &);

		/**
		 * @brief
		 *     This constructs a JSON object consisting of
		 *     the given C String value.
		 * 
		 * @param[in] value
		 *     This is the object to wrap in JSON.
		 */
		JSON(const char *);

		/**
		 * @brief
		 *     This constructs a JSON object consisting of
		 *     the given integer value.
		 * 
		 * @param[in] value
		 *     This is the object to wrap in JSON.
		 */
		JSON(int);

		/**
		 * @brief
		 *     This constructs a JSON object consisting of
		 *     the given floating point value.
		 * 
		 * @param[in] value
		 *     This is the object to wrap in JSON.
		 */
		JSON(double);

		/**
		 * @brief
		 *     This encodes the JSON object into its string
		 *     format
		 * 
		 * @param[in] options
		 *     This is used to configure the various options
		 *     having to do with encoding a JSON object into
		 *     its string format.
		 * 
		 * @return
		 *     The string format of the JSON object
		 */
		std::string ToString(const EncodingOptions& options = EncodingOptions()) const;

		/**
		 * @brief
		 *     This method returns a new JSON object constructed
		 *     by parsing the JSON object from the given format
		 *     string.
		 * 
		 * @param[in] format
		 *     This is the string format of the JSON object to
		 *     construct.
		 * 
		 * @return
		 *     new JSON object constructed by parsing the JSON
		 *     object from the given format string.
		 */
		static JSON FromString(const std::string &);

	private:
		// Private Properties

		/**
		 * @brief
		 *     structure to hold all the private properties of the
		 *     JSON instance.
		 */
		struct Impl;

		/**
		 * @brief
		 *     This contains all private properties of the JSON
		 *     class.
		 */
		std::unique_ptr<struct Impl> impl_;
	};
} // namespace JSON

#endif /* __JSON_HPP__ */