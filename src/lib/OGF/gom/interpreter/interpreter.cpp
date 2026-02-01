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

#include <OGF/gom/interpreter/interpreter.h>
#include <OGF/gom/types/connection.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/types/callable.h>
#include <OGF/basic/os/file_manager.h>
#include <OGF/basic/modules/modmgr.h>

#include <geogram/basic/file_system.h>

#include <fstream>

namespace OGF {

    std::map<std::string, SmartPointer<Interpreter> > Interpreter::instance_;
    std::map<std::string, Interpreter* >
       Interpreter::instance_by_file_extension_;
    Interpreter* Interpreter::default_interpreter_ = nullptr;

    Interpreter::Interpreter() {
	globals_ = new GlobalScope(this);
        MetaTypesScope* meta_types = new MetaTypesScope();
        MetaTypesScope* OGF = meta_types->create_subscope("OGF");
        OGF->create_subscope("NL");
        OGF->create_subscope("Numeric");
        OGF->create_subscope("Memory");
        meta_types->create_subscope("std");
        meta_types_ = meta_types;
        if(default_interpreter_ == nullptr) {
            default_interpreter_ = this;
        }
	record_set_property_ = false;
	show_add_to_history_ = false;
    }

    void Interpreter::initialize(
	Interpreter* instance, const std::string& language,
	const std::string& extension
    ) {
	instance->set_language(language);
	instance->set_filename_extension(extension);
        instance_[language] = instance;
	instance_by_file_extension_[extension] = instance;
    }

    void Interpreter::terminate(
        const std::string& language, const std::string& extension
    ) {
	{
	    auto it = instance_.find(language);
	    if(it == instance_.end()) {
		Logger::err("gom") << language << ": no such language"
				   << std::endl;
	    } else {
		instance_.erase(it);
	    }
	}
	{
	    auto it = instance_by_file_extension_.find(extension);
	    if(it == instance_by_file_extension_.end()) {
		Logger::err("gom")
		    << extension << ": no such language file extension"
		    << std::endl;
	    } else {
		instance_by_file_extension_.erase(it);
	    }
	}
    }

    void Interpreter::terminate() {
    }

    void Interpreter::save_history(const std::string& file_name) const {
        std::ofstream out(file_name.c_str()) ;
        for(unsigned int i=0; i<history_.size(); i++) {
            out << history_[i] << std::endl ;
        }
    }

    std::string Interpreter::get_history() const {
	std::string result;
        for(unsigned int i=0; i<history_.size(); i++) {
            result += history_[i];
	    result += "\n";
        }
	return result;
    }

    void Interpreter::clear_history() {
        history_.clear() ;
    }

    bool Interpreter::execute_file(const std::string& file_name) {
        std::ifstream in(file_name.c_str()) ;
        if(!in) {
            Logger::err("Interpreter")
                << "cannot open file: " << file_name << std::endl ;
            return false;
        }
        std::string line;
        while(std::getline(in,line)) {
            if (!execute(line, false)){
                return false;
            }
        }
        return true;
    }

    Interpreter::~Interpreter() {
    }

    void Interpreter::add_to_history(const std::string& command_in) {
	std::string command = command_in;
        if(command != "") {
            if(*command.rbegin() == '\n') {
                command = command.substr(0,command.length()-1);
	    }
	    history_.push_back(command);
	    if(show_add_to_history_) {
		Logger::out("History") << command << std::endl;
	    }
        }
    }

    void Interpreter::record_invoke_in_history(
	Object* target, const std::string& slot_name, const ArgList& args
    ) {
	geo_argused(target);
	geo_argused(slot_name);
	geo_argused(args);
    }

    void Interpreter::record_set_property_in_history(
	Object* target, const std::string& prop_name, const Any& value
    ) {
	geo_argused(target);
	geo_argused(prop_name);
	geo_argused(value);
    }

    std::string Interpreter::back_resolve(Object* object) const {
	geo_argused(object);
	return "";
    }

