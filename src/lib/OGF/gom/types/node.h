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


#ifndef H_OGF_GOM_TYPES_NODE_H
#define H_OGF_GOM_TYPES_NODE_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/types/object.h>

/**
 * \file OGF/gom/types/node.h
 * \brief The base class for all composite objects in the GOM system.
 */

namespace OGF {

//_________________________________________________________

    /**
     * \brief A composite object in the GOM system.
     * \details By composite, we mean that it stores
     *  a list of references to children objects.
     */
    gom_class GOM_API Node : public Object {
    public:

        /**
         * \brief Node constructor.
         * \param[in] parent a pointer to this Node's parent if
         *  it exists, or nullptr otherwise.
         */
        explicit Node(Node* parent = nullptr) ;

        /**
         * \brief Node destructor.
         */
        ~Node() override ;

        /**
         * \brief Adds a child to this Node.
         * \details Reference-counted pointers are stored in
         *  the Node (memory ownership is transferred to the
         *  Node).
         * \param[in] child a pointer to the child to be added.
         */
        virtual void add_child(Node* child) ;

        /**
         * \brief Removes a child from this Node.
         * \parma[in] child a pointer to the child to be removed.
         */
        virtual void remove_child(Node* child) ;

    gom_slots:
        /**
         * \brief Gets a child by index.
         * \param[in] i the index
         * \return a pointer to the child of index \p i
         * \pre i < get_nb_children()
         */
        Node* ith_child(index_t i) const {
            ogf_assert(i < get_nb_children()) ;
            return children_[i] ;
        }

    gom_properties:
        /**
         * \brief Gets the number of children.
         * \return the number of children of this Node.
         */
        size_t get_nb_children() const {
            return children_.size();
        }

        /**
         * \brief Gets the parent.
         * \return a pointer to the parent of this Node
         *  if it exists or nullptr otherwise
         */
        Node* get_parent() const {
            return parent_;
        }

    protected:
        /**
         * \brief Swaps two children
         * \param[in] n1 , n2 the two children to swap
         */
        void swap_children(Node* n1, Node* n2) {
            index_t i = NO_INDEX;
            index_t j = NO_INDEX;
            for(index_t k=0; k<children_.size(); ++k) {
                if(children_[k] == n1) {
                    i = k;
                }
                if(children_[k] == n2) {
                    j = k;
                }
            }
            geo_assert(i != NO_INDEX && j != NO_INDEX);
            std::swap(children_[i], children_[j]);
        }

	/**
	 * \brief Makes a child the first element in the list
	 * \param[in] n the child
	 */
	void move_child_to_top(Node* n) {
	    SmartPointer<Node> pn;
	    auto it = std::find(children_.begin(), children_.end(), n);
	    geo_assert(it != children_.end());
	    pn = *it;
	    children_.erase(it);
	    children_.insert(children_.begin(), pn);
	}

	/**
	 * \brief Makes a child the last element in the list
	 * \param[in] n the child
	 */
	void move_child_to_bottom(Node* n) {
	    SmartPointer<Node> pn;
	    auto it = std::find(children_.begin(), children_.end(), n);
	    geo_assert(it != children_.end());
	    pn = *it;
	    children_.erase(it);
	    children_.push_back(pn);
	}

    private:
        std::vector< SmartPointer<Node> > children_ ;
        Node* parent_ ;
    } ;

    /**
     * \brief An automatic reference-counted pointer to a Node.
     */
    typedef SmartPointer<Node> Node_var ;

//_________________________________________________________

}
#endif
