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

#include <OGF/gom/lua/lua_interpreter.h>
#include <OGF/gom/lua/interop.h>
#include <OGF/gom/lua/lua_graphite_object.h>
#include <OGF/gom/lua/vec_mat_interop.h>
#include <OGF/gom/types/connection.h>
#include <OGF/gom/types/node.h>
#include <OGF/gom/types/callable.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/reflection/meta_struct.h>
#include <OGF/basic/os/file_manager.h>
#include <OGF/basic/modules/modmgr.h>
#include <geogram/image/color.h>
#include <geogram/basic/process.h>
#include <geogram/lua/lua_io.h>

extern "C" {
#include <geogram/third_party/lua/lauxlib.h>
#include <geogram/third_party/lua/lualib.h>
}

#include <sstream>
#include <cctype>

/*************************************************************************/

namespace {
    using namespace OGF;

    /**
     * \brief Tests whether objects of a given type need quotes in Lua
     * \param[in] mtype a pointer to the meta type
     * \retval true if values need quotes, false otherwise
     */
    bool needs_quotes(const MetaType* mtype) {
	if(mtype == nullptr) {
	    return true;
	}
	const std::string& name = mtype->name();
	if(
	    name == "int" ||
	    name == "bool" ||
	    name == "float" ||
	    name == "double" ||
	    name == "Numeric::index_t" ||
	    name == "Numeric::int32" ||
	    name == "Numeric::uint32" ||
	    name == "Numeric::int64" ||
	    name == "Numeric::uint64" ||
	    name == "Numeric::float32" ||
	    name == "Numeric::float64" ||
	    name == "unsigned int" ||
	    name == "GEO::Color"
	) {
	    return false;
	}
	return true;
    }

    /**
     * \brief Tests whether a string has quotes
     * \param[in] s the string
     * \return true if \p s has quotes, false otherwise
     */
    bool is_quoted(const std::string& s) {
	if(s.length() < 2) {
	    return false;
	}
	return
	    (*s.begin() == '\'' && *s.rbegin() == '\'') ||
	    (*s.begin() == '\"' && *s.rbegin() == '\"') ;
    }

    /**
     * \brief Removes the quotes from a string
     * \pre has_quotes(s)
     * \param[in] s the string
     * \return the same string as \p s but without the quotes
     */
    std::string unquote(const std::string& s) {
	geo_debug_assert(is_quoted(s));
	return s.substr(1,s.length()-2);
    }
}

namespace OGF {
    using namespace OGF::GOMLua;

    LuaInterpreter::LuaInterpreter() {
	lua_state_ = luaL_newstate();
	luaL_openlibs(lua_state_);
	init_lua_graphite(this);
	init_lua_io(lua_state_);
    }

    LuaInterpreter::~LuaInterpreter() {
	lua_close(lua_state_);
    }

    void LuaInterpreter::reset() {
	lua_close(lua_state_);
	lua_state_ = luaL_newstate();
	luaL_openlibs(lua_state_);
	init_lua_graphite(this);
	init_lua_io(lua_state_);
    }

    bool LuaInterpreter::execute(
        const std::string& command, bool save_in_history, bool log
    ) {
	geo_lua_check_stack(lua_state_);
	bool result = true;

        if(log) {
            Logger::out("GOMLua") << command << std::endl;
        }

        if(save_in_history) {
            add_to_history(command);
        }

	if(luaL_dostring(lua_state_,command.c_str())) {
	    adjust_lua_state();
	    const char* msg = lua_tostring(lua_state_,-1);
	    display_error_message(msg);
	    result = false;
	}
        return result;
    }

    bool LuaInterpreter::execute_file(const std::string& file_name_in) {
	geo_lua_check_stack(lua_state_);
	std::string file_name = file_name_in;
        Environment::instance()->set_value("current_gel_file", file_name);
        if(!FileManager::instance()->find_file(file_name)) {
            Logger::err("GOMLua") << "Cannot find file \'"
				  << file_name << "\'" << std::endl;
            return false;
        }
	bool result = true;
	if(luaL_dofile(lua_state_,file_name.c_str())) {
	    adjust_lua_state();
	    const char* msg = lua_tostring(lua_state_,-1);
	    Logger::err("Lua") << msg << std::endl;
	    result = false;
	}
        return result;
    }

