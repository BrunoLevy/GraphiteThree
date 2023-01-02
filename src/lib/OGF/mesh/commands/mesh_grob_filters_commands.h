/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2009 INRIA - Project ALICE
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
 *  Contact: Bruno Levy - levy@loria.fr
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
 *
 * As an exception to the GPL, Graphite can be linked with the following 
 *  (non-GPL) libraries:  Qt, SuperLU, WildMagic and CGAL
 */
 

#ifndef H_OGF_MESH_COMMANDS_MESH_GROB_FILTER_COMMANDS_H
#define H_OGF_MESH_COMMANDS_MESH_GROB_FILTER_COMMANDS_H

#include <OGF/mesh/common/common.h>
#include <OGF/mesh/commands/mesh_grob_commands.h>

/**
 * \file OGF/mesh/commands/mesh_grob_filter_commands.h
 * \brief Commands that manipulate mesh filters.
 */

namespace OGF {
    /**
     * \brief A class to select subsets in an array
     */
    class MESH_API Filter {
    public:
        /**
         * \brief Filter constructor
         * \details Throws an exception if description cannot
         *  be parsed
         * \param[in] size size of the array
         * \param[in] description description of the subset, as
         *  a ';'-separated list of selections:
         *   - '*' selects all elements
         *   - 'nnn' selects an individual element
         *   - 'nnn-mmm' selects the interval [nnn,mmm]
         *   - '!nnn' unselects an individual element
         *   - '!nnn-mmm' unselects the interval [nnn,mmm]
         *  Example: "*;!5" selects everything but element number 5
         * \param[in] floating_point if set, values to be tested are
         *  floating point values, else they are element indices.
         */
        Filter(
            index_t size, const std::string& description,
            bool floating_point=false
        );

        /**
         * \brief Tests an element
         * \param[in] item the element to be tested
         * \retval true if the element is in the subset
         * \retval false otherwise
         */
        bool test(index_t item) const;

        /**
         * \brief Tests an element by value
         * \param[in] item the element to be tested
         * \retval true if the element is in the subset
         * \retval false otherwise
         */
        bool test(double value) const;

    protected:
        /**
         * \brief used in 'items' mode (ctor, floating_point = false)
         */
        void parse_items(const std::string& destription);

        /**
         * \brief used in 'values' mode (ctor, floating_point = true)
         */
        void parse_values(const std::string& destription);        
        
    private:
        index_t size_;
        vector<double> include_items_;
        vector<std::pair<double, double> > include_intervals_;
        vector<double> exclude_items_;
        vector<std::pair<double, double> > exclude_intervals_;
    };

    /*******************************************************************/
    
    /**
     * \brief Commands that manipulate mesh attributes.
     */
    gom_class MESH_API MeshGrobFiltersCommands : public MeshGrobCommands {
    public:
        /**
         * \brief MeshGrobAttributesCommands constructor.
         */
        MeshGrobFiltersCommands();

        /**
         * \brief MeshGrobAttributesCommands destructor.
         */
        ~MeshGrobFiltersCommands() override;
	
    gom_slots:

        /**
         * \brief sets a filter
         * \param[in] where one of vertices, facets, cells
         * \param[in] filter semi-column-separated list of 
         *  star,id,id1-id2,!id,!id1-id2
         * \param[in] propagate propagate filter to other elements
         */
        gom_arg_attribute(where, handler, "combo_box")
        gom_arg_attribute(where, values, "vertices;facets;cells")
        void set_filter(
            const std::string& where, const std::string& filter="*",
            bool propagate=true
        );

        /**
         * \brief adds subsets to a filter
         * \param[in] where one of vertices, facets, cells
         * \param[in] filter semi-column-separated list of 
         *  star,id,id1-id2,!id,!id1-id2
         * \param[in] propagate propagate filter to other elements
         */
        gom_arg_attribute(where, handler, "combo_box")
        gom_arg_attribute(where, values, "vertices;facets;cells")
        void add_to_filter(
            const std::string& where, const std::string& filter,
            bool propagate=true
        );

        /**
         * \brief removes subsets from a filter
         * \param[in] where one of vertices, facets, cells
         * \param[in] filter semi-column-separated list of 
         *  star,id,id1-id2,!id,!id1-id2
         * \param[in] propagate propagate filter to other elements
         */
        gom_arg_attribute(where, handler, "combo_box")
        gom_arg_attribute(where, values, "vertices;facets;cells")
        void remove_from_filter(
            const std::string& where, const std::string& filter,
            bool propagate=true
        );

        /**
         * \param[in] filter semi-column-separated list of 
         *  star,id,id1-id2,!id,!id1-id2
         * \param[in] propagate propagate filter to other elements
         */
	gom_arg_attribute(attribute, handler, "combo_box")
	gom_arg_attribute(attribute, values, "$grob.scalar_attributes")        
        void set_filter_from_attribute(
            const std::string& attribute, const std::string& filter,
            bool propagate=true
        );

        /**
         * \param[in] filter semi-column-separated list of 
         *  star,id,id1-id2,!id,!id1-id2
         * \param[in] propagate propagate filter to other elements
         */
	gom_arg_attribute(attribute, handler, "combo_box")
	gom_arg_attribute(attribute, values, "$grob.scalar_attributes")        
        void add_to_filter_attribute(
            const std::string& attribute, const std::string& filter,
            bool propagate=true
        );

        /**
         * \param[in] filter semi-column-separated list of 
         *  star,id,id1-id2,!id,!id1-id2
         * \param[in] propagate propagate filter to other elements
         */
	gom_arg_attribute(attribute, handler, "combo_box")
	gom_arg_attribute(attribute, values, "$grob.scalar_attributes")        
        void remove_from_filter_attribute(
            const std::string& attribute, const std::string& filter,
            bool propagate=true
        );
        
        
        /**
         * \brief propagates a filter from elements to all other
         *  elements (for instance, from cells to vertices and facets)
         * \param[in] from one of vertices, facets, cells
         */
        gom_arg_attribute(from, handler, "combo_box")
        gom_arg_attribute(from, values, "vertices;facets;cells")            
        void propagate_filter(const std::string& from);
        
        gom_arg_attribute(filter, handler, "combo_box")
        gom_arg_attribute(filter, values, "$grob.filters")
        void copy_filter_to_selection(const std::string& filter);

        gom_arg_attribute(selection, handler, "combo_box")
        gom_arg_attribute(selection, values, "$grob.selections")
        void copy_selection_to_filter(
            const std::string& selection, bool propagate=true
        );

        gom_arg_attribute(where, handler, "combo_box")
        gom_arg_attribute(where, values, "vertices;facets;cells;all")
        void delete_filters(const std::string& where="all");
    };
}

#endif
