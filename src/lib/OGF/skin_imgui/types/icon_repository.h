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

#ifndef H_SKIN_IMGUI_ICON_REPOSITORY_H
#define H_SKIN_IMGUI_ICON_REPOSITORY_H

#include <OGF/skin_imgui/common/common.h>
#include <geogram_gfx/third_party/imgui/imgui.h>
#include <geogram_gfx/basic/GL.h> 

#include <map>
#include <set>
#include <string>

/**
 * \file OGF/skin_imgui/types/icon_repository.h
 * \details A class that keeps the correspondance between names and icons.
 */
namespace OGF {

//___________________________________________________________________________

    /**
     * \brief Keeps the correspondence between names and cached icons.
     */
    class SKIN_IMGUI_API IconRepository {
    public:

        /**
         * \brief IconRepository constructor.
         */
        IconRepository();

        /**
         * \brief IconRepository destructor.
         */
        ~IconRepository();

        /**
         * \brief Attaches an icon to a given name.
         * \details If the specified name is already bound, then an
         *  error message is displayed.
         * \param[in] icon_name the name of the icon
         * \param[in] gl_texture the OpenGL texture that 
	 *   corresponds to the icon.
         */
        void bind_icon(const std::string& icon_name, GLuint gl_texture);

        /**
         * \brief Finds an icon by name.
         * \details The icon is searched in the already-bound icons and in
         *  the lib/icons/ subdirectories of the OGF_PATH. If it is not
         *  found, then it is replaced with the "on_icon" icon (a red cross).
         * \param[in] icon_name the name of the icon
	 * \param[in] mipmap if true, and if the icon was not already in the 
	 *  repository, create mipmaps.
         * \return the ImTextureID that corresponds to the icon.
         */
        ImTextureID resolve_icon(
	    const std::string& icon_name, bool mipmap=false
	) const;
    
    private:
	typedef union {
	    ImTextureID im_texture_id;
	    GLuint gl_texture_id;
	} Icon;
	
        std::map<std::string, Icon> icons_;
	
	/** \brief To notify not found icons only once. */
	mutable std::set<std::string> not_found_;
    };

//___________________________________________________________________________

}

#endif