    std::string Interpreter::back_parse(const Any& value) const {
	std::string result;
	value.get_value(result);
	return String::quote(result);
    }

/*******************************************************************/

    // Note: we systematically use type_name() rather than type()->name()
    // since some MetaTypes may not exist in GOM (such as ArgList, used
    // by Object::set_properties())

    static void inspect_property(Object* object, MetaProperty* mprop) {
        std::string readonly = "";
        std::string second_member = "";
        if(mprop->read_only()) {
            readonly = "readonly ";
        }
        if(object){
            std::string value = "";
            std::ostringstream oss;
            object->get_property(mprop->name(), value);
            if(mprop->type_name() == "string") {
                value = "\"" + value + "\"";
            }
            oss << " = " << value;
            second_member = oss.str();
        }

        Logger::out("GOM") << "   "
                           << readonly
                           << mprop->type_name() << " "
                           << mprop->name()
                           << second_member << ";" << std::endl;
    }


    void Interpreter::inspect_method(Object* object, MetaMethod* mmethod) {
	geo_argused(object);
	Interpreter::inspect_meta_method(mmethod);
    }

    void Interpreter::inspect_meta_class(Object* object, MetaClass* mclass) {
        std::string is_obj = "";
        if(object){
            is_obj = "GOM object, ";
        }
        Logger::out("GOM") << is_obj << "class = "
                           << mclass->name()
                           << " : "
                           << mclass->super_class_name()
                           << " { " << std::endl;

        {
            Logger::out("GOM") << "properties:" << std::endl;
            std::vector<MetaProperty*> properties;
            mclass->get_properties(properties);
            for(unsigned int i=0; i<properties.size(); i++) {
                inspect_property(object, properties[i]);
            }
        }

        {
            Logger::out("GOM") << "constructors:" << std::endl;
            std::vector<MetaConstructor*> constructors;
            mclass->get_constructors(constructors);
            for(unsigned int i=0; i<constructors.size(); i++) {
                inspect_method(object, constructors[i]);
            }
        }

        {
            Logger::out("GOM") << "slots:" << std::endl;
            std::vector<MetaSlot*> slots;
            mclass->get_slots(slots);
            for(unsigned int i=0; i<slots.size(); i++) {
                inspect_method(object, slots[i]);
            }
        }

        {
            Logger::out("GOM") << "signals:" << std::endl;
            std::vector<MetaSignal*> signals;
            mclass->get_signals(signals);
            for(unsigned int i=0; i<signals.size(); i++) {
                inspect_method(object, signals[i]);
            }
        }

        Logger::out("GOM") << "};" << std::endl;
    }

    void Interpreter::inspect(Object* object) {
	if(object == nullptr) {
	    Logger::out("GOM") << "nil GOM object" << std::endl;
	} else {
	    inspect_meta_class(object, object->meta_class());
	}
    }

    void Interpreter::inspect_meta_type(MetaType* meta_type) {
	MetaClass* mclass = dynamic_cast<MetaClass*>(meta_type);
	MetaMethod* mmethod = dynamic_cast<MetaMethod*>(meta_type);
	if(mclass != nullptr) {
	    inspect_meta_class(nullptr, mclass);
	} else if(mmethod != nullptr) {
	    inspect_meta_method(mmethod);
	} else if(meta_type != nullptr) {
	    Logger::out("GOM") << meta_type->name() << std::endl;
	} else {
	    Logger::out("GOM") << "nil" << std::endl;
	}
    }

