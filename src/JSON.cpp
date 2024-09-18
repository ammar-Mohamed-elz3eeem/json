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

namespace JSON
{
	/**
	 * @brief
	 *     This contains all private properties of the
	 *     JSON class.
	 */
	struct JSON::Impl
	{
		// Properties

		// Methods
	};

	JSON::~JSON() = default;

	JSON::JSON(): impl_(new Impl())
	{}

} // namespace JSON
