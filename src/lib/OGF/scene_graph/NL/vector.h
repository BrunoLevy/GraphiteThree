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
 
#ifndef H_OGF_SCENE_GRAPH_NL_VECTOR_H
#define H_OGF_SCENE_GRAPH_NL_VECTOR_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/gom/types/object.h>
#include <geogram/basic/attributes.h>

namespace OGF {

    class Grob;
    
    namespace NL {
    
	/**
	 * \brief A scriptable Vector objects.
	 */
	gom_class SCENE_GRAPH_API Vector :
	    public Object, public AttributeStoreObserver {
	    
	  public:
	    /**
	     * \brief Vector constructor.
	     * \param[in] size number of items
	     * \param[in] dimension number of elements per item
	     * \param[in] element_meta_type type of the elements. 
	     *  If nullptr defaults to double
	     */
	    Vector(
		index_t size=0, index_t dimension=1,
		MetaType* element_meta_type=nullptr
	    );

	    /**
	     * \brief Vector constructor.
	     * \details Constructs a view on a Grob attribute.
	     * \param[in] grob a pointer to a Grob.
	     * \param[in] attribute_store a pointer to an AttributeStore of
	     *  the Grob.
	     */
	    Vector(Grob* grob, AttributeStore* attribute_store);

	    /**
	     * \brief Vector constructor.
	     * \param[in] grob a pointer to a Grob or nil
	     * \param[in] data data pointer
	     * \param[in] size number of items
	     * \param[in] dimension number of elements per item
	     * \param[in] element_meta_type type of the elements. 
	     * \param[in] read_only indicates whether the vector 
	     *  can be modified
	     */
	    Vector(
		Grob* grob,  void* data, index_t size, index_t dimension,
		MetaType* element_meta_type, bool read_only
	    );
	    
	    /**
	     * \brief Vector destructor.
	     */
	     ~Vector() override;

	    /**
	     * \brief Gets an element.
	     * \param[in] i element index, in 0..nb_elements()-1
	     * \param[out] value the value of the element, stored in an Any.
	     */
	    void get_element(index_t i, Any& value) const override;

	    /**
	     * \brief Sets an element.
	     * \param[in] i element index, in 0..nb_elements()-1
	     * \param[in] value the value of the element, stored in an Any.
	     */
	    void set_element(index_t i, const Any& value) override;

	    /**
	     * \brief Gets the data pointer.
	     * \return a pointer to the first element. All elements are stored
	     *  contiguously. 
	     */
	    Memory::pointer data() const {
		return base_addr_;
	    }

	    /**
	     * \brief Gets the data pointer as doubles.
	     * \return a pointer to the first element. All elements are stored
	     *  contiguously. If data type is not double, returns nullptr.
	     */
	    double* data_double() const;

	    /**
	     * \brief Gets the data pointer as index_t.
	     * \return a pointer to the first element. All elements are stored
	     *  contiguously. If data type is not index_t, returns nullptr.
	     */
	    index_t* data_index_t() const;
	    
	  gom_properties:
	    
	    /**
	     * \copydoc Object::get_nb_elements()
	     */
	    index_t get_nb_elements() const override {
		return nb_elements();
	    }

	    /**
	     * \brief Gets the dimension.
	     * \return the number of elements per item.
	     */
	    index_t get_dimension() const {
		return dimension();
	    }

	    /**
	     * \brief Gets the size.
	     * \return the number of items.
	     */
	    index_t get_size() const {
		return size();
	    }
	    
	    /**
	     * \brief Gets the MetaType of the elements.
	     * \return a pointer to the MetaType of the elements.
	     */
	    MetaType* get_element_meta_type() const {
		return element_meta_type_;
	    }

	    /**
	     * \brief Tests whether this Vector is read-only.
	     * \retval true if this Vector is read-only.
	     * \retval false if this Vector is read-write.
	     */
	    bool get_read_only() const {
		return read_only_;
	    }
	    
	  gom_slots:
	    
	    /**
	     * \brief Resizes a vector.
	     * \details Previous values are not kept.
	     * \param[in] new_size the new size.
	     * \param[in] new_dim the new dimension.
	     * \param[in] element_meta_type if non null, change elements
	     *  meta type.
	     */
	    virtual void resize(
		index_t new_size, index_t new_dim=1,
		MetaType* element_meta_type=nullptr
	    );

	  protected:
	    
	    /**
	     * \brief Tests whether index i is valid.
	     * \details If index is invalid, displays an error message.
	     * \retval true if index i is in 0 .. get_size()-1
	     * \retval false otherwise.
	     */
	    bool check_index(index_t i) const;
	    
	  private:
	    size_t element_size_;
	    MetaType* element_meta_type_;
	    bool owns_memory_;
	    Grob* grob_;
	    AttributeStore* attribute_store_;
	    bool read_only_;
	};
	
	/**********************************************************/

    }
}

#endif

