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
#include <OGF/renderer/context/rendering_context.h>
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
        if(current_shader_ == nullptr) {
	    return;
	}

	RenderingContext* context = RenderingContext::current();
	FrameBufferObject* FBO = nullptr;
	FrameBufferObject* aux_FBO = nullptr;

	if(current_shader_->get_transparency() != Shader::TRANSP_OPAQUE) {
	    FBO = context->get_FBO(RenderingContext::MAIN_FBO);

	    FullScreenEffectImpl* fso = context->get_full_screen_effect();
	    if(fso != nullptr && fso->FBO() != nullptr) {
		FBO = fso->FBO();
	    }

	    aux_FBO = context->get_FBO(RenderingContext::AUX_FBO);

	    // Initialize aux FBO if not already initialized.
	    if(!aux_FBO->initialized()) {
		GEO_CHECK_GL();
		aux_FBO->initialize(
		    context->get_width(), context->get_height(),
		    true, // with depth buffer
		    GL_RGBA,
		    false // no mip-mapping
		);
		GEO_CHECK_GL();
	    }

	    // Bind aux FBO, clear color buffer and depth buffer
	    FBO->unbind();
	    aux_FBO->bind_as_framebuffer();
            glClearColor(0.0, 0.0, 0.0, 0.0);
	    glClear((GLbitfield)(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	    // Copy depth from FBO to aux_FBO
	    // note: need to keep GL_DEPTH_TEST activated, else
	    //  draw_unit_textured_quad(TEX_QUAD_DEPTH) does not update zbuffer

	    FBO->bind_depth_buffer_as_texture();
	    GLint vp[4];
	    glGetIntegerv(GL_VIEWPORT, vp);
	    glViewport(
		0,0,GLint(context->get_width()),GLint(context->get_height())
	    );
	    draw_unit_textured_quad(TEX_QUAD_DEPTH);
	    glViewport(vp[0],vp[1],vp[2],vp[3]);
	    FBO->unbind();
	    aux_FBO->bind_as_framebuffer();
	}

	current_shader_->draw() ;

	if(current_shader_->get_transparency() != Shader::TRANSP_OPAQUE) {
	    // Backup viewport transform
	    GLint vp[4];
	    glGetIntegerv(GL_VIEWPORT, vp);

	    // Draw aux_FBO contents with alpha-blending
	    aux_FBO->unbind();
	    FBO->bind_as_framebuffer();

	    glActiveTexture(GL_TEXTURE0);
	    aux_FBO->bind_as_texture();

	    glDepthMask(GL_FALSE);
	    glDisable(GL_DEPTH_TEST);
	    glEnable(GL_BLEND);
	    glBlendEquation(GL_FUNC_ADD);
	    switch(current_shader_->get_transparency()) {
	    case Shader::TRANSP_ACCUM:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	    case Shader::TRANSP_BLEND:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	    case Shader::TRANSP_OPAQUE:
		break;
	    }

	    glViewport(
		0,0,GLint(context->get_width()),GLint(context->get_height())
	    );

	    draw_unit_textured_quad();

	    // Restore everything so that the other shaders can work as
	    // expected.
	    aux_FBO->unbind();
	    glEnable(GL_DEPTH_TEST);
	    glDepthMask(GL_TRUE);
	    glDisable(GL_BLEND);
	    glViewport(vp[0],vp[1],vp[2],vp[3]);
	}
    }
}
