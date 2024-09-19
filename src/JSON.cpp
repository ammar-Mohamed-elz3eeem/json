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
			default:
				return true;
		}
	}


	JSON::JSON(): impl_(new Impl())
	{}

} // namespace JSON