    void Interpreter::inspect_meta_method(MetaMethod* mmethod) {

        std::ostringstream out;

        if(dynamic_cast<MetaConstructor*>(mmethod) != nullptr) {
            out << "   " << mmethod->return_type()->name() << "(";
        } else {
            out << "   "
                << mmethod->return_type_name() << " "
                << mmethod->name() << "(";
        }

        for(index_t i=0; i<mmethod->nb_args(); i++) {
            const MetaArg* marg = mmethod->ith_arg(i);
            out << marg->type_name() << " " << marg->name();
            if(marg->has_default_value()) {
                std::string default_value = marg->default_value().as_string();
                if(marg->type_name() == "std::string") {
                    default_value = "\"" + default_value + "\"";
                }
                out << "=" << default_value;
            }
            if(i != mmethod->nb_args() - 1) {
                out << ", ";
            }
        }

        out << ");";

        Logger::out("GOM") << out.str() << std::endl;
    }


    void Interpreter::list_classes() {
        Logger::out("GOM") << "List of all known MetaClasses:" << std::endl;

        std::vector<MetaType*> all_types;
        Meta::instance()->list_types(all_types);
        for(MetaType* cur : all_types) {
            MetaClass* cur_class = dynamic_cast<MetaClass*>(cur);
            if(cur_class != nullptr) {
                std::string abstraction = "";
                if(cur_class->is_abstract())
                    abstraction = " (abstract)";

                std::string superclass = cur_class->super_class_name();
                if(superclass != "") {
                    superclass = std::string(": ") + superclass;
                }

                Logger::out("GOM")
                    << cur_class->name() << " "
                    << superclass << " "
                    << abstraction
                    << std::endl;
            }
        }
    }

    /**********************************************************************/

    void Interpreter::set_environment_value(
        const std::string& name, const std::string& value
    ) {
	Environment::instance()->set_value(name, value);
    }

    std::string Interpreter::get_environment_value(
        const std::string& name
    ) {
        std::string result;
        if(Environment::instance()->has_value(name)) {
            result = Environment::instance()->get_value(name);
        }
        return result;
    }

    void Interpreter::out(
	const std::string& message, const std::string& tag
    ) {
	Logger::out(tag) << message << std::endl;
    }

    void Interpreter::err(
	const std::string& message, const std::string& tag
    ) {
	Logger::err(tag) << message << std::endl;
    }

    void Interpreter::warn(
	const std::string& message, const std::string& tag
    ) {
	Logger::warn(tag) << message << std::endl;
    }

    void Interpreter::status(const std::string& message) {
	Logger::status() << message << std::endl;
    }

    bool Interpreter::load_module(const std::string& module_name) {
	return ModuleManager::instance()->load_module(module_name);
    }

    void Interpreter::append_dynamic_libraries_path(const std::string& path) {
	ModuleManager::append_dynamic_libraries_path(path);
    }

    Connection* Interpreter::connect(Request* from, Callable* to) {
	// Special case: target is a Request.
	// We create a SlotConnection, that does not do reference counting
	// to target, else this creates circular references preventing
	// objects from being garbage-collected.
	Request* rq = dynamic_cast<Request*>(to);
	if(rq != nullptr) {
	    return new SlotConnection(
		from->object(), from->method()->name(),
		rq->object(), rq->method()->name()
	    );
	}
	return new CallableConnection(
	    from->object(), from->method()->name(), to
	);
    }

    void Interpreter::bind_object(const std::string& id, Object* object) {
	Any value;
	value.set_value(object);
	bind(id,value);
    }

    Object* Interpreter::resolve_object_by_global_id(
	const std::string& id, bool quiet
    ) const {
	Object* result = nullptr;
        if(id.length() > 0 && id[0] == '@') {
            size_t sept = id.find("::#");
            if(sept != std::string::npos) {
                std::string classname = id.substr(1, sept-1);
                std::string index_str =
                    id.substr(sept + 3, id.length() - (sept + 3));
                unsigned int index;
                ogf_convert_from_string(index_str, index);
                MetaClass* mclass = dynamic_cast<MetaClass*>(
                    Meta::instance()->resolve_meta_type(classname)
                );
                if(mclass == nullptr) {
		    if(!quiet) {
			Logger::err("GOM")
			    << classname << ":No such metaclass" << std::endl;
		    }
                } else {
		    result = Object::id_to_object(index);
                    if(result == nullptr) {
                        Logger::err("GOM")
                            << id << ": No such object in "
                            << classname << " instances" << std::endl;
                    }
		    if(result->meta_class() != mclass) {
                        Logger::err("GOM")
                            << id << ": is not of class "
                            << classname
			    << " but of class "
			    << result->meta_class()->name()
			    << std::endl;
		    }
                }
            }
        }

        if(result == nullptr && !quiet) {
            Logger::err("GOM") << "No such object:" << id << std::endl;
        }
	return result;
    }

