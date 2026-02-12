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

#include <OGF/scene_graph_gfx/shaders/shader_manager.h>
#include <OGF/scene_graph_gfx/shaders/scene_graph_shader_manager.h>
#include <OGF/scene_graph/types/scene_graph_library.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/scene_graph/grob/grob.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/interpreter/interpreter.h>

namespace OGF {

    ShaderManager::ShaderManager(
        Grob* grob, SceneGraphShaderManager* sg_shader_man
    ) {
        grob_ = grob ;
        sg_shader_manager_ = sg_shader_man ;
        current_shader_ = nullptr ;
	geo_assert(sg_shader_man->interpreter() != nullptr);
    }


    ShaderManager::~ShaderManager() {
    }

    void ShaderManager::update() {
        if(current_shader() != nullptr) {
            current_shader()->update() ;
        }
    }

    void ShaderManager::set_shader(const std::string& value) {
        auto it = shaders_.find(value) ;
        if(it == shaders_.end()) {
            ArgList args ;
            args.create_arg("grob", grob()) ;
            Object* o = sg_shader_manager_->interpreter()->create(value, args);
            if(o == nullptr) {
                Logger::err("ShaderMgr")
                    << "No factory for " << value << std::endl;
            }
            Shader* shader = dynamic_cast<Shader*>(o) ;
            if(shader != nullptr) {
                shaders_[value] = shader ;
            } else {
                Logger::err("ShaderMgr")
                    << o->meta_class()->name()
                    << " is not a shader" << std::endl;
            }
        }
        it = shaders_.find(value) ;
        if(it == shaders_.end()) {
            Logger::err("ShaderManager")
                << value << ": no such shader" << std::endl ;
            current_shader_ = nullptr ;
            return ;
        }
        current_shader_ = it->second ;
        grob_->scene_graph()->update() ;
    }

    std::string ShaderManager::get_shader() const {
        std::string result ;
        if(current_shader_ != nullptr) {
            result = current_shader_->meta_class()->name() ;
        }
        return result ;
    }

    void ShaderManager::draw() {
	if(grob() == nullptr || grob()->nb_graphics_locks_ > 0) {
	    return;
	}
        if(current_shader_ != nullptr) {
	    current_shader_->draw() ;
        }
    }
}
