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

#include <OGF/scene_graph/NL/vector.h>
#include <OGF/scene_graph/grob/grob.h>
#include <OGF/gom/reflection/meta.h>

namespace OGF {

    namespace NL {

	/**********************************************************/

	// TODO: function to copy other vector

	Vector::Vector(
	    index_t size, index_t dimension, MetaType* element_meta_type
	) {
	    owns_memory_ = true;
	    read_only_ = false;
	    size_ = 0;
	    dimension_ = dimension;
	    base_addr_ = nullptr;
	    element_meta_type_ = element_meta_type;
	    if(element_meta_type_ == nullptr) {
		element_meta_type_ = ogf_meta<double>::type();
	    }
	    element_size_ = element_meta_type_->life_cycle()->object_size();
	    if(size != 0) {
		resize(size, dimension, get_element_meta_type());
	    }
	    grob_ = nullptr;
	    attribute_store_ = nullptr;
	}

	Vector::Vector(Grob* grob, AttributeStore* attribute_store) {
	    owns_memory_ = false;
	    read_only_ = false;
	    grob_ = grob;
	    attribute_store_ = attribute_store;
	    size_ = 0;
	    dimension_ = 0;
	    base_addr_ = nullptr;
	    element_meta_type_ =
		Meta::instance()->resolve_meta_type_by_typeid_name(
		    attribute_store_->element_typeid_name()
		);
	    // Special case for Attribute<bool> (all this because the way
	    // STL handles vector<bool>, argh...)
	    if(
		attribute_store_->element_typeid_name() ==
		typeid(Numeric::uint8).name()
	    ) {
		element_meta_type_ = ogf_meta<bool>::type();
	    }
	    geo_assert(element_meta_type_ != nullptr);
	    element_size_ = element_meta_type_->life_cycle()->object_size();
	    base_addr_ = nullptr;
	    register_me(attribute_store_);
	}

	Vector::Vector(
	    Grob* grob, void* data, index_t size, index_t dimension,
	    MetaType* element_meta_type, bool read_only
	) {
	    owns_memory_ = false;
	    read_only_ = read_only;
	    grob_ = grob;
	    attribute_store_ = nullptr;
	    size_ = size;
	    dimension_ = dimension;
	    base_addr_ = Memory::pointer(data);
	    element_meta_type_ = element_meta_type;
	    element_size_ = element_meta_type_->life_cycle()->object_size();
	}

	Vector::~Vector() {
	    if(owns_memory_) {
		element_meta_type_->life_cycle()->delete_array(base_addr_);
		base_addr_ = nullptr;
	    } else if(attribute_store_ != nullptr) {
		// disconnected_ is true if the AttributeStore was
		// destroyed before.
		if(!disconnected_) {
		    unregister_me(attribute_store_);
		}
	    }
	}

	void Vector::resize(
	    index_t size, index_t dimension, MetaType* element_meta_type
	) {
	    if(
		size == size_ &&
		dimension == dimension_ &&
		(element_meta_type == nullptr ||
		 element_meta_type == element_meta_type_)
	    ) {
		return;
	    }

	    if(!owns_memory_ || read_only_) {
		Logger::err("NL::Vector")
		    << "Cannot resize, Vector does not own memory"
		    << std::endl;
		return;
	    }

	    element_meta_type_->life_cycle()->delete_array(base_addr_);

	    if(element_meta_type != nullptr) {
		element_meta_type_ = element_meta_type;
	    } else {
		element_meta_type_ = ogf_meta<double>::type();
	    }

	    element_size_ = element_meta_type_->life_cycle()->object_size();

	    base_addr_ =
		element_meta_type_->life_cycle()->new_array(size * dimension);

	    size_ = size;
	    dimension_ = dimension;

	    // If element type is a plain ordinary datatype,
	    // then we set everybody to zero using a low-level
	    // clearing operation.
	    if(element_meta_type_->life_cycle()->is_pod()) {
		Memory::clear(base_addr_, nb_elements() * element_size_);
	    }

	}

	bool Vector::check_index(index_t i) const {
	    if(i < nb_elements()) {
		return true;
	    }
	    Logger::err("Vector") << i << " index out of legal range "
				  << "[0.." << nb_elements()-1 << "]"
				  << std::endl;
	    return false;
	}

	void Vector::get_element(index_t index, Any& value) const {
	    if(!check_index(index)) {
		value.reset();
		return;
	    }
	    value.create(element_meta_type_);
	    value.copy_from(
		base_addr_ + index*element_size_, element_meta_type_
	    );
	}

	void Vector::set_element(index_t index, const Any& value) {
	    if(read_only_) {
		Logger::err("GOM") << "Vector is read-only" << std::endl;
		return;
	    }
	    if(!check_index(index)) {
		return;
	    }
	    value.copy_to(base_addr_ + index*element_size_, element_meta_type_);
	    // If this vector is an attribute of an object, mark this object
	    // as dirty for graphics update.
	    if(grob_ != nullptr) {
		grob_->update();
	    }
	}

	double* Vector::data_double() const {
	    if(get_element_meta_type() != ogf_meta<double>::type()) {
		return nullptr;
	    }
#ifdef GEO_COMPILER_CLANG
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wcast-align"
	    return (double*)base_addr_;
#  pragma GCC diagnostic pop
#else
	    return (double*)base_addr_;
#endif
	}

	index_t* Vector::data_index_t() const {
	    if(
		get_element_meta_type() != ogf_meta<index_t>::type() &&
		get_element_meta_type() != ogf_meta<unsigned int>::type()
	    ) {
		return nullptr;
	    }
#ifdef GEO_COMPILER_CLANG
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wcast-align"
	    return (index_t*)base_addr_;
#  pragma GCC diagnostic pop
#else
	    return (index_t*)base_addr_;
#endif
	}

	/**********************************************************/

    }
}
