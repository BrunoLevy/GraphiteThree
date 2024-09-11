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


#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/scene_graph/types/scene_graph_library.h>
#include <OGF/scene_graph/types/geofile.h>
#include <OGF/scene_graph/grob/grob.h>
#include <OGF/gom/interpreter/interpreter.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/services/factory.h>
#include <OGF/gom/interpreter/interpreter.h>
#include <OGF/basic/os/file_manager.h>
#include <OGF/basic/os/text_utils.h>
#include <OGF/basic/modules/modmgr.h>

#include <geogram/basic/file_system.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/basic/command_line.h>

#include <sstream>


namespace OGF {

/*************************************************************************/

    SceneGraph::SceneGraph(Interpreter* interpreter) :
	CompositeGrob(nullptr),
	interpreter_(
            interpreter != nullptr ? interpreter :
            Interpreter::default_interpreter()
        ),
	render_area_(nullptr),
	application_(nullptr) {
        Grob::scene_graph_ = this;
        SceneGraphLibrary::instance()->set_scene_graph(this);
    }

    SceneGraph::~SceneGraph() {
    }

    void SceneGraph::set_scene_graph_shader_manager(
	Object* sgshdmgr
    ) {
	scene_graph_shader_manager_ = sgshdmgr;
    }

    Object* SceneGraph::get_scene_graph_shader_manager() const {
	return scene_graph_shader_manager_;
    }

    void SceneGraph::set_visibility(index_t index, bool value) {
        Grob* g = ith_child(index);
        if(g != nullptr) {
            g->set_visible(value);
        }
    }

    void SceneGraph::register_grob_commands(
        MetaClass* grob_class, MetaClass* commands_class
    ) {
        SceneGraphLibrary::instance()->register_grob_commands(
            grob_class->name(), commands_class->name()
        );
    }

    void SceneGraph::set_visibilities(const std::string& x) {
        std::vector< std::string > values;
        String::split_string( x, ';', values );
        for( index_t i = 0; i < get_nb_children(); i++ ) {
            Grob* g = ith_child(i);
            if( g != nullptr ) {
                bool value = values[i] == "true" ? true : false;
                g->set_visible( value );
            }
        }
        update_values();
    }

    std::string SceneGraph::get_visibilities() const {
        std::string result;
        for(index_t i=0; i<get_nb_children(); i++) {
            Grob* g = ith_child(i);
            if(g != nullptr) {
                if(result.length() != 0) {
                    result += ";";
                }
                result = result + (g->get_visible() ? "true" : "false");
            }
        }
        return result;
    }

    std::string SceneGraph::get_types() const {
        std::string result;
        for(index_t i=0; i<get_nb_children(); i++) {
            Grob* g = ith_child(i);
            if(g != nullptr) {
                if(result.length() != 0) {
                    result += ";";
                }
                result = result + g->meta_class()->name();
            }
        }
        return result;
    }

    void SceneGraph::update_values() {
        values_changed(get_values());
        visibilities_changed(get_visibilities());
        types_changed(get_types());
        value_changed(this);
        if(this == SceneGraphLibrary::instance()->scene_graph()) {
            SceneGraphLibrary::instance()->
                scene_graph_values_changed_notify_environment();
        }
    }

    void SceneGraph::update() {
        value_changed(this);
    }


    void SceneGraph::begin_graphite_file(
        OutputGraphiteFile& out, bool all_scene
    ) {
        if(all_scene) {
            std::string version;
            Environment::instance()->get_value("version", version);

            // Scene graph header
            {
                ArgList args;
                args.create_arg("version", version);
                args.create_arg("current_object", get_current_object());

		// skin_imgui version

		copy_property_to_arglist("camera.auto_focus",args);
		copy_property_to_arglist("camera.draw_selected_only",args);
		copy_property_to_arglist("camera.clipping",args);
		copy_property_to_arglist("camera.lighting_matrix",args);

		copy_property_to_arglist("xform.u", args);
		copy_property_to_arglist("xform.v", args);
		copy_property_to_arglist("xform.w", args);
		copy_property_to_arglist("xform.zoom", args);
		copy_property_to_arglist("xform.look_at", args);

                out.write_scene_graph_header(args);
            }

        }

        // Command line
        {
            std::vector<std::string> args;
            CmdLine::get_args(args);
            out.write_command_line(args);
        }

        // History
        {
            std::vector<std::string> history(
                interpreter()->history_size()
            );
            for(index_t i=0;
                i<interpreter()->history_size(); ++i
            ) {
                history[i] = interpreter()->history_line(i);
            }
            out.write_history(history);
        }
    }

