/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000 Bruno Levy
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     levy@loria.fr
 *
 *     ISA Project
 *     LORIA, INRIA Lorraine,
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs.
 */

#ifndef H_OGF_BASIC_TYPES_ANY_H
#define H_OGF_BASIC_TYPES_ANY_H

#include <OGF/gom/common/common.h>
#include <geogram/basic/life_cycle.h>
#include <typeinfo>

/**
 * \file OGF/gom/types/any.h
 * \brief A class to hold a value of arbitrary type.
 */

#ifdef GEO_COMPILER_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif


namespace OGF {

    class MetaType;

    /**
     * \brief A class that stores a variable of arbitrary type.
     */
    class GOM_API Any {
      private:

	/**
	 * \brief The size in bytes for storing values in the buffer
	 *  rather than doing dynamic allocation.
	 */
	enum { BUFFER_SIZE = 40 };

      public:

	/**
	 * \brief Any constructor.
	 */
        Any() : value_(nullptr), in_buffer_(false), meta_type_(nullptr) {
	}

	/**
	 * \brief Any destructor.
	 */
	~Any() {
	    destroy();
	}

	/**
	 * \brief Tests whether this Any is null.
	 * \retval true if the Any does not store a value.
	 * \retval false otherwise.
	 */
	bool is_null() const {
	    return value_ == nullptr;
	}

	/**
	 * \brief Resets this Any to the initial null value.
	 */
	void reset() {
	    destroy();
	}

	/**
	 * \brief Any copy constructor.
	 * \param[in] rhs the Any to be copied.
	 */
        Any(const Any& rhs) :
	    value_(nullptr), in_buffer_(false), meta_type_(nullptr) {
	    copy(rhs);
	}

	/**
	 * \brief Any affectation operator.
	 * \param[in] rhs the Any to be copied.
	 * \return a reference to this Any after affectation.
	 */
	Any& operator=(const Any& rhs) {
	    if(&rhs != this) {
		destroy();
		copy(rhs);
	    }
	    return *this;
	}

	/**
	 * \brief Gets a string representation.
	 * \return a string representation of the stored argument
	 */
	std::string as_string() const {
	    std::string result;
	    if(value_ != nullptr) {
		convert_to_string(meta_type_, result, value_);
	    }
	    return result;
	}

	/**
	 * \brief Gets the MetaType of the stored value.
	 * \return a pointer to the MetaType or nullptr
	 *  if there is no stored value.
	 */
	MetaType* meta_type() const {
	    return meta_type_;
	}

	/**
	 * \brief Gets the LifeCycle.
	 * \details The LifeCycle knows how to construct, copy, and destroy
	 *  objects of a given type.
	 * \return a pointer to the LifeCycle.
	 */
	LifeCycle* life_cycle() const;

	/**
	 * \brief Sets the value of this Any.
	 * \param[in] value the value to be stored.
	 */
	template <class T> void set_value(const T& value) {
	    MetaType* new_type = resolve_meta_type<T>();
	    if(new_type == meta_type_) {
		life_cycle()->assign(value_, Memory::pointer(&value));
		return;
	    }
	    destroy();
	    meta_type_ = new_type;

#if defined(GEO_COMPILER_GCC)
// GCC does not see that we use placement new in the buffer
// only if object size is smaller than BUFFER_SIZE and
// generates a warning.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wplacement-new"
#elif defined(GEO_COMPILER_MSVC)
// MSVC barks on test with constant condition.
#pragma warning(push)
#pragma warning(disable:4127)
#endif

	    if(sizeof(T) <= BUFFER_SIZE) {
		in_buffer_ = true;
		value_ = buffer_;
		new(value_)T(value);
	    } else {
		in_buffer_ = false;
		value_ = Memory::pointer(new T(value));
	    }

#if defined(GEO_COMPILER_GCC)
#pragma GCC diagnostic pop
#elif defined(GEO_COMPILER_MSVC)
#pragma warning(pop)
#endif

	}

	/**
	 * \brief Sets the value of this Any (overload for
	 *  string literals).
	 * \details Strings literals are stored internally as
	 *  std::string.
	 * \param[in] value the string to be stored.
	 */
	void set_value(const char* value) {
	    set_value<std::string>(std::string(value));
	}

