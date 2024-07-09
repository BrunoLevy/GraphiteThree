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
 
 
#include <OGF/scene_graph/grob/grob.h>
#include <OGF/scene_graph/grob/composite_grob.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/scene_graph/types/geofile.h>
#include <OGF/scene_graph/commands/commands.h>
#include <OGF/scene_graph/shaders/shader.h>
#include <OGF/scene_graph/shaders/shader_manager.h>
#include <OGF/scene_graph/types/scene_graph_shader_manager.h>
#include <OGF/basic/math/geometry.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/interpreter/interpreter.h>

#include <geogram/basic/file_system.h>
#include <sstream>

namespace OGF {

//_________________________________________________________

    Grob::Grob(
        CompositeGrob* parent
    ) : Node(parent) {
        if(parent != nullptr) {
            scene_graph_ = parent->scene_graph();
        }
        filename_ = "";
        visible_ = true;
        obj_to_world_.load_identity();
        dirty_ = false;
        nb_graphics_locks_ = 0;
    }

    Grob::~Grob() {   
        // Note: this grob is removed from the attribute manager in
        // the remove_child() function of CompositeGrob
    }

    void Grob::initialize_name(const std::string& name) {
	CompositeGrob* parent = dynamic_cast<CompositeGrob*>(get_parent());
        if((parent != nullptr) && parent->is_bound(name)) {
            int id=0;
            std::string cur_name;
            do {
                id++;
                std::ostringstream s;
                s << name << "_" << id;
                cur_name = s.str();
            } while(parent->is_bound(cur_name));
            name_ = cur_name;
        } else {
            name_ = name;
        }
    }

    void Grob::rename(const std::string& value) {
        initialize_name(value);
        if(scene_graph() != this) {
            scene_graph()->update_values();
            scene_graph()->grob_renamed();
        }
    }
    
    Grob* Grob::duplicate(SceneGraph* sg) {
        Grob* result = sg->create_object(this->meta_class()->name());
        result->attributes() = attributes();
        return result;
    }

    // An approximate version of world_bbox that
    // uses the (transformed) local bbox
    Box3d Grob::world_bbox() const
    {
        Box3d lb = this->bbox();
        Box3d result;
        const mat4& M = get_obj_to_world_transform();
        if( M.is_identity() ) {
            return lb;
        }
        vec3 p = vec3(lb.xyz_min); 
        vec3 d = vec3(lb.xyz_max);
        result.add_point(transform_point(p, M));
        p.x += d.x;
        result.add_point(transform_point(p, M));
        p.x -= d.x; p.y += d.y;
        result.add_point(transform_point(p, M));
        p.x += d.x;
        result.add_point(transform_point(p, M));
        p.x -= d.x; p.y -= d.y; p.z += d.z;
        result.add_point(transform_point(p, M));
        p.x += d.x;
        result.add_point(transform_point(p, M));
        p.x -= d.x; p.y += d.y;
        result.add_point(transform_point(p, M));
        p.x += d.x;
        result.add_point(transform_point(p, M));
        return result;
    }

    Grob* Grob::find(SceneGraph* sg, const std::string& name) {
        Grob* result = nullptr;
        if(sg->is_bound(name)) {
            result = dynamic_cast<Grob*>(sg->resolve(name));
        }
        return result;
    }

    void Grob::update() {
        dirty_ = true;
        value_changed(this);
        scene_graph()->update();
    }

    void Grob::redraw() {
	update();
	if(scene_graph()->get_application() != nullptr) {
	    scene_graph()->get_application()->invoke_method("draw");
	}
    }

    Object* Grob::query_interface(const std::string& name_in) {
        std::string name = meta_class()->name() + name_in + "Commands";
        ArgList args;
        Object* o = interpreter()->create(name, args);
        if(o == nullptr) {
            name = meta_class()->name() + name_in;
            o = interpreter()->create(name, args);           
        }
        if(o == nullptr) {
            name = name_in;
            o = interpreter()->create(name, args);
        }
        if(o == nullptr) { return nullptr; }
        if(o->meta_class()->find_property("grob") != nullptr) {
            args.clear();
            args.create_arg("grob", this);
            o->set_properties(args);
        }
        return o;
    }

    bool Grob::is_serializable() const {
        return false;
    }
    
    bool Grob::serialize_read(InputGraphiteFile& in) {
        geo_argused(in);
        Logger::out("Grob") << "Cannot read from stream" 
                            << std::endl;
        return false;
    }
    
    bool Grob::serialize_write(OutputGraphiteFile& out) {
        geo_argused(out);        
        Logger::out("Grob") << "Cannot write to stream"
                            << std::endl;
        return false;
    }

    bool Grob::load(const FileName& value) {
        if(is_serializable()) {
            bool result = false;
            try {
                InputGraphiteFile in(value);
                result = serialize_read(in);
            } catch(const std::logic_error& e) {
                Logger::err("I/O") << "Caught exception: " << e.what()
                                   << std::endl;
                result = false;
            }
            return result;
        }
        return false;
    }
    
    bool Grob::save(const NewFileName& value) {
	if(
	    FileSystem::extension(value) == "graphite" &&
	    scene_graph() != this
	) {
	    std::string bkp = scene_graph()->get_current_object();
	    scene_graph()->set_current_object(name());
	    bool result = scene_graph()->save_current_object(value);
	    scene_graph()->set_current_object(bkp);
	    return result;
	}
	
        if(is_serializable()) {
            bool result = false;
            try {
                OutputGraphiteFile out(value);
                result = serialize_write(out);
            } catch(const std::logic_error& e) {
                Logger::err("I/O") << "Caught exception: " << e.what()
                                   << std::endl;
                result = false;
            }
            return result;
        }
        return false;
    }

    void Grob::clear() {
    }

    bool Grob::append(const FileName& value) {
	geo_argused(value);
	Logger::err("GOM") << "append() not implemented"
			   << std::endl;
	return false;
    }
    
    bool Grob::get_visible() const {
        return visible_;
    }
    
    void Grob::set_visible(bool x) {
	if(x != visible_) {
	    visible_ = x;
	    update();
	}
    }

    Shader* Grob::get_shader() const {
        if(shader_manager_ == nullptr) {
            return nullptr;
        }
        return shader_manager_->current_shader();
    }

    void Grob::set_shader(const std::string& shader_classname) {
        if(shader_manager_ == nullptr) {
            return;
        }
        SceneGraphShaderManager* sg_shader_mgr =
            shader_manager_->scene_graph_shader_manager();
        if(sg_shader_mgr == nullptr) {
            return;
        }
        Any ret_val;
        ArgList args;
        args.create_arg("value", shader_classname);
        sg_shader_mgr->invoke_method("shader", args, ret_val);
    }

    Interpreter* Grob::interpreter() {
	return scene_graph()->interpreter();
    }
    
//_________________________________________________________

}