    void LuaInterpreter::bind(const std::string& id, const Any& value) {
	geo_lua_check_stack(lua_state_);
	lua_pushgraphiteval(lua_state_, value);
	lua_setglobal(lua_state_, id.c_str());
    }

    Any LuaInterpreter::resolve(const std::string& name, bool quiet) const {
	Any any_result;
	geo_lua_check_stack(lua_state_);

	Object* result = Interpreter::resolve_object_by_global_id(name, true);
	if(result != nullptr) {
	    any_result.set_value(result);
	    return any_result;
	}

	lua_getglobal(lua_state_, name.c_str());
	lua_tographiteval(lua_state_, -1, any_result);
	lua_pop(lua_state_,1);

	if(any_result.is_null() && !quiet) {
	    Logger::err("Lua") << name << ": no such global"
			       << std::endl;
	}
	return any_result;
    }

    Any LuaInterpreter::eval(const std::string& expression, bool quiet) const {
	geo_lua_check_stack(lua_state_);
	geo_argused(quiet);
	Any result;
	lua_pushfstring(lua_state_,"return %s", expression.c_str());
	// Note: luaL_dostring returns false on success (!)
	if(!luaL_dostring(lua_state_, lua_tostring(lua_state_,-1))) {
	    lua_tographiteval(lua_state_,-1,result);
	    lua_pop(lua_state_,1);
	}
	return result;
    }

    void LuaInterpreter::list_names(std::vector<std::string>& names) const {
	geo_lua_check_stack(lua_state_);
	names.clear();

	lua_pushglobaltable(lua_state_); // stack: -1=>table
	lua_pushnil(lua_state_);         // stack: -1=>nil  -2=>table
	while(lua_next(lua_state_,-2) != 0) {
	    // stack: -1=>key -2=>value -3=>table
	    // copy key so that lua_tostring does not modify the original
	    lua_pushvalue(lua_state_,-2);
	    // stack: -1=>key -2=>value -3=>key -4=>table
	    const char* key = lua_tostring(lua_state_,-1);
	    names.push_back(std::string(key));
	    lua_pop(lua_state_,2);
	    // stack: -1=>key -2=>table
	}
	// stack: -1=>table
	lua_pop(lua_state_,1);

	/*
	lua_pushglobaltable(lua_state_);
	int index = lua_gettop(lua_state_);
	lua_pushnil(lua_state_);
	while(lua_next(lua_state_,index) != 0) {
	    const char* key = lua_tostring(lua_state_,-2);
	    names.push_back(std::string(key));
	    lua_pop(lua_state_,1);
	}
	*/
    }

    void LuaInterpreter::record_invoke_in_history(
	Object* target, const std::string& slot_name, const ArgList& args
    ) {
	geo_lua_check_stack(lua_state_);
	std::string cmd_funccall = back_resolve(target);

	if(cmd_funccall == "") {
	    return; // could not find object
	}

	MetaSlot* mslot = target->meta_class()->find_slot(slot_name);

	cmd_funccall += ".";
	cmd_funccall += slot_name;
	std::string cmd_args;
	index_t nb_cmd_args=0;
	std::string first_arg_name = "";
	std::string first_arg_val = "";

	for(index_t i=0; i<args.nb_args(); ++i) {
	    std::string arg_name = args.ith_arg_name(i);
	    const MetaArg* marg = mslot->find_arg(arg_name);
	    MetaType* arg_meta_type = nullptr;
	    std::string arg_default_val;
	    if(marg != nullptr) {
		arg_meta_type = marg->type();
		arg_default_val = back_parse(
		    marg->default_value(), arg_meta_type
		);
	    }
	    std::string arg_val = back_parse(
		args.ith_arg_value(i), arg_meta_type
	    );

	    // Do not display parameter if equal to default value.
	    if(arg_default_val == arg_val) {
		continue;
	    }

	    if(cmd_args.length() > 0) {
		cmd_args += ", ";
	    }

	    cmd_args += args.ith_arg_name(i);
	    cmd_args += "=";
	    cmd_args += arg_val;
	    if(nb_cmd_args == 0) {
		first_arg_name = args.ith_arg_name(i);
		first_arg_val = arg_val;
	    }
	    ++nb_cmd_args;
	}

	std::string command = cmd_funccall;
	if(nb_cmd_args > 0) {
	    // Special case: there is a single argument, and it is the first
	    // one. Do not use name-value-pairs calls, to make history look
	    // nicer.
	    if(
		nb_cmd_args == 1 &&
		first_arg_name == mslot->ith_arg(0)->name()
	    ) {
		command += "(" + first_arg_val + ")";
	    } else {
		command += "{";
		command += cmd_args;
		command += "}";
	    }
	} else {
	    command += "()";
	}
	add_to_history(command);
    }