	/**
	 * \brief Gets the stored value.
	 * \param[out] value the stored value.
	 * \retval true if the specified type was correct.
	 * \retval false otherwise.
	 */
	template <class T> bool get_value(T& value) const {
	    if(value_ == nullptr) {
		return false;
	    }
	    // Test if the stored value is of type T
	    if(meta_type_ == resolve_meta_type<T>()) {
		value = value_as<T>();
		return true;
	    }
	    // If the stored value is a string, convert it to T
	    if(meta_type_ == resolve_meta_type<std::string>()) {
		const std::string& string_value = value_as<std::string>();
		convert_from_string(
		    resolve_meta_type<T>(),
		    string_value, (Memory::pointer)(&value)
		);
		return true;
	    }
	    return false;
	}

	/**
	 * \brief Gets the stored value (std::string overload).
	 * \param[out] value the stored value.
	 * \retval true if the specified type was correct.
	 * \retval false otherwise.
	 */
	bool get_value(std::string& value) const {
	    value = as_string();
	    return true;
	}

	/**
	 * \brief Gets the stored value (index_t overload).
	 * \param[out] value the stored value.
	 * \retval true if the specified type was correct.
	 * \retval false otherwise.
	 */
	bool get_value(index_t& value) const {
	    if(get_value<index_t>(value)) {
		return true;
	    }
	    {
		signed_index_t tmp;
		if(get_value<signed_index_t>(tmp)) {
		    value = index_t(tmp);
		    return true;
		}
	    }
	    {
		float tmp;
		if(get_value<float>(tmp)) {
		    value = index_t(tmp);
		    return true;
		}

	    }
	    {
		double tmp;
		if(get_value<double>(tmp)) {
		    value = index_t(tmp);
		    return true;
		}

	    }
	    return false;
	}

	/**
	 * \brief Gets the stored value (signed_index_t overload).
	 * \param[out] value the stored value.
	 * \retval true if the specified type was correct.
	 * \retval false otherwise.
	 */
	bool get_value(signed_index_t& value) const {
	    if(get_value<signed_index_t>(value)) {
		return true;
	    }
	    {
		index_t tmp;
		if(get_value<index_t>(tmp)) {
		    value = signed_index_t(tmp);
		    return true;
		}
	    }
	    {
		float tmp;
		if(get_value<float>(tmp)) {
		    value = signed_index_t(tmp);
		    return true;
		}

	    }
	    {
		double tmp;
		if(get_value<double>(tmp)) {
		    value = signed_index_t(tmp);
		    return true;
		}

	    }
	    return false;
	}

	/**
	 * \brief Gets the stored value (float overload).
	 * \param[out] value the stored value.
	 * \retval true if the specified type was correct.
	 * \retval false otherwise.
	 */
	bool get_value(float& value) const {
	    if(get_value<float>(value)) {
		return true;
	    }
	    {
		double tmp;
		if(get_value<double>(tmp)) {
		    value = float(tmp);
		    return true;
		}
	    }
	    {
		index_t tmp;
		if(get_value<index_t>(tmp)) {
		    value = float(tmp);
		    return true;
		}
	    }
	    {
		signed_index_t tmp;
		if(get_value<signed_index_t>(tmp)) {
		    value = float(tmp);
		    return true;
		}
	    }
	    return false;
	}

	/**
	 * \brief Gets the stored value (double overload).
	 * \param[out] value the stored value.
	 * \retval true if the specified type was correct.
	 * \retval false otherwise.
	 */
	bool get_value(double& value) const {
	    if(get_value<double>(value)) {
		return true;
	    }
	    {
		float tmp;
		if(get_value<float>(tmp)) {
		    value = double(tmp);
		    return true;
		}
	    }
	    {
		index_t tmp;
		if(get_value<index_t>(tmp)) {
		    value = double(tmp);
		    return true;
		}
	    }
	    {
		signed_index_t tmp;
		if(get_value<signed_index_t>(tmp)) {
		    value = double(tmp);
		    return true;
		}
	    }
	    return false;
	}

