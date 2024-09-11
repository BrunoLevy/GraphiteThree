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


#ifndef H_SCENE_GRAPH_SKIN_PREFERENCES_H
#define H_SCENE_GRAPH_SKIN_PREFERENCES_H

#include <OGF/scene_graph/common/common.h>

/**
 * \file OGF/scene_graph/skin/preferences.h
 * \brief Utility functions to manipulate preferences
 */

namespace OGF {

//_________________________________________________________

    /**
     * \brief Utility functions to manipulate preferences.
     */
    namespace Preferences {

        /**
         * \brief Saves the preferences.
         * \details Preferences are saved to home directory/graphite.ini
         */
        void SCENE_GRAPH_API save_preferences();

        /**
         * \brief Declares a variable as a preference variables.
         * \details Preference variables are stored in the GEO::Environment.
         *  Variables that are declared as preference variables are loaded/saved
         *  from/into the preferences file.
	 * \param[in] name the name of the variable, that should have already
	 *  been declared using CmdLine::declare_arg()
	 */
	void SCENE_GRAPH_API declare_preference_variable(const std::string& name);

        /**
         * \brief Declares a variable as a preference variables.
         * \details Preference variables are stored in the GEO::Environment.
         *  Variables that are declared as preference variables are loaded/saved
         *  from/into the preferences file. Note: value argument is declared
	 *  as const char* rather than const std::string& because this function
	 *  is used with string literals and the C++ compiler finds that it is
	 *  easier to convert a string literal to a boolean than to a
	 *  std::string.
         * \param[in] name name of the preference variable
	 * \param[in] value the initial value of the preference variable
	 * \param[in] help an optional description to be displayed when invoking
	 *  graphite /? or graphite -h
         */
        void SCENE_GRAPH_API declare_preference_variable(
	    const std::string& name, const char* value,
	    const std::string& help=""
	);

	/**
	 * \brief Tests whether a variable is declared as a preference variable.
	 * \see declare_preference_variable()
         * \param[in] name name of the preference variable
	 * \retval true if the preference variable is declared.
	 * \retval false otherwise.
	 */
	bool SCENE_GRAPH_API preference_variable_is_declared(
	    const std::string& name
	);

        /**
         * \brief Declares a variable as a preference variables.
         * \details Preference variables are stored in the GEO::Environment.
         *  Variables that are declared as preference variables are loaded/saved
         *  from/into the preferences file.
         * \param[in] name name of the preference variable
	 * \param[in] value the initial value of the preference variable
	 * \param[in] help an optional description to be displayed when invoking
	 *  graphite /? or graphite -h
         */
        void SCENE_GRAPH_API declare_preference_variable(
	    const std::string& name, int value, const std::string& help=""
	);

        /**
         * \brief Declares a variable as a preference variables.
         * \details Preference variables are stored in the GEO::Environment.
         *  Variables that are declared as preference variables are loaded/saved
         *  from/into the preferences file.
         * \param[in] name name of the preference variable
	 * \param[in] value the initial value of the preference variable
	 * \param[in] help an optional description to be displayed when invoking
	 *  graphite /? or graphite -h
         */
        void SCENE_GRAPH_API declare_preference_variable(
	    const std::string& name, bool value, const std::string& help=""
	);

    }

//_________________________________________________________

}

#endif
