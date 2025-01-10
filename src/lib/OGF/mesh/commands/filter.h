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


#ifndef H_OGF_MESH_COMMANDS_FILTER_H
#define H_OGF_MESH_COMMANDS_FILTER_H

#include <OGF/mesh/common/common.h>

/**
 * \file OGF/mesh/commands/filter.h
 * \brief Filter class
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
         * \param[in] value the element value to be tested
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
}

#endif