	/**
	 * \brief Gets the stored value (pointers overload).
	 * \details More complicated than using the default function,
	 *  this is because AnyStore<B> is not a subtype of AnyStore<A>
	 *  if B is a subclass of A (inheritance does not play
	 *  well with templates).
	 * \param[out] value the stored value.
	 * \retval true if the specified type was correct.
	 * \retval false otherwise.
	 */
	template <class T> bool get_value(T*& value) const {
	    value = nullptr;
	    if(is_null()) {
		return true;
	    }

	    if(is_smart_pointer_type(meta_type())) {
		// TODO: check pointer types
		Counted* counted = value_as<Counted*>();
		value = reinterpret_cast<T*>(counted);
		return true;
	    }

	    if(!is_pointer_type(meta_type())) {
		Logger::warn("GOM")
		    << "Invalid Any to pointer conversion: not pointer type "
		    << std::endl;
		Logger::warn("GOM")
		    << "type = "
		    << meta_type_name(meta_type())
		    << std::endl;
		Logger::warn("GOM")
		    << "value = "
		    << as_string()
		    << std::endl;
		return false;
	    }

	    /*
	    // TODO: check pointer types ? (but I do not want to include
	    // <meta.h> I think, to be checked).
	    if(!pointer_can_be_casted_to(meta_type(), ogf_meta<T>::type())) {
		Logger::warn("GOM")
		    << "Invalid Any to pointer conversion: invalid cast"
		    << std::endl;
		return false;
	    }
	    */

	    value = (T*)(value_as<Memory::pointer>());
	    return true;
	}

	/**
	 * \brief Initializes this Any with the default value of a specified
	 *  MetaType.
	 * \param[in] meta_type a pointer to the MetaType.
	 */
	void create(MetaType* meta_type) {
	    destroy();
	    meta_type_ = meta_type;
	    if(life_cycle()->object_size() <= BUFFER_SIZE) {
		in_buffer_ = true;
		value_ = buffer_;
		life_cycle()->construct(value_);
	    } else {
		in_buffer_ = false;
		value_ = life_cycle()->new_object();
	    }
	}

	/**
	 * \brief Assigns a value from a specifed address and specified type
	 *  to this Any.
	 * \param[in] addr a memory address
	 * \param[in] meta_type the MetaType
	 */
	void copy_from(Memory::pointer addr, MetaType* meta_type) {
	    if(value_ != nullptr && this->meta_type() == meta_type) {
		life_cycle()->assign(value_, addr);
		return;
	    }
	    if(value_ != nullptr) {
		destroy();
	    }
	    meta_type_ = meta_type;
	    if(life_cycle()->object_size() <= BUFFER_SIZE) {
		in_buffer_ = true;
		value_ = buffer_;
		life_cycle()->copy_construct(value_, addr);
	    } else {
		in_buffer_ = false;
		value_ = life_cycle()->new_object(addr);
	    }
	}

	/**
	 * \brief Copies the value stored in this Any at a specified address.
	 * \param[out] addr a memory address where to store the value
	 * \param[in] meta_type the MetaType
	 * \retval true on success
	 * \retval false if type mistmatches stored type and no conversion
	 *  is available.
	 */
	bool copy_to(Memory::pointer addr, MetaType* meta_type) const {
	    if(value_ == nullptr) {
		return false;
	    }
	    if(meta_type == this->meta_type()) {
		life_cycle()->assign(addr, value_);
		return true;
	    }
	    // If meta types do not match, try conversions.
	    return copy_convert_to(addr, meta_type);
	}

        /**
         * \brief Converts an object of a given type into a string.
         * \details It does the same thing as ogf_convert_to_string(),
         *  namely it uses the Serializer registered in the Meta repository.
         *  We cannot use ogf_convert_to_string() since it would introduce
         *  a circular dependency in Meta, that uses ArgList.
         * \param[in] meta_type a pointer to the MetaType
         * \param[out] string the string representation of the object
         * \param[in] value a pointer to the object
         */
        static void convert_to_string(
            MetaType* meta_type,
            std::string& string, Memory::pointer value
        );

        /**
         * \brief Converts a string into an object of a given type.
         * \details It does the same thing as ogf_convert_from_string(),
         *  namely it uses the Serializer registered in the Meta repository.
         *  We cannot use ogf_convert_from_string() since it would introduce
         *  a circular dependency in Meta, that uses ArgList.
         * \param[in] meta_type a pointer to the MetaType
         * \param[in] string a const reference to the string
         * \param[out] value a pointer to the converted value
         */
        static void convert_from_string(
	    MetaType* meta_type,
            const std::string& string, Memory::pointer value
        );