    void LuaInterpreter::record_set_property_in_history(
	Object* target, const std::string& prop_name, const Any& value
    ) {
	geo_lua_check_stack(lua_state_);

	if(!record_set_property_) {
	    return;
	}

	std::string target_name = back_resolve(target);
	if(target_name == "") {
	    return; // Could not find target
	}

	MetaProperty* mprop = target->meta_class()->find_property(prop_name);
	MetaType* mtype = nullptr;
	if(mprop != nullptr) {
	    mtype = mprop->type();
	}
	std::string val = back_parse(value,mtype);

	if(mprop != nullptr) {
	    if(needs_quotes(mprop->type())) {
		if(!is_quoted(val)) {
		    val = String::quote(val);
		}
	    } else {
		if(is_quoted(val)) {
		    val = unquote(val);
		}
	    }
	}

	// Compress multiple assignments to same property into a single
	// assignment in history.
	std::string command = target_name + "." + prop_name + " = " + val;
	if(
	    history_.size() != 0 &&
	    String::string_starts_with(
		*history_.rbegin(), target_name + "." + prop_name + " = "
	    )
	) {
	    *history_.rbegin() = command;
	    if(show_add_to_history_) {
		Logger::out("History") << command << std::endl;
	    }
	} else {
	    add_to_history(command);
	}
    }

    std::string LuaInterpreter::recursive_back_resolve(
	Object* haystack,
	const std::string& haystack_name,
	Object* needle
    ) const {
	geo_lua_check_stack(lua_state_);
	// Case 1: simple !
	if(haystack == needle) {
	    return haystack_name;
	}
	// Case 2: haystack is a Scope, traverse it recursively
	Scope* scope = dynamic_cast<Scope*>(haystack);
	if(
	    scope != nullptr &&
	    dynamic_cast<InterfaceScope*>(haystack) == nullptr &&
	    dynamic_cast<MetaTypesScope*>(haystack) == nullptr
	) {
	    std::vector<std::string> names;
	    scope->list_names(names);
	    for(const std::string& name: names) {
		Any any;
		any = scope->resolve(name);
		Object_var haystack2;
		if(
		    any.meta_type() != nullptr &&
		    any.meta_type()->is_subtype_of(ogf_meta<Object*>::type()) &&
		    any.get_value(haystack2)
		) {
		    bool name_does_not_need_quotes =
			isalpha(name[0]) || name[0] == '_';
		    for(char c: name) {
			name_does_not_need_quotes = name_does_not_need_quotes &&
			    (isalnum(c) || (c == '_'));
		    }
		    std::string haystack2_name = haystack_name;
		    if(name_does_not_need_quotes) {
			haystack2_name = haystack_name;
			if(haystack2_name != "") {
			    haystack2_name += ".";
			}
			haystack2_name += name;
		    } else {
			haystack2_name = haystack_name + "[\"" + name + "\"]";
		    }
		    std::string result = recursive_back_resolve(
			haystack2, haystack2_name, needle
		    );
		    if(result != "") {
			return result;
		    }
		}
	    }
	}
	// Case 3: haystack is an Object, traverse its properties of type Object*
	// (except the Scopes, we don't want to traverse Grob interfaces for
	// instance since we already have a mechanism to back_resolve interfaces
	// in LuaInterpreter::back_resolve()).
	MetaClass* mclass = haystack->meta_class();
	for(index_t i=0; i<mclass->nb_properties(false); ++i) {
	    MetaProperty* mprop = mclass->ith_property(i,false);
	    if(
		mprop->type()->is_subtype_of(ogf_meta<Object*>::type()) &&
		!mprop->type()->is_subtype_of(ogf_meta<Scope*>::type())
	    ) {
		Any haystack2_as_any;
		Object* haystack2;
		if(
		    haystack->get_property(mprop->name(), haystack2_as_any) &&
		    haystack2_as_any.get_value(haystack2) &&
		    haystack2 != nullptr
		) {
		    std::string haystack2_name =
			haystack_name + "." + mprop->name();
		    std::string result = recursive_back_resolve(
			haystack2, haystack2_name, needle
		    );
		    if(result != "") {
			return result;
		    }
		}
	    }
	}
	return "";
    }