    void SceneGraph::end_graphite_file(OutputGraphiteFile& out) {
        geo_argused(out);
        Logger::out("GeoFile") << "<< EOF" << std::endl;
    }

    std::string SceneGraph::get_values() const {
        std::string result;
        for(index_t i=0; i<get_nb_children(); i++) {
            Grob* cur = ith_child(i);
            if(cur != nullptr) {
                if(result.length() > 0) {
                    result += ";";
                }
                result += cur->get_name();
            }
        }
        return result;
    }

    Grob* SceneGraph::duplicate_current() {
        Grob* cur = resolve(current_object_);
        set_current_object("",false);
        if(cur == nullptr) {
            Logger::err("SceneGraph") << "no object selected" << std::endl;
            return nullptr;
        }

        std::string copy_name = cur->name() + "_copy";
        Grob* dupl = cur->duplicate(this);
        dupl->rename(copy_name);
        grob_created(dupl->name());
        set_current_object(cur->name(),false);
	return dupl;
    }

    void SceneGraph::move_current_up() {
        if(current() == nullptr) {
            return;
        }
        Grob* prev = nullptr;
        for(index_t i=0; i+1<get_nb_children(); ++i) {
            if(ith_child(i+1) == current()) {
                prev = ith_child(i);
                break;
            }
        }
        if(prev == nullptr) {
            return;
        }
        swap_children(prev, current());
    }

    void SceneGraph::move_current_down() {
        if(current() == nullptr) {
            return;
        }
        Grob* next = nullptr;
        for(index_t i=0; i+1<get_nb_children(); ++i) {
            if(ith_child(i) == current()) {
                next = ith_child(i+1);
                break;
            }
        }
        if(next == nullptr) {
            return;
        }
        swap_children(next, current());
    }

    void SceneGraph::delete_current_object() {
        Grob* cur = resolve(current_object_);
        set_current_object(std::string(),false);
        if(cur != nullptr) {
            std::string name = cur->name();  // copy it before deletion
            cur->get_parent()->remove_child(cur);
            grob_deleted( name );
        }
        update_values();
        for(index_t i=0; i<get_nb_children(); i++) {
            Grob* cur_child = ith_child(i);
            if(cur_child != nullptr) {
                set_current_object(cur_child->name(),false);
                return;
            }
        }
        if(interpreter() != nullptr) {
            interpreter()->add_to_history(
                "scene_graph.delete_current_object()"
            );
        }
    }

    void SceneGraph::clear() {
        while(get_nb_children() != 0) {
            delete_current_object();
        }
    }

    void SceneGraph::set_current_object(const std::string& value) {
        set_current_object(value, true);
    }

    void SceneGraph::set_current_object(
        const std::string& value_in, bool record_history
    ) {
	std::string value = value_in;
        if(current_object_ == value) {
            return;
        }
	if(value != "" && resolve(value) == nullptr) {
	    Logger::warn("SceneGraph") << value << ": no such object"
				       << std::endl;
	    value = "";
	}
        current_object_ = value;
        disable_slots();
        current_object_changed(current_object_);
        enable_slots();
        if(record_history && interpreter() != nullptr) {
            std::ostringstream out;
            out << "scene_graph.current_object = \""
                << value << "\"" << std::endl;
            interpreter()->add_to_history(out.str());
        }
    }

    const std::string& SceneGraph::get_current_object() const {
        return current_object_;
    }

