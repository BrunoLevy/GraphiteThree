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

#include <OGF/scene_graph_gfx/full_screen_effects/full_screen_effect.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/gom/interpreter/interpreter.h>
#include <geogram_gfx/full_screen_effects/full_screen_effect.h>

namespace OGF {

    FullScreenEffect::FullScreenEffect(
	SceneGraph* scene_graph
    ) : scene_graph_(scene_graph) {
    }

    void FullScreenEffect::update() {
        if(implementation() != nullptr) {
            implementation()->update();
        }
	if(scene_graph_->get_render_area() != nullptr) {
	    scene_graph_->get_render_area()->invoke_method("update");
	}
    }

    bool FullScreenEffect::set_property(
	const std::string& name, const Any& value
    ) {
	Interpreter* interpreter =
	    (scene_graph_ == nullptr) ? nullptr : scene_graph_->interpreter();
	return set_property_and_record_to_history(name, value, interpreter);
    }

    /***************************************************************/

    PlainFullScreenEffect::PlainFullScreenEffect(
	SceneGraph* sg
    ) : FullScreenEffect(sg) {
    }

    FullScreenEffectImpl* PlainFullScreenEffect::implementation() {
        return nullptr;
    }

}
