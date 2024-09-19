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