    Grob* SceneGraph::load_object(
        const FileName& file_name_in, const std::string& type,
        bool invoked_from_gui
    ) {

        std::string file_name = file_name_in;
#ifdef GEO_OS_WINDOWS
        FileSystem::flip_slashes(file_name);
#endif
        if(!FileSystem::is_file(file_name)) {
            Logger::err("SceneGraph")
                << "cannot open file: " << file_name << std::endl;
            return nullptr;
        }

        std::string extension = FileSystem::extension(file_name);

        if(extension == "aln") {
            load_aln(file_name, this);
            return this;
        }

        if(extension == "graphite" || extension == "graphite_ascii") {
            try {
                InputGraphiteFile in(file_name);
                serialize_read(in);
            } catch(const std::logic_error& e) {
                Logger::err("GeoFile") << "Caught exception: "
                                   << e.what() << std::endl;
            }
	    {
		Object* sgsm = get_scene_graph_shader_manager();
		if(sgsm != nullptr && current() != nullptr) {
		    ArgList args;
		    args.create_arg("value", current()->name());
		    sgsm->invoke_method("current_object",args);
		}
	    }
            return this;
        }

        std::string base_name = FileSystem::base_name(file_name);
        if(extension == "gz") {
            base_name = FileSystem::base_name(base_name);
        }


        std::vector<std::string> class_names;

        if(type == "default") {
            std::string class_name_str =
                SceneGraphLibrary::instance()->file_extension_to_grob(
                    extension
                );
            String::split_string(class_name_str, ';', class_names);
            if(class_names.size() == 0) {
                Logger::err("SceneGraph")
                    << "invalid extension: " << extension << std::endl;
                return nullptr;
            }
        } else {
            class_names.push_back(type);
        }

        Grob* result = nullptr;
        for(unsigned int i=0; i<class_names.size(); i++) {
            disable_signals();

            result = create_object(class_names[i]);
            enable_signals();
	    //   Commented-out, causes problems under Ubuntu:
            // triggers a GUI draw from
	    // a GUI draw (to be investigated...)
            // Logger::status() << "Loading object:" << base_name << std::endl;
            result->rename(base_name);
            result->set_filename(file_name);
            bool ok = result->load(file_name);

            update_values();
            grob_created( result->name() );
            set_current_object(result->name(),false);

            if(ok) {
                break;
            } else {
		result = nullptr;
                delete_current_object();
            }
        }

        if(interpreter() != nullptr) {
            std::ostringstream out;
            out << "scene_graph.load_object(\""
                << file_name << "\")" << std::endl;
            interpreter()->add_to_history(out.str());
        }

        if(invoked_from_gui) {
            const std::string dir = FileSystem::dir_name(file_name);
            if( FileSystem::is_directory(dir) ) {
                FileSystem::set_current_working_directory(dir);
            }
        }

        return result;
    }

    void SceneGraph::load_objects(
        const std::string& file_names_str, const std::string& type,
        bool invoked_from_gui
    ) {
        std::vector<std::string> file_names;
        String::split_string(file_names_str, ';', file_names);
        for(unsigned int i=0; i<file_names.size(); i++) {
            load_object(file_names[i],type,invoked_from_gui);
        }
    }

    bool SceneGraph::save_current_object(const NewFileName& file_name) {
        if(is_bound(current_object_)) {
            Grob* grob = resolve(current_object_);
            std::string ext = FileSystem::extension(file_name);
            if(ext == "graphite" || ext == "graphite_ascii") {
                if(!grob->is_serializable()) {
                    Logger::err("SceneGraph")
                        << "Cannot serialize current object"
                        << std::endl;
                    return false;
                }
                OutputGraphiteFile out(std::string(file_name).c_str());
                try {
                    begin_graphite_file(out,false);
                    serialize_grob_write(grob,out);
                    end_graphite_file(out);
                } catch(const std::logic_error& e) {
                    Logger::err("GeoFile") << "Caught exception: " << e.what()
                                       << std::endl;
		    return false;
                }
            } else {
                return grob->save(file_name);
            }
        }
	return true;
    }

    Grob* SceneGraph::create_object(
        const GrobClassName& class_name_in, const std::string& name
    ) {
	std::string class_name = class_name_in;
        if(class_name == "OGF::SceneGraph") {
            Logger::err("SceneGraph")
                << "Cannot create a SceneGraph in a SceneGraph !"
                << std::endl;
            return nullptr;
        }
        MetaType* mtype = Meta::instance()->resolve_meta_type(class_name);
        MetaClass* mclass = dynamic_cast<MetaClass*>(mtype);

	if(mclass == nullptr) {
	    if(class_name.find("::") == std::string::npos) {
		class_name = "OGF::" + class_name;
	    }
	    if(!String::string_ends_with(class_name, "Grob")) {
		class_name = class_name + "Grob";
	    }
	    mtype = Meta::instance()->resolve_meta_type(class_name);
	    mclass = dynamic_cast<MetaClass*>(mtype);
	}


        if(mclass == nullptr) {
            Logger::err("SceneGraph")
                << class_name_in << ": no such object class"
                << std::endl;
            return nullptr;
        }

        ArgList args;
        args.create_arg("parent",this);
        Grob* result = dynamic_cast<Grob*>(mclass->factory()->create(args));
        ogf_assert(result != nullptr);

	// Need to do that because if there are several levels of nested types,
	// then the stored meta class is not always the right one !
	result->set_meta_class(mclass);

        grob_created(result->name());
        update_values();
	if(name != "") {
	    result->rename(name);
	}
        set_current_object(result->name(),false);
        return result;
    }

