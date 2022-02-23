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

#ifndef H_OGF_SCENE_GRAPH_TYPES_COMPOSITE_GROB_H
#define H_OGF_SCENE_GRAPH_TYPES_COMPOSITE_GROB_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/scene_graph/grob/grob.h>
#include <OGF/gom/interpreter/interpreter.h>

#include <map>

/**
 * \file OGF/scene_graph/grob/composite_grob.h
 * \brief A class for Grob composed of several parts
 */

namespace OGF {

//_________________________________________________________


    /**
     * \brief An Interpreter Scope that corresponds to the objects
     *  in a CompositeGrob.
     */
    gom_class SCENE_GRAPH_API CompositeGrobScope : public Scope {
    public:
	/**
	 * \brief CompositeGrobScope constructor.
	 * \param[in] grob the CompositeGrob this CompositeGrobScope
	 *  belongs to.
	 */
	CompositeGrobScope(CompositeGrob* grob);

	/**
	 * \brief CompositeGrobScope destructor.
	 */
	 ~CompositeGrobScope() override;

	/**
	 * \copydoc Scope::resolve()
	 */
	 Any resolve(const std::string& name) override;

	/**
	 * \copydoc Scope::list_names()
	 */
	 void list_names(std::vector<std::string>& names) const override;
    };

    
    /**
     * \brief A Composite Graphite Object. 
     * \details Each children has a unique name. 
     *  Functionalities are provided
     *  to retreive a child given its name or given its
     *  index. 
     */
    gom_class SCENE_GRAPH_API CompositeGrob : public Grob {
    public:

        /**
         * \brief CompositeGrob constructor.
         * \param[in] parent a pointer to the parent, or nil
         *  if there is no parent
         */
        CompositeGrob(CompositeGrob* parent);

    gom_properties:

	/**
	 * \brief Gets the Scope with the contained objects.
	 * \details Used by the interpreter.
	 */
	Scope* get_objects() const {
	    return new CompositeGrobScope(const_cast<CompositeGrob*>(this));
	}
	
    gom_slots:
        /**
         * \brief Tests wether an object of the specified name
         * exists in this CompositeGrob.
         * \param[in] name the name to be tested
         */
        bool is_bound(const std::string& name) const;
        
        /**
         * \brief Finds a child by name.
         * \param[in] name the name
         * \pre is_bound(name)
         */
        Grob* resolve(const std::string& name) const;

        /**
         * \brief Gets a child by index
         * \param[in] i index of the child
         * \return a pointer to the ith child
         * \pre i < get_nb_children()
         */
        Grob* ith_child(index_t i) const;

        /**
         * \brief Adds a child to this Grob
         * \param[in] child a pointer to the child to be added
         * \pre child class is a subclass of Grob
         */
         void add_child(Node* child) override;

        /**
         * \brief Removes a child from this Grob
         * \param[in] child a pointer to the child to be removed
         */
	 void remove_child(Node* child) override;

    public:
        
        /**
         * \copydoc Grob::bbox()
         */
         Box3d bbox() const override;

        /**
         * \copydoc Grob::world_bbox()
         */
	 Box3d world_bbox() const override;
    };

    
    /**
     * \brief An automatic reference-counted pointer to
     *  a CompositeGrob.
     */
    typedef SmartPointer<CompositeGrob> CompositeGrob_var;
    
//_________________________________________________________

}
#endif

