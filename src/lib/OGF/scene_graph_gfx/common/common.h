
/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 INRIA - Project ALICE
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
 *  Contact for Graphite: Bruno Levy - Bruno.Levy@inria.fr
 *  Contact for this Plugin: Bruno Levy
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
 * (non-GPL) libraries:
 *     Qt, tetgen, SuperLU, WildMagic and CGAL
 */


#ifndef OGF_scene_graph_gfx_COMMON_COMMON
#define OGF_scene_graph_gfx_COMMON_COMMON

#include <OGF/basic/common/common.h>
#ifdef scene_graph_gfx_EXPORTS
#   define SCENE_GRAPH_GFX_API OGF_EXPORT
#else
#   define SCENE_GRAPH_GFX_API OGF_IMPORT
#endif

namespace OGF {
    static class SCENE_GRAPH_GFX_API scene_graph_gfx_libinit {
    public:
        scene_graph_gfx_libinit();
        ~scene_graph_gfx_libinit();

        static void increment_users();
        static void decrement_users();

    private:
        static void initialize();
        static void terminate();
        static int count_;
    } scene_graph_gfx_libinit_instance;
}

#endif