    void Interpreter::list_names(std::vector<std::string>& names) const {
	names.clear();
    }

    Object* Interpreter::resolve_object(
	const std::string& id, bool quiet
    ) const {
	Any any_result = resolve(id, quiet);
	Object* result = nullptr;
	if(!any_result.get_value(result)) {
	    Logger::err("GOM") << id << " is not an Object" << std::endl;
	}
	return result;
    }

    Any Interpreter::resolve(const std::string& id, bool quiet) const {
	Any result;
	result.set_value(resolve_object_by_global_id(id,quiet));
	return result;
    }

    Any Interpreter::eval(
	const std::string& expression, bool quiet
    ) const {
	Any result;
	geo_argused(expression);
	if(!quiet) {
	    Logger::warn("Interpreter") << "eval(" << expression << ")"
					<< ": not implemented in baseclass"
					<< std::endl;
	}
	return result;
    }


    Object* Interpreter::eval_object(
	const std::string& expression, bool quiet
    ) const {
	Any any_result = eval(expression, quiet);
	Object* result = nullptr;
	bool OK = true;
	OK = any_result.meta_type() != nullptr &&
	     Any::is_pointer_type(any_result.meta_type()) &&
	     any_result.get_value(result);
	if(!OK && !quiet) {
	    Logger::err("GOM") << expression << " is not an Object"
			       << std::endl;
	}
	return result;
    }

    std::string Interpreter::eval_string(
	const std::string& expression, bool quiet
    ) const {
	Any any_result = eval(expression, quiet);
	std::string result = any_result.as_string();
	if(any_result.meta_type() == ogf_meta<std::string>::type()) {
	    result = "\'" + result + "\'";
	}
	return result;
    }


    MetaType* Interpreter::resolve_meta_type(
	const std::string& type_name
    ) const {
        return Meta::instance()->resolve_meta_type(type_name);
    }

    bool Interpreter::bind_meta_type(MetaType* mtype) {
        return Meta::instance()->bind_meta_type(mtype);
    }

    Object* Interpreter::create(const ArgList& args_in) {
	Object* result = nullptr;
	if(args_in.has_arg("classname")) {
	    // Classname and constructor arguments specified in
	    // a single ArgList.
	    std::string classname = args_in.get_arg("classname");
	    result = create(classname, args_in);
	    if(result == nullptr) {
		Logger::err("GOM")
		    << "create(): could not create object of class: "
		    << classname
		    << std::endl;
	    }
	} else if(
	    args_in.nb_args() == 1 &&
	    args_in.ith_arg_name(0) == "arg#0"
	) {
	    MetaType* mtype = args_in.ith_arg_type(0);

	    // A single argument, a string with the classname.
	    if(mtype == ogf_meta<std::string>::type()) {
		std::string classname = args_in.ith_arg_value(0).as_string();
		ArgList args;
		result = create(classname, args);
		if(result == nullptr) {
		    Logger::err("GOM")
			<< "create(): could not create object of class: "
			<< classname
			<< std::endl;
		}
	    }

	    // A single argument, a MetaClass
	    if(
		Any::is_pointer_type(mtype) &&
		Any::pointed_type(mtype)->is_subtype_of(
		    ogf_meta<Object>::type()
		)
	    ) {
		Object* object = nullptr;
		args_in.ith_arg_value(0).get_value(object);
		MetaClass* mclass = dynamic_cast<MetaClass*>(object);
		if(mclass != nullptr) {
		    std::string classname = mclass->name();
		    ArgList args;
		    result = create(classname, args);
		    if(result == nullptr) {
			Logger::err("GOM")
			    << "create(): could not create object of class: "
			    << classname
			    << std::endl;
		    }
		}
	    }

	} else {
	    Logger::err("GOM") << "create(): missing classname argument"
			       << std::endl;
	    result = nullptr;
	}
	return result;
    }

