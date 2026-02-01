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

#include <OGF/gom/types/any.h>
#include <OGF/gom/reflection/meta.h>
#include <geogram/basic/memory.h>

#ifdef GEO_COMPILER_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif

namespace OGF {

    LifeCycle* Any::life_cycle() const {
	geo_debug_assert(meta_type_ != nullptr);
	return meta_type_->life_cycle();
    }

    bool Any::convert_from_string(
        MetaType* mtype, const std::string& string, Memory::pointer value
    ) {
#ifdef GEO_OS_WINDOWS
        // If we do not do that, there is an infinite recursion in the
        // assertion mechanism because it tries to display messages in
        // Graphite's terminal window.
        if(mtype == nullptr) {
            return false;
        }
#endif
        geo_assert(mtype != nullptr);
        Serializer* serializer = mtype->serializer();
        geo_assert(serializer != nullptr);
        std::istringstream stream(string);
        bool conversion_ok = serializer->serialize_read(stream, value);
	if(!conversion_ok) {
	    Logger::err("GOM") << "Invalid value \'" << string << "\' for "
			       << mtype->name() << std::endl;
	}
	return conversion_ok;
    }

    bool Any::convert_to_string(
	MetaType* mtype, std::string& string, Memory::pointer value
    ) {
#ifdef GEO_OS_WINDOWS
        // If we do not do that, there is an infinite recursion in the
        // assertion mechanism because it tries to display messages in
        // Graphite's terminal window.
        if(mtype == nullptr) {
            return false;
        }
#endif
        geo_assert(mtype != nullptr);
        Serializer* serializer = mtype->serializer();
        geo_assert(serializer != nullptr);
        std::ostringstream stream ;
        bool conversion_ok = serializer->serialize_write(stream, value);
	if(!conversion_ok) {
	    Logger::err("GOM") << "Could not convert " << mtype->name()
			       << " to string" << std::endl;
	}
        string = stream.str();
	return conversion_ok;
    }

    bool Any::is_smart_pointer_type(const MetaType* mtype) {
	return mtype == ogf_meta<SmartPointer<Counted>>::type();
    }

    bool Any::is_pointer_type(const MetaType* mtype) {
	return mtype == ogf_meta<std::nullptr_t>::type() ||
	    (mtype->name()[mtype->name().length()-1] == '*');
    }

    MetaType* Any::pointed_type(const MetaType* mtype) {
	geo_debug_assert(is_pointer_type(mtype));
	if(mtype == ogf_meta<std::nullptr_t>::type()) {
	    return ogf_meta<void>::type();
	}

	std::string deferenced_type_name =
	    mtype->name().substr(0, mtype->name().length()-1);
	return Meta::instance()->resolve_meta_type(deferenced_type_name);
    }

    MetaType* Any::resolve_meta_type_by_typeid_name(
	const std::string& typeid_name
    ) {
	return Meta::instance()->resolve_meta_type_by_typeid_name(typeid_name);
    }


    bool Any::pointer_can_be_casted_to(
	const MetaType* derived_pointer_type,
	const MetaType* base_pointer_type
    ) {
	MetaClass* base = dynamic_cast<MetaClass*>(
	    pointed_type(base_pointer_type)
	);
	MetaClass* derived = dynamic_cast<MetaClass*>(
	    pointed_type(derived_pointer_type)
	);
	return base != nullptr && derived != nullptr &&
            derived->is_subtype_of(base);
    }

    /**
     * \brief Helper function for Any::copy_to. Tentatively converts the
     *  stored value to a specified type.
     * \param[in] any a pointer to the Any that contains the value to be
     *  converted.
     * \param[in] addr a pointer to the address where to store the extracted
     *  value.
     * \param[in] meta_type the specified MetaType for \p addr. If it matches T,
     *  then conversion is tempted, else the function returns false.
     * \retval true if conversion was successful.
     * \retval false otherwise.
     */
    template <class T> bool try_copy_convert_to(
        const Any* any, T* addr, MetaType* meta_type
    ) {
	if(meta_type != ogf_meta<T>::type()) {
	    return false;
	}
	return any->get_value(*addr);
    }

    bool Any::copy_convert_to(Memory::pointer addr, MetaType* meta_type) const {
	if(try_copy_convert_to(this, (index_t*)addr, meta_type)) {
	    return true;
	}
	if(try_copy_convert_to(this, (signed_index_t*)addr, meta_type)) {
	    return true;
	}
	if(try_copy_convert_to(this, (float*)addr, meta_type)) {
	    return true;
	}
	if(try_copy_convert_to(this, (double*)addr, meta_type)) {
	    return true;
	}
	if(try_copy_convert_to(this, (std::string*)addr, meta_type)) {
	    return true;
	}
	if(try_copy_convert_to(this, (size_t*)addr, meta_type)) {
	    return true;
	}
	if(try_copy_convert_to(this, (bool*)addr, meta_type)) {
	    return true;
	}
	return false;
    }

    std::string Any::meta_type_name(const MetaType* mt) {
	return mt->name();
    }

    /********************************************************************/

}

#ifdef GEO_COMPILER_CLANG
#pragma GCC diagnostic pop
#endif