	/**
	 * \brief Tests whether a MetaType is a pointer type.
	 * \retval true if MetaType is a pointer type.
	 * \retval false otherwise.
	 */
	static bool is_pointer_type(const MetaType* mtype);

	/**
	 * \brief Tests whether a MetaType is a smart pointer type.
	 * \retval true if MetaType is SmartPointer<Counted>.
	 * \retval false otherwise.
	 */
	static bool is_smart_pointer_type(const MetaType* mtype);

	/**
	 * \brief Gets the deferenced type.
	 * \param[in] mtype a pointer MetaType.
	 * \return the deferenced type.
	 */
	static MetaType* pointed_type(const MetaType* mtype);

	/**
	 * \brief Tests whether a pointer type can be casted
	 *  to an object type.
	 * \param[in] derived_pointer_type the MetaType of the derived
	 *  pointer type.
	 * \param[in] base_pointer_type the MetaType of the base pointer type.
	 * \retval true if pointers of type derived_pointer_type can be casted
	 *  to pointers of type base_pointer_type.
	 * \retval false otherwise.
	 */
	static bool pointer_can_be_casted_to(
	    const MetaType* derived_pointer_type,
	    const MetaType* base_pointer_type
	);

        /**
         * \brief Finds a MetaType by typeid name
         * \param[in] typeid_name the C++ RTTI name of the system,
         *  as obtained by typeid<T>.name()
         * \return the MetaType associated with \p type_name if it exists
         *  or nullptr otherwise
	 * \details Redirects to Meta::resolve_meta_type_by_typeid_name(). It
	 *  is there because we cannot include <meta.h> since it uses ArgList
	 *  that in turn uses Any.
         */
	static MetaType* resolve_meta_type_by_typeid_name(
            const std::string& typeid_name
        );

	/**
	 * \brief Gets the MetaType associated with a type.
	 * \return a pointer to the MetaType.
	 */
	template <class T> static MetaType* resolve_meta_type() {
	    static MetaType* mtype =
		resolve_meta_type_by_typeid_name(typeid(T).name());
	    return mtype;
	}

      protected:

	/**
	 * \brief Tentatively converts the value stored in this Any to a type
	 *  and if successful, store it at a specified address.
	 * \param[out] addr a memory address where to store the value
	 * \param[in] meta_type the MetaType
	 * \retval true on success
	 * \retval false if no conversion is available.
	 */
	bool copy_convert_to(Memory::pointer addr, MetaType* meta_type) const;

	/**
	 * \brief Gets the value as a specific type.
	 * \tparam T the type.
	 * \return a reference to the stored value as type T.
	 * \details This is a brute-force cast, check before that
	 *  type matches.
	 */
	template <class T> const T& value_as() const {
	    return *(T*)(value_);
	}

	/**
	 * \brief Deallocates the stored variable.
	 */
	void destroy() {
	    if(value_ == nullptr) {
		return;
	    }
	    if(in_buffer_) {
		life_cycle()->destroy(value_);
		in_buffer_ = false;
	    } else {
		life_cycle()->delete_object(value_);
	    }
	    value_ = nullptr;
	    meta_type_ = nullptr;
	}

	/**
	 * \brief Copies another Any.
	 * \param[in] rhs the Any to be copied.
	 */
	void copy(const Any& rhs) {
	    geo_debug_assert(&rhs != this);
	    if(rhs.is_null()) {
		destroy();
		return;
	    }
	    meta_type_ = rhs.meta_type();
	    if(life_cycle()->object_size() <= BUFFER_SIZE) {
		in_buffer_ = true;
		value_ = buffer_;
		life_cycle()->copy_construct(value_, rhs.value_);
	    } else {
		in_buffer_ = false;
		value_ = life_cycle()->new_object(rhs.value_);
	    }
	}

	static std::string meta_type_name(const MetaType* mt);

      private:
	Memory::pointer value_;
	Memory::byte buffer_[BUFFER_SIZE];
	bool in_buffer_;
	MetaType* meta_type_;
    };

}

#ifdef GEO_COMPILER_CLANG
#pragma GCC diagnostic pop
#endif

#endif