    Object* Interpreter::create(
	const std::string& classname, const ArgList& args
    ) {
        Object* result = nullptr;
        MetaClass* mclass = Meta::instance()->resolve_meta_class(classname);
        if(mclass != nullptr) {
            result = mclass->create(args);
        }
        return result;
    }

    Interpreter* Interpreter::instance_by_language(
	const std::string& language
    ) {
	auto it = instance_.find(language);
	if(it == instance_.end()) {
	    return nullptr;
	}
	return it->second;
    }

    Interpreter* Interpreter::instance_by_file_extension(
	const std::string& extension
    ) {
	auto it = instance_by_file_extension_.find(extension);
	if(it == instance_by_file_extension_.end()) {
	    return nullptr;
	}
	return it->second;
    }

    /********************************************************/

    Scope::Scope(Object* object) : object_(object) {
    }

    Scope::~Scope() {
    }

    void Scope::list_names(std::vector<std::string>& names) const {
	names.clear();
    }

    void Scope::search(const std::string& needle, const std::string& path) {
        std::vector<std::string> names;
        list_names(names);
        for(const std::string& name : names) {
            Any a = resolve(name);
            if(
                a.meta_type() != nullptr &&
                a.meta_type()->name() == "OGF::Scope*"
            ) {
                Scope* s = nullptr;
                a.get_value(s);
                if(s != nullptr) {
                    s->search(needle, path + "." + name);
                }
            }
            if(
                a.meta_type() != nullptr &&
                a.meta_type()->name() == "OGF::MetaType*"
            ) {
                MetaType* m = nullptr;
                a.get_value(m);
                if(m != nullptr) {
                    m->search(needle, path + "." + name);
                }
            }
        }
    }


    index_t Scope::get_nb_elements() const {
	std::vector<std::string> names;
	list_names(names);
	return index_t(names.size());
    }

    void Scope::get_element(index_t i, Any& value) const {
	std::vector<std::string> names;
	list_names(names);
	if(i >= names.size()) {
	    return;
	}
	value = const_cast<Scope*>(this)->resolve(names[i]);
    }

    /********************************************************/

    GlobalScope::GlobalScope(Interpreter* interpreter)
	: Scope(interpreter) {
    }

    GlobalScope::~GlobalScope() {
    }

    void GlobalScope::list_names(std::vector<std::string>& names) const {
	geo_assert(
	    dynamic_cast<Interpreter*>(object_) != nullptr
	);
	dynamic_cast<Interpreter*>(object_)->list_names(names);
    }

    Any GlobalScope::resolve(const std::string& name) {
	geo_assert(
	    dynamic_cast<Interpreter*>(object_) != nullptr
	);
	return dynamic_cast<Interpreter*>(
	    object_
	)->resolve(name);
    }

    /********************************************************/

    InterfaceScope::InterfaceScope(Object* object)
	: Scope(object) {
    }

