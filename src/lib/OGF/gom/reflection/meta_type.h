/*
 *  GXML/Graphite: Geometry and Graphics Programming Library + Utilities
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

#ifndef H_OGF_META_TYPES_META_TYPE_H
#define H_OGF_META_TYPES_META_TYPE_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/services/life_cycle.h>
#include <OGF/gom/services/serializer.h>
#include <OGF/gom/types/arg_list.h>
#include <OGF/gom/types/object.h>
#include <string>

/**
 * \file OGF/gom/reflection/meta_type.h
 * \brief Base class for meta informations
 */

namespace OGF {

    /**
     * \brief Stores a set of custom attributes, i.e. name-value pairs
     *  attached to the objects.
     * \details All the constructs of the language (classes, methods, 
     *  arguments...) can be "decorated" with custom attributes. For
     *  instance, these custom attribute can indicate which widget type
     *  should be used for a function argument in the GUI. They can also
     *  contain the documentation parsed from the Doxygen comments.
     */
    class GOM_API CustomAttributes {
    public:

        /**
         * \brief Gets the number of custom attributes.
         * \return the number of custom attribute
         */
        size_t nb_custom_attributes() const {
            return custom_attributes_.nb_args() ;
        }

        /**
         * \brief Gets the name of a custom attribute by index.
         * \param[in] i the index of the custom attribute
         * \return the name of the \p i th custom attribute
         * \pre i < nb_custom_attributes()
         */
        std::string ith_custom_attribute_name(index_t i) const {
            return custom_attributes_.ith_arg_name(i);
        }

        /**
         * \brief Gets the value of a custom attribute by index.
         * \param[in] i the index of the custom attribute
         * \return the value of the \p i th custom attribute
         * \pre i < nb_custom_attributes()
         */
        std::string ith_custom_attribute_value(index_t i) const {
            return custom_attributes_.ith_arg_value(i).as_string();
        }
        
        /**
         * \brief Tests whether a given custom attribute exists
         * \param[in] name name of the custom attribute
         * \retval true if the object has a custom attribute named \p name
         * \retval false otherwise
         */
        bool has_custom_attribute(const std::string& name) const {
            return custom_attributes_.has_arg(name) ;
        }

        /**
         * \brief Creates a new custom attribute
         * \param[in] name name of the custom attribute
         * \param[in] value value of the custom attribute
         * \pre !has_custom_attribute(name)
         */
        void create_custom_attribute(
            const std::string& name, const std::string& value
        ) {
            custom_attributes_.create_arg(name, value) ;
        }

        /**
         * \brief Sets the value of a new custom attribute
         * \param[in] name name of the custom attribute
         * \param[in] value value of the custom attribute
         * \pre has_custom_attribute(name)
         */
        void set_custom_attribute(
            const std::string& name, const std::string& value
        ) {
            custom_attributes_.set_arg(name, value) ;
        }

        /**
         * \brief Gets the value of a new custom attribute
         * \param[in] name name of the custom attribute
         * \return the value of the custom attribute
         * \pre has_custom_attribute(name)
         */
        std::string custom_attribute_value(const std::string& name) const {
	    geo_debug_assert(has_custom_attribute(name));
            return custom_attributes_.get_arg(name) ;
        }

	/**
	 * \brief Clears the custom attributes.
	 */
	void clear_custom_attributes() {
	    custom_attributes_.clear();
	}
	
    private:
        ArgList custom_attributes_ ;
    } ;


    /**
     * \brief Base class for everything that resides in the Meta repository.
     */
    gom_attribute(abstract,"true")
    gom_class GOM_API MetaInformation : public Object, public CustomAttributes {
    public:
        MetaInformation() {
        }
        virtual ~MetaInformation() ;
	
    gom_slots:

	/**
	 * \copydoc CustomAttributes::nb_custom_attributes()
	 */
	size_t nb_custom_attributes() const;

	/**
	 * \copydoc CustomAttributes::has_custom_attribute()
	 */
	bool has_custom_attribute(const std::string& name) const; 

	/**
	 * \copydoc CustomAttributes::ith_custom_attribute_name()
	 */
	std::string ith_custom_attribute_name(index_t i) const;

	/**
	 * \copydoc CustomAttributes::ith_custom_attribute_value()
	 */
	std::string ith_custom_attribute_value(index_t i) const;

	/**
	 * \copydoc CustomAttributes::custom_attribute_value()
	 */
	std::string custom_attribute_value(const std::string& name) const;
	
    } ;

    /**
     * \brief The representation of a type in the Meta repository.
     */
    gom_attribute(abstract,"true")    
    gom_class GOM_API MetaType : public MetaInformation {
    public:
        
        /**
         * \brief MetaType constructor.
         * \param[in] name C++ name of the type
         */
        explicit MetaType(const std::string& name) ;

        /**
         * \brief MetaType destructor.
         */
	~MetaType() override;


        /**
         * \brief Gets the C++ name of the type.
         * \return the C++ name of the type (as in C++ sources,
         *  with OGF:: scope)
         */
        const std::string& name() const {
            return name_ ;
        }

        /**
         * \brief Sets the typeid name.
         * \param[in] typeid_name the typeid name, 
         *  as obtained by typeid(T).name()
         */
        void set_typeid_name(const std::string& typeid_name ) {
            typeid_name_ = typeid_name ;
        }

        /**
         * \brief Gets the typeid name.
         * \return the typeid name, as obtained by typeid(T).name()
         */
        const std::string& typeid_name() const {
            return typeid_name_ ;
        }

        /**
         * \brief Gets the LifeCycle associated with the type.
         * \return a pointer to the LifeCycle or nil if none
         *  is available
         */
	LifeCycle* life_cycle() const {
	    return life_cycle_;
	}

        /**
         * \brief Sets the LifeCycle associated with the type.
         * \param[in] life_cycle a pointer to the LifeCycle. Ownership
         *  is transfered to this MetaType.
         */
	void set_life_cycle(LifeCycle* life_cycle) {
	    life_cycle_ = life_cycle;
	    // life_cycle_->ref(); 
	}
	
        /**
         * \brief Gets the serializer associated with the type.
         * \return a pointer to the Serializer or nil if none
         *  is available
         */
        Serializer* serializer() const {
            return serializer_ ;
        }

        /**
         * \brief Sets the serializer associated with the type.
         * \param[in] ser a pointer to the serializer. Ownership
         *  is transfered to this MetaType.
         */
        void set_serializer(Serializer* ser) {
            serializer_ = ser ;
        }

	/**
	 * \brief Removes all variables that use the meta type system
	 *  before deleting.
	 * \details If we do not do that, when deleting the meta type system,
	 *  we can delete the meta information in the wrong order, and delete
	 *  first meta classes that we needed to delete other ones.
	 */
	virtual void pre_delete();
	
    gom_slots:
        
        /**
         * \brief Tests whether this MetaType is a subclass of 
         *  another MetaType.
         * \param[in] other a pointer to a MetaType
         * \retval true if this MetaType is a subclass of \p other
         * \retval false otherwise
         */
	bool is_a(const MetaType* other) const override;

    gom_properties:
        /**
         * \brief Gets the C++ name of the type.
         * \return the C++ name of the type (as in C++ sources,
         *  with OGF:: scope)
         */
        const std::string& get_name() const {
            return name();
        }

        
    private:
        std::string name_;
        std::string typeid_name_;
	LifeCycle_var life_cycle_;
        Serializer_var serializer_;
    } ;

    /**
     * \brief Automatic reference-counted pointer to a MetaType.
     */
    typedef SmartPointer<MetaType> MetaType_var ;   
}

#endif 

