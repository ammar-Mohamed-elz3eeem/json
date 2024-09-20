#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <memory>
#include <string>
#include <stdint.h>
#include <StringExtensions/StringExtensions.hpp>
#include <ostream>


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

		/**
		 * @brief
		 *     This flag indicates whether or not to disregard
		 *     any cached encoding when asked to provide an
		 *     encoding.
		 */
		bool deleteCache = false;
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
		// Types

		/**
		 * @brief
		 *     These are the different kinds of values that
		 *     a JSON object can be.
		 */
		enum class Type
		{
			Invalid,
			Null,
			Boolean,
			String,
			Integer,
			FloatingPoint,
			Array,
			Object
		};

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
		 *     This method returns the element at the given index
		 *     of the json value, if it's an array.
		 *
		 * @param[in] index
		 *     The index of the element in the json array value.
		 *
		 * @return
		 *     the element at the given index
		 *
		 * @retval nullptr
		 *     This value is returned if there is no element at
		 *     the given index, or the json value is not array.
		 */
		std::shared_ptr<JSON> operator[](size_t) const;

		/**
		 * @brief
		 *     This method returns the element at the given index
		 *     of the json value, if it's an array.
		 *
		 * @param[in] index
		 *     The index of the element in the json array value.
		 *
		 * @return
		 *     the element at the given index
		 *
		 * @retval nullptr
		 *     This value is returned if there is no element at
		 *     the given index, or the json value is not array.
		 */
		std::shared_ptr<JSON> operator[](int) const;

		/**
		 * @brief
		 *     This method returns the element associated with
		 *     the given key of the json value, if it's an array.
		 *
		 * @param[in] index
		 *     The index of the element in the json array value.
		 *
		 * @return
		 *     the element at the given index
		 *
		 * @retval nullptr
		 *     This value is returned if there is no element at
		 *     the given index, or the json value is not array.
		 */
		std::shared_ptr<JSON> operator[](const std::string &) const;

		/**
		 * @brief
		 *     This method returns the element associated with
		 *     the given key of the json value, if it's an array.
		 *
		 * @param[in] index
		 *     The index of the element in the json array value.
		 *
		 * @return
		 *     the element at the given index
		 *
		 * @retval nullptr
		 *     This value is returned if there is no element at
		 *     the given index, or the json value is not array.
		 */
		std::shared_ptr<JSON> operator[](const char *) const;

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
		 *     This method returns the type of the json value.
		 *
		 * @return
		 *     type of the json value.
		 */
		Type getType() const;

		/**
		 * @brief
		 *     This method returns the size of the json value
		 *     if only it is an array.
		 *
		 * @return
		 *     size of the json value if only it is an array.
		 *
		 * @retval 0
		 *     is returned if the json value is not an array
		 *     or it is an empty array. 
		 */
		size_t getSize() const;

		/**
		 * @brief
		 *     This method returns an indication of whether or not
		 *     the JSON value is an object with an inner value
		 *     having the given key for a name.
		 *
		 * @param[in] s
		 *     This is the name of the inner value to check.
		 *
		 * @return 
		 *     true if the given key exists in the JSON object value,
		 *     false otherwise. 
		 */
		bool has(const std::string &s) const;

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
		 * @param[in] formatBeforeTrim
		 *     This is the string format of the JSON object to
		 *     construct, it may contain whitespace characters.
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

	/**
	 * @brief
	 *     This is a support function for Google tests
	 *     to print out the actual value of JSON object.
	 *
	 * @param[in] json
	 *     This is the json object we need its value.
	 *
	 * @param[in,out] os
	 *     This points to the stream in which we print the
	 *     json value.
	 */
	void PrintTo(
		const JSON &json,
		std::ostream *os);

	/**
	 * @brief
	 *     This is a support function for Google tests
	 *     to print out the actual value of JSON::Type object.
	 *
	 * @param[in] type
	 *     This is the json object type we need its value.
	 *
	 * @param[in,out] os
	 *     This points to the stream in which we print the
	 *     json type.
	 */
	void PrintTo(
		const JSON::Type &type,
		std::ostream *os);

} // namespace JSON

#endif /* __JSON_HPP__ */