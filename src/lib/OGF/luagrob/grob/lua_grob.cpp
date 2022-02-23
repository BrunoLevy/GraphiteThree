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
 * As an exception to the GPL, Graphite can be linked with the 
 *  following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */
 

#include <OGF/luagrob/grob/lua_grob.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/scene_graph/types/scene_graph_library.h>
#include <OGF/scene_graph/types/geofile.h>
#include <OGF/gom/lua/lua_interpreter.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/interpreter/interpreter.h>

#include <geogram_gfx/lua/lua_glup.h>
#include <geogram/lua/lua_io.h>

extern "C" {
#include <geogram/third_party/lua/lauxlib.h>
#include <geogram/third_party/lua/lualib.h>
}

namespace {
    using namespace OGF;
    
    int wrapper_Update(lua_State* L) {
	if(lua_gettop(L) != 0) {
	    return luaL_error(
		L, "'GLUP.Update()' invalid number of arguments"
	    );
	}
	lua_getglobal(L,"this");
	Grob* thisgrob = static_cast<Grob*>(lua_touserdata(L,-1));
	lua_pop(L,1);
	Object* render_area = thisgrob->scene_graph()->get_render_area();
	if(render_area != nullptr) {
	    render_area->invoke_method("update");
	}
	return 0;
    }
}

namespace OGF {
    
    LuaGrob::LuaGrob(CompositeGrob* parent) :
        Grob(parent),
	autorun_(false)
    {
        initialize_name("lua");
	box_.add_point(vec3(0.0, 0.0, 0.0));
	box_.add_point(vec3(1.0, 1.0, 1.0));
	lua_shader_state_ = luaL_newstate();
	luaL_openlibs(lua_shader_state_);
	init_lua_io(lua_shader_state_);
	init_lua_glup(lua_shader_state_);

	lua_getglobal(lua_shader_state_, "GLUP");
	lua_pushliteral(lua_shader_state_,"Update");
	lua_pushcfunction(lua_shader_state_,wrapper_Update); 
	lua_settable(lua_shader_state_,-3);
	lua_pop(lua_shader_state_,1);

	lua_pushlightuserdata(lua_shader_state_,this);
	lua_setglobal(lua_shader_state_,"this");
	
	lua_error_occured_ = false;
	execute_shader_command("function draw() end");
    }

    LuaGrob::~LuaGrob() {
	lua_close(lua_shader_state_);
    }
    
    void LuaGrob::update() {
        Grob::update();
    }
    
    void LuaGrob::clear() {
	shader_source_ = "";
        update();
    }
    
    Grob* LuaGrob::duplicate(SceneGraph* sg) {
        LuaGrob* result = dynamic_cast<LuaGrob*>(Grob::duplicate(sg));
        ogf_assert(result != nullptr);
	result->box_ = box_;
	result->autorun_ = autorun_;
	result->source_ = source_;
	result->shader_source_ = shader_source_;
        result->update();
        return result;
    }
    
    Box3d LuaGrob::bbox() const {
	return box_;
    }

    
    void LuaGrob::set_box(const Box3d& box) {
	box_ = box;
    }
    
    LuaGrob* LuaGrob::find_or_create(
        SceneGraph* sg, const std::string& name
    ) {
        LuaGrob* result = find(sg, name);
        if(result == nullptr) {
            std::string cur_grob_bkp = sg->get_current_object();
            result = dynamic_cast<LuaGrob*>(
                sg->create_object("OGF::LuaGrob")
            );
            ogf_assert(result != nullptr);
            result->rename(name);
            sg->set_current_object(result->name());
            sg->set_current_object(cur_grob_bkp);
        }
        return result;
    }
    
    LuaGrob* LuaGrob::find(SceneGraph* sg, const std::string& name) {
        LuaGrob* result = nullptr;
        if(sg->is_bound(name)) {
            result = dynamic_cast<LuaGrob*>(sg->resolve(name));
        }
        return result;
    }

    bool LuaGrob::is_serializable() const {
        return true;
    }
    