    std::string LuaInterpreter::back_resolve(Object* object) const {
	geo_lua_check_stack(lua_state_);

	if(object == nullptr) {
	    return "nil";
	}

	// Tests whether an object is of a class, where class is given by name
	auto object_is_a = [](
	    Object* object, const std::string& class_name, bool strict = true
	)->bool {
	    MetaClass* mclass = Meta::instance()->resolve_meta_class(class_name);
	    if(object == nullptr || mclass == nullptr) {
		return false;
	    }
	    if(strict) {
		return object->is_a(mclass);
	    }
	    // Use object->meta_class()->is_subclass_of(xxx) rather than
	    // object->is_a(xxx) because is_a() uses C++ dynamic type
	    // that will not "see" commands created as DynamicObject /
	    // DynamicMetaClass.
	    return (object->meta_class()->is_subclass_of(mclass));
	};

	// Interface
	// Not strict (see comment in object_is_a above)
	if(object_is_a(object, "OGF::Interface", false)) {
	    Any grob_any;
	    Object* grob;
	    if(
		object->get_property("grob",grob_any) &&
		grob_any.get_value(grob) &&
		grob != nullptr
	    ) {
		std::string interface_name = object->meta_class()->name();

		// For legibility of the history,
		// remove prefix ("OGF::GrobClassName") and suffix ("Commands")

		// Remove "Commands" suffix only if there is no Interface class
		// with conflicting name (for instance, Transport and
		// TransportCommands in WarpDrive)
		if(String::string_ends_with(interface_name, "Commands")) {
		    std::string interface_name_without_commands =
			String::remove_suffix(interface_name, "Commands");
		    if(
			Meta::instance()->resolve_meta_type(
			    interface_name_without_commands
			) == nullptr
		    ) {
			interface_name = interface_name_without_commands;
		    }
		}

		// Same thing for removing "Commands" suffix
		if(String::string_ends_with(interface_name, "Interface")) {
		    std::string interface_name_without_interface =
			String::remove_suffix(interface_name, "Interface");
		    if(
			Meta::instance()->resolve_meta_type(
			    interface_name_without_interface
			) == nullptr
		    ) {
			interface_name = interface_name_without_interface;
		    }
		}

		interface_name = String::remove_prefix(
		    interface_name, grob->meta_class()->name()
		);

		std::string ctxt = back_resolve(grob);
		if(ctxt == "") {
		    return "";
		}
		return ctxt + ".I." + interface_name;
	    }
	    return "";
	}

	// Shader
	if(object_is_a(object,"OGF::Shader")) {
	    Any grob_any;
	    Object* grob;
	    if(
		object->get_property("grob",grob_any) &&
		grob_any.get_value(grob) &&
		grob != nullptr
	    ) {
		std::string ctxt = back_resolve(grob);
		if(ctxt == "") {
		    return "";
		}
		return ctxt + ".shader";
	    }
	    return "";
	}

	// Request
	if(object_is_a(object, "OGF::Request")) {
	    Request* request = dynamic_cast<Request*>(object);
	    geo_debug_assert(request != nullptr);
	    std::string ctxt = back_resolve(request->object());
	    if(ctxt == "") {
		return "";
	    }
	    return ctxt + "." + request->method()->name();
	}

	// StructPropertyRef
	if(object_is_a(object, "OGF::StructPropertyRef")) {
	    StructPropertyRef* struct_prop =
		dynamic_cast<StructPropertyRef*>(object);
	    geo_debug_assert(struct_prop != nullptr);
	    std::string ctxt = back_resolve(struct_prop->object());
	    if(ctxt == "") {
		return "";
	    }
	    return ctxt + "." + struct_prop->property_name();
	}

	// Not found, use brute force !
	{
	    std::string result = recursive_back_resolve(
		const_cast<LuaInterpreter*>(this)->get_globals(),"",object
	    );
	    if(result != "") {
		return result;
	    }
	}

	return "";
    }

