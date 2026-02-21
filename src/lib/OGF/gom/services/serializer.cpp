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


#include <OGF/gom/services/serializer.h>
#include <OGF/gom/reflection/meta_enum.h>

#include <iostream>

namespace OGF {

//__________________________________________________________________________

    Serializer::~Serializer() {
    }

//__________________________________________________________________________

    // Note: we use "std::getline(in, x)" instead of "in >> x" since
    // "in >> x" would truncate the string if it contains whitespace
    // (and under XP, "My documents" directory contains a
    // whitespace !)

    bool StringSerializer::serialize_read(
        std::istream& stream, void* addr
    ) {
        std::string* object = static_cast<std::string*>(addr) ;
        std::getline(stream, *object) ;
        return true ;
    }

    bool StringSerializer::serialize_write(
        std::ostream& stream, void* addr
    ) {
        std::string* object = static_cast<std::string*>(addr) ;
        stream << *object ;
        return true ;
    }


//__________________________________________________________________________

    bool BoolSerializer::serialize_read(
        std::istream& stream, void* addr
    ) {
        bool* object = static_cast<bool*>(addr) ;
        std::string value ;
        stream >> value ;
        if(value == "true" || value == "1") {
            *object = true ;
        } else if(value == "false" || value == "0") {
            *object = false ;
        } else {
            return false ;
        }
        return true ;
    }

    bool BoolSerializer::serialize_write(
        std::ostream& stream, void* addr
    ) {
        bool* object = static_cast<bool*>(addr) ;
        stream << (*object ? "true" : "false") ;
        return true ;
    }

//__________________________________________________________________________

    bool PointerSerializer::serialize_read(
        std::istream& stream, void* addr
    ) {
// Workaround for bugged Wine's built-in MSVCP90.DLL
#ifdef GEO_OS_WINDOWS_WINE
        Numeric::uint32 val ;
        stream >> val ;
        void** object =
            static_cast<void**>(addr) ;
        *object = reinterpret_cast<void*>(val) ;
        return true ;
#else
    // special for os x : erreur with isstream in libc++ in 10.9
    #ifdef GEO_OS_APPLE
        unsigned long val ;
        stream >> val ;
        void** object =
            static_cast<void**>(addr) ;
        *object = reinterpret_cast<void*>(val) ;
        return true;
    #else
        Numeric::pointer val ;
        stream >> val ;
        void** object =
            static_cast<void**>(addr) ;
        *object = reinterpret_cast<void*>(val) ;
        return true ;
    #endif
#endif
    }

    bool PointerSerializer::serialize_write(
        std::ostream& stream, void* addr
    ) {
// Workaround for bugged Wine's built-in MSVCP90.DLL
#ifdef GEO_OS_WINDOWS_WINE
        void** object =
            static_cast<void**>(addr) ;
        Numeric::uint32 val =
            reinterpret_cast<Numeric::uint32>(*object) ;
        stream << val ;
        return true ;
#else
    // special for os x : erreur with isstream in libc++ in 10.9
    #ifdef GEO_OS_APPLE
        void** object =
            static_cast<void**>(addr) ;
        unsigned long val = reinterpret_cast<unsigned long>(*object) ;
        stream << val;
        return true;
    #else
       void** object =
            static_cast<void**>(addr) ;
        Numeric::pointer val =
            reinterpret_cast<Numeric::pointer>(*object) ;
        stream << val ;
        return true ;
    #endif
#endif
    }

//__________________________________________________________________________

    bool EnumSerializer::serialize_read(
        std::istream& stream, void* addr
    ) {
        int* val = static_cast<int*>(addr) ;
        std::string value_name ;
        stream >> value_name ;
        if(!meta_enum_->has_value(value_name)) {
	    Logger::err("EnumSerializer::read")
		<< "invalid value name: "
		<< value_name
		<< std::endl;
            *val = 0 ;
            return false ;
        }
        *val = meta_enum_->get_value_by_name(value_name) ;
        return true ;
    }

    bool EnumSerializer::serialize_write(
        std::ostream& stream, void* addr
    ) {
        int* val = static_cast<int*>(addr) ;
        if(!meta_enum_->has_value(*val)) {
            stream << "error" ;
	    Logger::err("EnumSerializer::write")
		<< "invalid value : "
		<< *val
		<< std::endl;
            return false ;
        }
        stream << meta_enum_->get_name_by_value(*val) ;
        return true ;
    }

//__________________________________________________________________________

   GenericSerializer<ArgList>::~GenericSerializer() {
   }

   bool GenericSerializer<ArgList>::serialize_read(
       std::istream& stream, void* addr
   ) {
       ogf_argused(stream);
       ogf_argused(addr);
       return false;
   }

   bool GenericSerializer<ArgList>::serialize_write(
       std::ostream& stream, void* addr
   ) {
       ogf_argused(stream);
       ogf_argused(addr);
       return false;
   }


//__________________________________________________________________________

}