    bool LuaGrob::serialize_read(InputGraphiteFile& in) {
        for(
            std::string chunk_class = in.next_chunk();
            chunk_class != "EOFL" && chunk_class != "SPTR";
            chunk_class = in.next_chunk()
        ) {
            if(chunk_class == "LUAH") {
                ArgList args;
                in.read_arg_list(args);
		double x1 = args.get_arg<double>("x1");
		double y1 = args.get_arg<double>("y1");
		double z1 = args.get_arg<double>("z1");
		double x2 = args.get_arg<double>("x2");
		double y2 = args.get_arg<double>("y2");
		double z2 = args.get_arg<double>("z2");
		box_.clear();
		box_.add_point(vec3(x1,y1,z1));
		box_.add_point(vec3(x2,y2,z2));
            } else if(chunk_class == "TEXT") {
		shader_source_ = in.read_string();
            } else if(chunk_class == "LUAG") {
		ArgList args;
		in.read_arg_list(args);
		autorun_ = args.get_arg<bool>("autorun");
		source_ = args.get_arg("program_source");
		shader_source_ = args.get_arg("shader_source");
	    }
        }
	execute_shader_program();
	if(autorun_) {
	    execute_program();
	}
        update();
        return true;
    }
    
    bool LuaGrob::serialize_write(OutputGraphiteFile& out) {
        ArgList args;

        args.create_arg("x1", box_.x_min());
        args.create_arg("y1", box_.y_min());
        args.create_arg("z1", box_.z_min());
        args.create_arg("x2", box_.x_max());
        args.create_arg("y2", box_.y_max());
        args.create_arg("z2", box_.z_max());
	
        out.write_chunk_header("LUAH", out.arg_list_size(args));
        out.write_arg_list(args);
        out.check_chunk_size();

	args.clear();
	
	args.create_arg("autorun",autorun_);
	args.create_arg("program_source",source_);
	args.create_arg("shader_source",shader_source_);
        out.write_chunk_header("LUAG", out.arg_list_size(args));
        out.write_arg_list(args);
        out.check_chunk_size();
	
        return true;
    }

    void LuaGrob::execute_program() {
	Interpreter::instance_by_language("Lua")->execute(
	    source_, false, false
	);
    }
    
    bool LuaGrob::execute_shader_command(const std::string& value) {
	if(luaL_dostring(lua_shader_state_,value.c_str())) {
	    adjust_lua_glup_state(lua_shader_state_);
	    const char* msg = lua_tostring(lua_shader_state_,-1);
	    LuaInterpreter::display_error_message(msg);
	    lua_error_occured_ = true;
	} else {
	    lua_error_occured_ = false;
	}
	update();
	return !lua_error_occured_;
    }

    void LuaGrob::execute_shader_program() {
	execute_shader_command(shader_source_);
    }

    void LuaGrob::load_program_source(const std::string& filename) {
	std::ifstream in(filename.c_str());
	if(in) {
	    in.seekg(0, std::ios::end);   
	    source_.reserve(size_t(in.tellg()));
	    in.seekg(0, std::ios::beg);
	    source_.assign(
		(std::istreambuf_iterator<char>(in)),
		std::istreambuf_iterator<char>()
            );
	} else {
	    Logger::err("LuaGrob") << "Could not open " << filename
				   << std::endl;
	}
    }
    
    void LuaGrob::save_program_source(const std::string& filename) {
	std::ofstream out(filename.c_str());
	if(out) {
	    out << source_;
	} else {
	    Logger::err("LuaGrob") << "Could not open " << filename
				   << std::endl;
	}
    }
    
    void LuaGrob::load_shader_source(const std::string& filename) {
	std::ifstream in(filename.c_str());
	if(in) {
	    in.seekg(0, std::ios::end);   
	    shader_source_.reserve(size_t(in.tellg()));
	    in.seekg(0, std::ios::beg);
	    shader_source_.assign(
		(std::istreambuf_iterator<char>(in)),
		std::istreambuf_iterator<char>()
            );
	} else {
	    Logger::err("LuaGrob") << "Could not open " << filename
				   << std::endl;
	}
	execute_shader_program();
    }

    void LuaGrob::save_shader_source(const std::string& filename) {
	std::ofstream out(filename.c_str());
	if(out) {
	    out << shader_source_;
	} else {
	    Logger::err("LuaGrob") << "Could not open " << filename
				   << std::endl;
	}
    }

    
}

