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

#ifndef H_OGF_GOM_SERVICES_SERIALIZER_H
#define H_OGF_GOM_SERVICES_SERIALIZER_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/types/arg_list.h>
#include <iostream>

/**
 * \file OGF/gom/services/serializer.h
 * \brief Implementation of the generic serialization mechanism.
 */


namespace OGF {

//______________________________________________________________________

    /**
     * \brief Abstract base class for reading and writing values 
     *  from/to streams.
     */
    class GOM_API Serializer : public Counted {
    public:

	/**
	 * \brief Serializer destructor.
	 */
	~Serializer() override;
	
        /**
         * \brief Reads a value from a stream.
         * \param[in] stream the input stream
         * \param[out] addr an untyped pointer 
         *  where to store the result
         * \retval true if the value could be successfully read
         * \retval false otherwise
         */
        virtual bool serialize_read(
            std::istream& stream, void* addr
        ) = 0;

        /**
         * \brief Writes a value to a stream.
         * \param[out] stream the output stream
         * \param[in] addr an untyped pointer 
         *  where to read the value from
         * \retval true if the value could be successfully written
         * \retval false otherwise
         */
        virtual bool serialize_write(
            std::ostream& stream, void* addr
        ) = 0;
    };

    /**
     * \brief Automatic reference-counted pointer to a Serializer.
     */
    typedef SmartPointer<Serializer> Serializer_var;

//______________________________________________________________________

    /**
     * \brief Implementation of Serializer for std::string.
     */
    class GOM_API StringSerializer : public Serializer {
    public:
        /**
         * \copydoc Serializer::serialize_read()
         */
        bool serialize_read(
            std::istream& stream, void* addr
        ) override;
        
        /**
         * \copydoc Serializer::serialize_write()
         */
        bool serialize_write(
            std::ostream& stream, void* addr
        ) override;
    };

//______________________________________________________________________

    /**
     * \brief Implementation of Serializer for bool.
     */
    class GOM_API BoolSerializer : public Serializer {
    public:
        /**
         * \copydoc Serializer::serialize_read()
         */
        bool serialize_read(
            std::istream& stream, void* addr
        ) override;
        
        /**
         * \copydoc Serializer::serialize_write()
         */
        bool serialize_write(
            std::ostream& stream, void* addr
        ) override;
    };

//______________________________________________________________________

    /**
     * \brief Implementation of Serializer for pointers.
     */
    class GOM_API PointerSerializer : public Serializer {
    public:

        /**
         * \copydoc Serializer::serialize_read()
         */
        bool serialize_read(
            std::istream& stream, void* addr
        ) override;

        /**
         * \copydoc Serializer::serialize_write()
         */
        bool serialize_write(
            std::ostream& stream, void* addr
        ) override;
    };

//______________________________________________________________________

    class MetaEnum;

    /**
     * \brief Implementation of Serializer for enums.
     */
    class GOM_API EnumSerializer : public Serializer {
    public:

        /**
         * \brief EnumSerializer constructor.
         * \details The symbolic names declared in the MetaEnum are
         *  written/read to the stream (instead of the numeric values).
         * \param[in] meta_enum a pointer to the MetaEnum
         */
        explicit EnumSerializer(MetaEnum* meta_enum) : meta_enum_(meta_enum) {
        }

        /**
         * \copydoc Serializer::serialize_read()
         */
        bool serialize_read(
            std::istream& stream, void* addr
        ) override;

        /**
         * \copydoc Serializer::serialize_write()
         */
        bool serialize_write(
            std::ostream& stream, void* addr
        ) override;

    private:
        MetaEnum* meta_enum_;
    };

//______________________________________________________________________

    /**
     * \brief Generic implementation of Serializer.
     * \tparam T a type that has operator<< and operator>>
     */
    template <class T> class GenericSerializer : public Serializer {
    public:
        /**
         * \copydoc Serializer::serialize_read()
         */
        bool serialize_read(
            std::istream& stream, void* addr
        ) override {
            T* object = static_cast<T*>(addr);
            stream >> (*object);
            return true;
        }

        /**
         * \copydoc Serializer::serialize_write()
         */
        bool serialize_write(
            std::ostream& stream, void* addr
        ) override {
            T* object = static_cast<T*>(addr);
            stream << (*object);
            return true;
        }
    };

    /**
     * \brief A specialization of GenericSerializer for ArgList.
     * \details Its functions do nothing and return false. This
     *  class is just here as a placeholder.
     */
    template <> class GOM_API GenericSerializer<ArgList> : public Serializer {

	/**
	 * \brief GenericSerializer destructor.
	 */
        ~GenericSerializer() override;
       
        /**
         * \copydoc Serializer::serialize_read()
         * \details Not implemented for ArgList
         * \return false
         */
        bool serialize_read(
            std::istream& stream, void* addr
	) override;

        /**
         * \copydoc Serializer::serialize_write()
         * \details Not implemented for ArgList
         * \return false
         */
        bool serialize_write(
            std::ostream& stream, void* addr
	) override;
    };
    
//______________________________________________________________________

}

#endif
