#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <memory>

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
		JSON(JSON &&) noexcept = delete;
		JSON &operator=(const JSON &) noexcept = delete;
		JSON &operator=(JSON &&) noexcept = delete;

		// Public Methods
		
		/**
		 * @brief
		 *     Construct a new JSON object
		 */
		JSON();

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