    Grob* SceneGraph::find_or_create_object(
	const GrobClassName& classname, const std::string& name
    ) {
	Grob* result = dynamic_cast<Grob*>(resolve(name));
	if(result != nullptr) {
	    if(result->meta_class()->name() == std::string(classname)) {
		return result;
	    }
	    return nullptr;
	}
	result = create_object(classname);
	result->rename(name);
	set_current_object(name);
	return result;
    }

    Grob* SceneGraph::current() {
        return resolve(current_object_);
    }

    /**
     * Loading alignment data files (as in Aim@Shape repository)
     */
    bool SceneGraph::load_aln(const std::string& filename, SceneGraph* sg) {

        std::ifstream in(filename.c_str());
        if(!in) {
            return false;
        }

        std::string dirname = FileSystem::dir_name(filename);
        if(dirname == "") {
            dirname = ".";
        }

        unsigned int nb;
        in >> nb;
        for(unsigned int i=0; i<nb; i++) {
            std::string current;
            in >> current;
            current = dirname + "/" + current;
            Grob* grob = sg->load_object(current);

            in >> current; // Load '#' comment.

            mat4 M;
            in >> M;
            M = M.transpose();
              // Ehhh oui, transpose ! On avait une chance sur deux :-)

            if(grob != nullptr) {
                grob->set_obj_to_world_transform(M);
            }
        }

        return true;
    }

    bool SceneGraph::is_serializable() const {
        return true;
    }


    bool SceneGraph::serialize_read(
        InputGraphiteFile& in
    ) {
        Grob* grob = nullptr;
        std::string current_object;
        for(std::string chunk_class = in.current_chunk_class();
            chunk_class != "EOFL";
            chunk_class = in.next_chunk()) {
            if(chunk_class == "SCNG") {
                ArgList scene_graph_args;
                in.read_scene_graph_header(scene_graph_args);
                if(scene_graph_args.has_arg("current_object")) {
                    current_object = scene_graph_args.get_arg("current_object");
                }
		copy_arglist_to_properties(scene_graph_args);
            } else if(chunk_class == "GROB") {
                grob = serialize_grob_read(in);
            }
        }

        if(current_object != "") {
            set_current_object(current_object);
            grob = current();
        }

        if(grob != nullptr) {
            grob->update();
        }


        Logger::out("GeoFile") << ">> EOF" << std::endl;
        return true;
    }


    bool SceneGraph::serialize_write(
        OutputGraphiteFile& out
    ) {
        begin_graphite_file(out,true);
        for(index_t i=0; i<get_nb_children(); i++) {
            Grob* grob = ith_child(i);
            serialize_grob_write(grob,out);
        }
        end_graphite_file(out);
        return true;
    }

    bool SceneGraph::save_viewer_properties(const std::string& filename) {
	bool result = true;
	try {
	    OutputGraphiteFile out(filename);
	    begin_graphite_file(out,true);
	    end_graphite_file(out);
	} catch(const std::logic_error& e) {
	    Logger::err("I/O") << "Caught exception: " << e.what()
			       << std::endl;
	    result = false;
	}
	return result;
    }

    void SceneGraph::get_grob_shader(
        Grob* grob, std::string& classname, ArgList& properties
    ) {
	grob->get_shader_and_shader_properties(classname, properties, false);
        // last arg to false: do not copy properties of pointer type,
        // since they are not serializable

	/*
        classname = "";
        properties.clear();
        Object* sgsm = get_scene_graph_shader_manager();
	if(sgsm == nullptr) {
            properties.clear();
        } else {
	    sgsm->get_grob_shader(grob, classname, properties, false);
        }
        // last arg to false: do not copy properties of pointer type,
        // since they are not serializable
	*/
    }

    void SceneGraph::set_grob_shader(
        Grob* grob, const std::string& classname, const ArgList& properties
    ) {
	grob->set_shader_and_shader_properties(classname, properties);
	/*
        Object* sgsm = get_scene_graph_shader_manager();
        if(sgsm != nullptr) {
	    // TODO: cannot access this function from GOM
	    // sgsm->set_grob_shader(grob, classname, properties);
	}
	*/
    }