    Any InterfaceScope::resolve(const std::string& name_in) {
	Any result;

	if(object_ == nullptr) {
	    return result;
	}

	std::string object_class_name = object_->meta_class()->name();
	std::string name = name_in;
	if(!String::string_starts_with(name, object_class_name)) {
	    name = object_class_name + name;
	}

	MetaClass* mclass = dynamic_cast<MetaClass*>(
	    Meta::instance()->resolve_meta_type(name)
	);

	if(mclass == nullptr && !String::string_ends_with(name, "Interface")) {
	    mclass = dynamic_cast<MetaClass*>(
		Meta::instance()->resolve_meta_type(name + "Interface")
	    );
	}

	if(mclass == nullptr && !String::string_ends_with(name, "Commands")) {
	    mclass = dynamic_cast<MetaClass*>(
		Meta::instance()->resolve_meta_type(name + "Commands")
	    );
	}

	if(mclass == nullptr) {
	    /*
	    Logger::warn("GOM") << object_class_name << ".I."
				<< name_in << ": no such Interface"
				<< std::endl;
	    */
	    return result;
	}

	ArgList args;
	Object* interface = mclass->factory()->create(args);
	if(mclass->find_property("grob") != nullptr) {
	    args.create_arg("grob",object_);
	    interface->set_properties(args);
	}
	result.set_value(interface);
	return result;
    }

    InterfaceScope::~InterfaceScope() {
    }

    inline void trim_prefix(std::string& s, const std::string prefix) {
	if(String::string_starts_with(s, prefix)) {
	    s = s.substr(prefix.length(), s.length()-prefix.length());
	}
    }

    inline void trim_suffix(std::string& s, const std::string suffix) {
	if(String::string_ends_with(s, suffix)) {
	    s = s.substr(0, s.length()-suffix.length());
	}
    }

    void InterfaceScope::list_names(std::vector<std::string>& names) const {
	names.clear();
	if(object_ == nullptr) {
	    return;
	}
	const std::string& mclassname = object_->meta_class()->name();
	std::string k =  mclassname + "_interfaces";
	if(!Environment::instance()->has_value(k)) {
	    return;
	}
	std::vector<std::string> interfaces;
	std::string interfaces_str = Environment::instance()->get_value(k);
	String::split_string(interfaces_str,';',interfaces);
	FOR(i,interfaces.size()) {
	    std::string name = interfaces[i];
	    trim_prefix(name, mclassname);
	    std::string trimmed_name = name;
	    trim_suffix(trimmed_name, "Interface");
	    trim_suffix(trimmed_name, "Commands");

	    index_t nb_classes = 0;
	    nb_classes += (
		Meta::instance()->resolve_meta_type(
		    mclassname+trimmed_name+"Interface"
		) != nullptr);
	    nb_classes += (
		Meta::instance()->resolve_meta_type(
		    mclassname+trimmed_name+"Commands"
		) != nullptr);
	    nb_classes += (
		Meta::instance()->resolve_meta_type(
		    mclassname+trimmed_name
		) != nullptr);

	    if(nb_classes > 1) {
		names.push_back(name);
	    } else {
		names.push_back(trimmed_name);
	    }
	}
    }

    /********************************************************/

    MetaTypesScope::MetaTypesScope(const std::string& prefix) :
        Scope(nullptr),
        prefix_(prefix) {
    }

    MetaTypesScope::~MetaTypesScope() {
    }

    Any MetaTypesScope::resolve(const std::string& name) {
        Any result;

        auto it = subscopes_.find(name);
        if(it != subscopes_.end()) {
            result.set_value((Scope*)(it->second));
            return result;
        }

        MetaType* mtype = Meta::instance()->resolve_meta_type(prefix_ + name);
        result.set_value(mtype);
        return result;
    }

    void MetaTypesScope::list_names(std::vector<std::string>& names) const {
        names.clear();
        std::vector<std::string> type_names;
        Meta::instance()->list_type_names(type_names);
        for(std::string cur: type_names) {
            // Skip pointer types
            if(*cur.rbegin() == '*') {
                continue;
            }
            // Skip names that have space (unsigned int, unsigned long)
            if(cur.find(' ') != std::string::npos) {
                continue;
            }
            if(prefix_ == "") {
                if(cur.find("::") == std::string::npos) {
                    names.push_back(cur);
                }
            } else {
                if(
                    String::string_starts_with(cur, prefix_) &&
                    cur.find("::",prefix_.length()) == std::string::npos
                ) {
                    names.push_back(cur.substr(prefix_.length()));
                }
            }
        }
        for(auto it: subscopes_) {
            names.push_back(it.first);
        }
    }

