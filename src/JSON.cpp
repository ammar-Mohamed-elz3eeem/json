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
			default:
				return true;
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

} // namespace JSON