    void SceneGraph::serialize_grob_write(
        Grob* grob, OutputGraphiteFile& out
    ) {
        std::string classname = grob->meta_class()->name();

        if(grob->is_serializable()) {
            Logger::out("GeoFile")
                << "<< " << grob->name()
                << " (" << grob->meta_class()->name() << ")" << std::endl;

            // Grob header
            {
                ArgList grob_properties = grob->attributes();
                grob_properties.create_arg(
                    "class_name", grob->meta_class()->name()
                );
                grob_properties.create_arg("name", grob->name());
                out.write_grob_header(grob_properties);
            }

            // Shader
            {
                std::string shader_class_name;
                ArgList shader_properties;
                get_grob_shader(grob, shader_class_name, shader_properties);
                if(shader_class_name != "") {
                    shader_properties.create_arg(
                        "class_name", shader_class_name
                    );
                    shader_properties.create_arg(
                        "visible", grob->get_visible() ? "true" : "false"
                    );
                }
                out.write_shader(shader_properties);
            }
            grob->serialize_write(out);
            out.write_separator();
        } else {
            Logger::out("SceneGraph")
                << "Could not serialize " << grob->name()
                << "(" << grob->meta_class()->name() << ")" << std::endl;
        }
    }

    Grob* SceneGraph::serialize_grob_read(
        InputGraphiteFile& in
    ) {
        Grob* result = nullptr;

        geo_assert(in.current_chunk_class() == "GROB");
        ArgList grob_properties;
        in.read_grob_header(grob_properties);

        std::string grob_class_name = "";
        std::string grob_name = "unnamed";
        if(grob_properties.has_arg("class_name")) {
            grob_class_name = grob_properties.get_arg("class_name");
        }

        if(grob_properties.has_arg("name")) {
            grob_name = grob_properties.get_arg("name");
        }

        Logger::out("GeoFile")
            << ">> " << grob_name
            << " (" << grob_class_name << ")" << std::endl;
        disable_signals();
        result = create_object(grob_class_name);


        // Not to be copied in grob attributes.
        index_t i = grob_properties.find_arg_index("name");
        if(i != index_t(-1)) {
            grob_properties.delete_ith_arg(i);
        }
        i = grob_properties.find_arg_index("class_name");
        if(i != index_t(-1)) {
            grob_properties.delete_ith_arg(i);
        }

        result->attributes() = grob_properties;
        enable_signals();

        in.next_chunk();
        if(in.current_chunk_class() != "SHDR") {
            Logger::err("GeoFile") << "Got " << in.current_chunk_class()
                                   << " instead of SHDR chunk" << std::endl;
            Logger::err("GeoFile") << "Skipping object" << std::endl;
            while(in.current_chunk_class() != "SPTR") {
                in.next_chunk();
            }
            return nullptr;
        }

        ArgList shader_properties;
        in.read_shader(shader_properties);

        bool visible = true;
        if(shader_properties.has_arg("visible")) {
            visible = (shader_properties.get_arg("visible") == "true");
        }

        if(result != nullptr) {
            result->rename(grob_name);
            result->serialize_read(in);
            result->set_visible(visible);
            update_values();
            grob_created( result->name() );
            set_current_object(result->name(),false);

            std::string shader_class_name;
            if(shader_properties.has_arg("class_name")) {
                shader_class_name = shader_properties.get_arg("class_name");
            }
            if(shader_class_name != "") {
                set_grob_shader(result, shader_class_name, shader_properties);
            }
        }
        return result;
    }

    void SceneGraph::copy_property_to_arglist(
	const std::string& obj_prop_name, ArgList& args
    ) {
	std::string obj_name;
	std::string prop_name;
	bool OK = String::split_string(obj_prop_name, '.', obj_name, prop_name);
	geo_assert(OK);
	Object* obj = interpreter()->resolve_object(obj_name);
	if(obj != nullptr) {
	    std::string prop_val;
	    if(obj->get_property(prop_name, prop_val)) {
		args.create_arg(obj_prop_name, prop_val);
	    } else {
		Logger::warn("Geofile")
		    << obj_prop_name << " not found" << std::endl;
	    }
	} else {
	    Logger::warn("Geofile") << obj_name << " not found" << std::endl;
	}
    }

    void SceneGraph::copy_arglist_to_properties(
	const ArgList& args
    ) {
	for(index_t i=0; i<args.nb_args(); ++i) {
	    std::string obj_prop_name = args.ith_arg_name(i);
	    std::string prop_val = args.ith_arg_value(i).as_string();
	    std::string obj_name;
	    std::string prop_name;
	    if(String::split_string(obj_prop_name, '.', obj_name, prop_name)) {
		Object* obj = interpreter()->resolve_object(obj_name);
		if(obj != nullptr) {
		    obj->set_property(prop_name, prop_val);
		} else {
		    Logger::warn("Geofile") << obj_name
					    << " not found" << std::endl;
		}
	    }
	}
    }

    Interpreter* SceneGraph::interpreter() {
	return interpreter_;
    }

}