    MetaTypesScope* MetaTypesScope::create_subscope(const std::string& name) {
        std::string prefix = prefix_ + name + "::";
        MetaTypesScope* result = new MetaTypesScope(prefix);
        subscopes_[name] = result;
        return result;
    }


    /********************************************************/

    void Interpreter::automatic_completion(
	const std::string& line,
	index_t startw, index_t endw,
	const std::string& prefix,
	std::vector<std::string>& matches
    ) {
	geo_argused(endw);

	std::string context = "";

	context = line.substr(0,startw);

	if(context != "") {
	    if(context[context.length()-1] == '.') {
		context = context.substr(0, context.length()-1);
	    }
	    if(context[context.length()-1] == '(') {
		context = context.substr(0, context.length()-1);
	    }
	}

	if(context == "scene_graph.current_object=") {
	    std::string k = "grob_instances";
	    if(Environment::instance()->has_value(k)) {
		std::string instances =
		    Environment::instance()->get_value(k);
		String::split_string(instances,';',matches);
		FOR(i,matches.size()) {
		    matches[i] = "\'" + matches[i] + "\'";
		}
		filter_completion_candidates(prefix, matches);
	    }
	    return;
	}

	if(context == "gom.inspect_class") {
	    std::vector<MetaType*> all_types;
	    Meta::instance()->list_types(all_types);
	    for(MetaType* cur : all_types) {
		MetaClass* cur_class = dynamic_cast<MetaClass*>(cur);
		if(cur_class != nullptr) {
		    matches.push_back("\'" + cur_class->name() + "\'");
		}
		filter_completion_candidates(prefix, matches);
	    }
	    return;
	}

	int pos = int(context.length())-1;
	int depth=0;
	while(
	    pos >= 0 &&
	    strchr(",+-*/={} ",context[size_t(pos)]) == nullptr
	) {
	    if(depth==0 && context[size_t(pos)] == '(') {
		break;
	    }
	    if(context[size_t(pos)] == '(') {
		--depth;

	    } else if(context[size_t(pos)] == ')') {
		++depth;
	    }
	    pos = pos-1;
	}
	pos = pos+1;
	context = context.substr(
	    size_t(pos),
	    size_t(int(context.length())-pos)
	);

	get_keys(context,matches);
	filter_completion_candidates(prefix,matches);
    }

    void Interpreter::get_keys(
	const std::string& context, std::vector<std::string>& keys
    ) {
	keys.clear();

	// You do not want to create multiple object when pushing
	// <tab> !!
	{
	    size_t p = context.find("scene_graph.create_object(");
	    if(p != std::string::npos) {
		size_t q = context.find(")",p);
		if(q != std::string::npos) {
		    return;
		}
	    }
	}

	if(context == "") {
	    list_names(keys);
	} else {
	    Any context_any = eval(context, true);
	    get_keys(context_any, keys);
	}
    }