    std::string LuaInterpreter::back_parse(
	const Any& value, MetaType* mtype
    ) const {

	geo_lua_check_stack(lua_state_);

	if(value.meta_type() == nullptr) {
	    return "nil";
	}

	// If stored value is a string, parse/unparse it to get a normalized
	// representation.
	if(
	    mtype != nullptr &&
	    mtype != ogf_meta<std::string>::type() &&
	    value.is_a(ogf_meta<std::string>::type())
	) {
	    std::string string_value;
	    if(value.get_value(string_value)) {

		// Find Grob by name in SceneGraph
		if(String::string_ends_with(mtype->name(),"GrobName")) {
		    MetaClass* msg = Meta::instance()->resolve_meta_class(
			"OGF::SceneGraph"
		    );
		    Object* scene_graph = msg->get_instance();
		    Any scope_as_any;
		    Scope* scope;
		    if(
			scene_graph != nullptr &&
			scene_graph->get_property("objects",scope_as_any)  &&
			scope_as_any.get_value(scope)
		    ) {
			scope->ref();
			Any o_as_any = scope->resolve(string_value);
			scope->unref();
			if(!o_as_any.is_null()) {
			    return back_parse(o_as_any, mtype);
			}
		    }
		}

		Any parsed;
		parsed.create(mtype);
		if(
		    Any::convert_from_string(
			mtype, string_value, parsed.data()
		    )
		) {
		    return back_parse(parsed,mtype);
		}
	    }
	}

	if(value.meta_type() == ogf_meta<Color>::type()) {
	    Color C;
	    if(value.get_value(C)) {
		return String::format(
		    "{ %f, %f, %f, %f }", C.r(), C.g(), C.b(), C.a()
		);
	    }
	}

	if(value.meta_type() == ogf_meta<vec3>::type()) {
	    vec3 p;
	    if(value.get_value(p)) {
		return String::format("{ %f, %f, %f }", p.x, p.y, p.z);
	    }
	}

	if(value.is_a(ogf_meta<Object*>::type())) {
	    Object* o;
	    if(value.get_value(o)) {
		return back_resolve(o);
	    }
	}

	std::string result;
	value.get_value(result);
	if(needs_quotes(value.meta_type())) {
	    result = String::quote(result);
	}
	return result;
    }

    void LuaInterpreter::adjust_lua_state() {
    }

    void LuaInterpreter::get_keys(
	const std::string& context, std::vector<std::string>& keys
    ) {
	geo_lua_check_stack(lua_state_);
	keys.clear();
	if(context != "") {
	    lua_pushfstring(lua_state_,"return %s", context.c_str());
	    luaL_dostring(lua_state_, lua_tostring(lua_state_,-1));
	    int index = lua_gettop(lua_state_);
	    bool done = false;
	    if(lua_istable(lua_state_, index)) {
		done = true;
		lua_pushnil(lua_state_);
		while(lua_next(lua_state_,index) != 0) {
		    if(lua_isstring(lua_state_,-2)) {
			const char* key = lua_tostring(lua_state_,-2);
			keys.push_back(std::string(key));
		    }
		    lua_pop(lua_state_,1);
		}
	    }
	    lua_pop(lua_state_,1);
	    if(done) {
		return;
	    }
	}
	Interpreter::get_keys(context, keys);
    }

    void LuaInterpreter::display_error_message(const std::string& err) {
	const char* msg = err.c_str();
	const char* msg2 = strchr(msg,']');
	if(msg2 != nullptr) {
	    msg = msg2+2;
	}
	Logger::err("Lua") << msg << std::endl;
    }

    std::string LuaInterpreter::name_value_pair_call(
	const std::string& args
    ) const {
	return '{' + args + '}';
    }
}