    void Interpreter::get_keys(
	Any& context, std::vector<std::string>& keys
    ) {
	keys.clear();
	MetaType* mtype = context.meta_type();
	if(mtype == nullptr) {
	    return;
	}

	if(!Any::is_pointer_type(mtype)) {
	    return;
	}

	mtype = Any::pointed_type(mtype);
	if(!mtype->is_subtype_of(ogf_meta<Object>::type())) {
	    return;
	}

	Object* object = nullptr;
	context.get_value(object);
	if(object == nullptr) {
	    return;
	}
	Request* request = dynamic_cast<Request*>(object);

	if(request != nullptr) {
	    MetaMethod* mm = request->method();
	    if(mm == nullptr) {
		return;
	    }

	    if(mm->nb_args() == 0) {
		keys.push_back(")");
		return;
	    } else if(mm->nb_args() == 1) {
		list_names(keys);
	    } else {
		std::string args;
		FOR(i, mm->nb_args()) {
		    if(args != "") {
			args += ",";
		    }
		    args += mm->ith_arg_name(i);
		    std::string arg_type_name = mm->ith_arg_type_name(i);
		    if(mm->ith_arg_has_default_value(i)) {
			if(
			    arg_type_name == "bool" ||
			    arg_type_name == "int" ||
			    arg_type_name == "unsigned int" ||
			    arg_type_name == "float" ||
			    arg_type_name == "double" ||
			    arg_type_name == "OGF::index_t" ||
			    arg_type_name == "GEO::index_t"
			) {
			    args += "=" +
				mm->ith_arg_default_value(i).as_string();
			} else {
			    args +=
				"=" + stringify(
				       mm->ith_arg_default_value(i).as_string()
				    );
			}
		    } else {
			if(arg_type_name == "bool") {
			    args += "=false";
			} else if(
			    arg_type_name == "int" ||
			    arg_type_name == "unsigned int" ||
			    arg_type_name == "OGF::index_t" ||
			    arg_type_name == "GEO::index_t"
			) {
			    args += "=0";
			} else if(
			    arg_type_name == "float" ||
			    arg_type_name == "double"
			) {
			    args += "=0.0";
			} else {
			    args += ("=" + stringify(""));
			}
		    }
		}
		keys.push_back(name_value_pair_call(args) + ")");
	    }
	    return;
	}

	if(dynamic_cast<Scope*>(object) != nullptr) {
	    dynamic_cast<Scope*>(object)->list_names(keys);
	    return;
	}

	MetaClass* mclass = object->meta_class();
	index_t n = index_t(mclass->nb_members());
	FOR(i,n) {
	    MetaMember* member = mclass->ith_member(i);
	    if(dynamic_cast<MetaConstructor*>(member) == nullptr) {
		if(dynamic_cast<MetaMethod*>(member) != nullptr) {
		    keys.push_back(member->name()+"(");
		} else {
		    keys.push_back(member->name());
		}
	    }
	}
    }


    void Interpreter::filter_completion_candidates(
	const std::string& prefix, std::vector<std::string>& completions
    ) {
	std::vector<std::string> result;
	for(size_t i=0; i<completions.size(); ++i) {
	    if(String::string_starts_with(completions[i],prefix)) {
		result.push_back(completions[i]);
	    }
	}
	std::swap(completions, result);
    }

    void Interpreter::filename_completion(
	const std::string& prefix_in,
	std::vector<std::string>& completions
    ) {
	std::string prefix;
	char quote = '\'';
	if(prefix_in.length() > 0) {
	    quote = prefix[0];
	    prefix = prefix_in.substr(1,prefix.length()-1);
	}

	std::string dirname = FileSystem::dir_name(prefix);

	std::vector<std::string> entries;
	FileSystem::get_directory_entries(dirname,entries);
	for(size_t i=0; i<entries.size(); ++i) {
	    if(
		String::string_starts_with(entries[i],prefix) &&
		FileSystem::is_file(entries[i])
	    ) {
		completions.push_back(
		    quote + dirname + "/" + entries[i] + quote
		);
	    }
	}
    }

    std::string Interpreter::stringify(const std::string& str) const {
	return '\'' + str + '\'';
    }

    std::string Interpreter::name_value_pair_call(
	const std::string& args
    ) const {
	return args;
    }

    void Interpreter::search(
	const std::string& needle_in, const std::string& path_in
    ) {
        std::string needle = needle_in;
        if(needle == "") {
            Logger::err("GOM") << "Search: empty string specified" << std::endl;
        }

        // "*" to display all meta-information available in the system
	// (takes a while !!)
        if(needle == "*") {
            needle = "";
        }

        std::string path = path_in;
        if(path != "") {
            path = path + ".";
        }
        meta_types_->search(needle,path+"gom.meta_types");
        globals_->search(needle,path+"globals");
    }

}
