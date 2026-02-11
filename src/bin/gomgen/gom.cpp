/*
 *  Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 Bruno Levy
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

#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/reflection/meta_builtin.h>
#include <OGF/gom/reflection/meta_enum.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/reflection/meta_property.h>
#include <OGF/gom/reflection/meta_slot.h>
#include <OGF/gom/reflection/meta_signal.h>
#include <OGF/gom/reflection/dynamic_struct.h>
#include <OGF/basic/os/text_utils.h>

#include <geogram/basic/file_system.h>

// Swig includes need to be AFTER OGF includes
// else it causes a problem under Windows.
#include "gom.h"
#include <swig/Modules/swigmod.h>
#include <swig/CParse/cparse.h>

#include <sstream>

namespace {

    /**
     * \brief Finds all possible scopes formed by a scoped class name.
     * \details This function is required because SWIG does not keep
     *  scopes in the base class names, thus we try to find each base
     *  class from the scope of the derived class.
     * \param[in] name the scoped class name
     * \param[out] scopes all the possible scopes
     */
    void find_scopes(std::string& name, std::vector<std::string>& scopes) {
	scopes.clear();
	for(
	    size_t pos=name.find("::");
	    pos != std::string::npos;
	    pos = name.find("::",pos+2)
	) {
	    scopes.push_back(name.substr(0, pos));
	}
    }

}

namespace {

    /**
     * \brief A global flag that indicates whether
     *  an error occured.
     */
    static bool error_flag = false;


    /**
     * \brief Tentatively converts a string to an integer.
     * \param[in] str the input string. If starting with '0x',
     *  then the number is in hexadecimal, else in decimal.
     * \param[out] value a reference to the parsed integer
     * \retval true if the string could be parsed, i.e. if it
     *  contained an integer, and only an integer
     * \retval false otherwise
     */
    bool string_to_int(const std::string& str, int& value) {
	errno = 0;
	int base = 10;
	const char* ptr = str.c_str();
	if(str.length() >= 2 && str.substr(0,2) == "0x") {
	    base = 16;
	    ptr += 2;
	}

	char* end;
	value = int(strtol(ptr, &end, base));
	return (end != str && *end == '\0' && errno == 0);
    }


    /**
     * \brief Converts a SWIG string into a std::string.
     * \param[in] s a pointer to the SWIG string to be converted
     * \return a std::string that copies the contents of \p s
     */
    inline std::string gom_string(String* s) {
	return std::string(Char(s));
    }

    /**
     * \brief Removes the double quotes around a string.
     * \details If the input string has no double quotes around it,
     *  it is left unmodified.
     * \param[in,out] s a reference to the string to be unquoted
     */
    void gom_unquote(std::string& s) {
	if(s.length() >= 2 && s[0] == '\"' && s[s.length() - 1] == '\"') {
	    s = s.substr(1,s.length() - 2);
	}
    }

    inline std::string gom_type_name_from_string(String* s) {
	std::string result = std::string(Char(s));
	return result;
    }

    inline std::string gom_value_from_string(String* s) {
	if(!Strncmp(s, "OGF::", 5)) {
	    return std::string(Char(s)+5);
	}
	return std::string(Char(s));
    }

    void gom_msg(const char* msg, Node* n);

    inline std::string gom_type_name(SwigType* ty) {
	String* s = SwigType_str(SwigType_base(ty),nullptr);
	std::string result = gom_type_name_from_string(s);
	if(SwigType_ispointer(ty)) {
	    result = result + "*";
	}
	return result;
    }

    inline std::string gom_enum_name(Node* n) {
	String* s = Getattr(n, "name");
	if(s == nullptr) {
	    return "$unnamed$";
	}
	std::string name = Char(s);
	return name;
    }

    inline OGF::MetaClass* gom_class_from_member(Node* n) {
	Node* clazz = Getattr(n,"parentNode");
	std::string class_name = gom_type_name_from_string(
	    Getattr(clazz,"name")
	);
	OGF::MetaClass* result = dynamic_cast<OGF::MetaClass*>(
	    OGF::Meta::instance()->resolve_meta_type(class_name)
	);
	ogf_assert(result != nullptr);
	return result;
    }

    inline OGF::MetaBuiltinStruct* gom_struct_from_member(Node* n) {
	Node* clazz = Getattr(n,"parentNode");
	std::string class_name = gom_type_name_from_string(
	    Getattr(clazz,"name")
	);
	OGF::MetaBuiltinStruct* result = dynamic_cast<OGF::MetaBuiltinStruct*>(
	    OGF::Meta::instance()->resolve_meta_type(class_name)
	);
	ogf_assert(result != nullptr);
	return result;
    }

    void copy_gom_attributes(Node* from, OGF::CustomAttributes* to) {
	Hash* attributes = Getattr(from,"gom:attributes");
	if(attributes != nullptr) {
	    for(int i=0; i<Len(attributes); i++) {
		String* name  = Getitem(Keys(attributes), i);
		String* value = Getattr(attributes, name);
		to->create_custom_attribute(gom_string(name), gom_string(value));
	    }
	}
    }

    void copy_gom_attributes(Node* from, OGF::MetaMethod* to) {
	Hash* attributes = Getattr(from,"gom:attributes");
	if(attributes != nullptr) {
	    for(int i=0; i<Len(attributes); i++) {
		String* name_in  = Getitem(Keys(attributes), i);
		String* value_in = Getattr(attributes, name_in);
		std::string name = gom_string(name_in);
		std::string value = gom_string(value_in);
		// Transfer 'gomarg' attributes to the argument of the function.
		if(OGF::String::string_starts_with(name, "gomarg$")) {
		    std::vector<std::string> parts;
		    OGF::String::split_string(name,'$',parts);
		    if(parts.size() == 3) {
			const std::string& arg_name = parts[1];
			const std::string& attribute_name = parts[2];
			OGF::MetaArg* marg = to->find_arg(arg_name);
			if(marg != nullptr) {
			    marg->create_custom_attribute(attribute_name, value);
			} else {
			    error_flag = true;
			    OGF::Logger::err("GomGen")
				<< "Error in Doxygen comment:"
				<< std::endl;
			    OGF::Logger::err("GomGen")
				<< "gom_arg specified for undefined arg: "
				<< arg_name << std::endl;
			    OGF::Logger::err("GomGen")
				<< "in method: "
				<< to->container_meta_class()->name()
				<< "::" << to->name()
				<< " with args: "
				<< std::endl;
			    for(OGF::index_t j=0; j<to->nb_args(); ++j) {
				OGF::Logger::err("GomGen")
				    << " "
				    << to->ith_arg(j)->name()
				    << std::endl;
			    }
			}
		    } else {
			error_flag = true;
			OGF::Logger::err("GomGen")
			    << "malformed gom_arg attribute: "
			    << name << ":" << value << std::endl;
		    }
		} else {
		    to->create_custom_attribute(name, value);
		}
	    }
	}
    }

    void copy_gom_args(Node* from, OGF::MetaMethod* to) {
	Parm* parms = Getattr(from,"parms");
	if(parms != nullptr) {
	    int param_id = 1;
	    for(Node* p = parms; p != nullptr; p = nextSibling(p)) {
		SwigType* type_in  = Getattr(p,"type");
		String*   name_in  = Getattr(p,"name");
		String*   value_in = Getattr(p,"value");

		std::string name;
		if(name_in == nullptr) {
		    OGF::Logger::warn("GomGen")
			<< "anonymous arg in signal/slot"
			<< std::endl;
		    std::ostringstream s;
		    s << "prm_" << param_id;
		    name = s.str();
		} else {
		    name = gom_string(name_in);
		}

		std::string type = gom_type_name(type_in);
		OGF::MetaArg arg(name, type);
		copy_gom_attributes(p, &arg);
		if(value_in != nullptr) {
		    std::string default_value = gom_value_from_string(value_in);
		    gom_unquote(default_value);
		    //gom_unqualify(default_value);
		    arg.default_value().set_value(default_value);
		}
		to->add_arg(arg);
		param_id++;
	    }
	}
    }

//-------------------------------- Debugging ------------------------------

    void print_attr(Node* n, const char* attrib) {
	String* value = Getattr(n, attrib);
	if(value != nullptr) {
	    Printv(stdout, "[GOM]      ", attrib, "=", value, "\n", nullptr);
	}
    }

    void gom_msg(const char* msg, Node* n) {
	Printv(stdout, "[GOM] >>> ", msg, "\n", nullptr);

	List* k = Keys(n);
	Printv(stdout, "[GOM]      keys = ", nullptr);
	for(int i=0; i<Len(k); i++) {
	    Printv(stdout, Getitem(k, i), " ", nullptr);
	}
	Printv(stdout, "\n", nullptr);

	print_attr(n, "nodeType");
	print_attr(n, "name");
	print_attr(n, "type");
	print_attr(n, "decl");
	print_attr(n, "lang");
	print_attr(n, "kind");
	print_attr(n, "value");
	print_attr(n, "enumvalue");

	Parm* parms = Getattr(n,"parms");
	if(parms != nullptr) {
	    Printv(stdout, "[GOM]      parms = (  ", nullptr);
	    for (Node* p = parms; p != nullptr; p = nextSibling(p)) {
		SwigType *type  = Getattr(p,"type");
		String   *name  = Getattr(p,"name");
		String   *value = Getattr(p,"value");
		if(value != nullptr) {
		    Printv(
			stdout,
			"( ",
			SwigType_str(type,nullptr), " ", name, "=", value, " ) ",
			nullptr
			);
		} else {
		    Printv(
			stdout,
			"( ",
			SwigType_str(type,nullptr), " ", name, " ) ", nullptr
			);
		}
	    }
	    Printv(stdout, " )\n", nullptr);
	}


	List* baselist = Getattr(n,"baselist");
	if(baselist != nullptr) {
	    Printv(stdout, "[GOM]      baselist = (  ", nullptr);
	    for(int i=0; i<Len(baselist); i++) {
		Printv(stdout, Getitem(baselist, i), " ", nullptr);
	    }
	    Printv(stdout, " )\n", nullptr);
	}

	Hash* attributes = Getattr(n,"gom:attributes");
	if(attributes != nullptr) {
	    Printv(stdout, "[GOM]      attributes = (  ", nullptr);
	    for(int i=0; i<Len(attributes); i++) {
		String* name  = Getitem(Keys(attributes), i);
		String* value = Getattr(attributes, name);
		Printv(stdout, "( ", name, "=", value, ") ", nullptr);
	    }
	    Printv(stdout, " )\n", nullptr);
	}
    }

//--------------------------------------------------------------------------

    class GOMSWIG : public Language {
    public:

	enum GomMemberType { HIDDEN, SLOT, SIGNAL, PROPERTY };

	GOMSWIG() {
	    gomclass_flag   = false;
	    gom_member_type = HIDDEN;
	    gom_directive_flag = false;
	    user_attributes   = nullptr;
	    system_attributes = nullptr;
	}

	const std::vector<OGF::MetaClass*>& get_generated_classes() const {
	    return generated_classes_;
	}

	void set_package(
	    const std::string& package_name, const std::string& package_directory
	) {
	    package_name_ = package_name;
	    package_directory_ = package_directory;
	}

	virtual int top(Node* n) {
	    return Language::top(n);
	}

	virtual void main(int argc, char *argv[]) {
	    Language::main(argc, argv);
	}

	/* SWIG directives */

	virtual int pragmaDirective(Node *n) {

	    String* lang = Getattr(n, "lang");

	    if(!Strcmp(lang, "gom")) {
		gom_directive_flag = true;
		String* name = Getattr(n, "name");
		if(!Strcmp(name, "gomclass")) {
		    gomclass_flag = true;
		} else if(!Strcmp(name, "gomslots")) {
		    gom_member_type = SLOT;
		} else if(!Strcmp(name, "gomsignals")) {
		    gom_member_type = SIGNAL;
		} else if(!Strcmp(name, "gomproperties")) {
		    gom_member_type = PROPERTY;
		}
		return SWIG_OK;
	    } else if(!Strcmp(lang,"gomattribute")) {
		String* name  = Getattr(n, "name");
		String* value = Getattr(n, "value");
		save_attribute(name, value);
		return SWIG_OK;
	    }
	    return Language::pragmaDirective(n);
	}

	/* C/C++ parsing */

	virtual int cDeclaration(Node *n) {
	    gom_directive_flag = false;
	    Node* clazz = Getattr(n,"parentNode");
	    if(clazz != nullptr && Getattr(clazz,"gom:kind") != nullptr) {
		switch(gom_member_type) {
		    case SIGNAL:
			Setattr(n, "gom:kind", "signal");
			break;
		    case SLOT:
			Setattr(n, "gom:kind", "slot");
			break;
		    case PROPERTY:
			Setattr(n, "gom:kind", "property");
			break;
		    case HIDDEN:
			break;
		}
		copy_attributes(n);
	    }
	    cleanup_attributes();
	    return Language::cDeclaration(n);
	}

	virtual int externDeclaration(Node *n) {
	    cleanup_attributes();
	    return Language::externDeclaration(n);
	}


	//   Note: there is no enumHandler()/enumvalueHandler function,
	// for enum, I've put the code in the "xxxDeclaration" functions.
	virtual int enumDeclaration(Node *n) {
	    std::string name = gom_enum_name(n);
	    if(name != "$unnamed$") {
		OGF::Meta::instance()->bind_meta_type(new OGF::MetaEnum(name));
	    }
	    cleanup_attributes();
	    return Language::enumDeclaration(n);
	}

	virtual int enumvalueDeclaration(Node *n) {

	    std::string name = Char(Getattr(n,"name"));
	    Node* enumm = Getattr(n,"parentNode");
	    std::string enum_name = gom_enum_name(enumm);
	    if(enum_name == "$unnamed$") {
		return Language::enumvalueDeclaration(n);
	    }
	    OGF::MetaEnum* menum = dynamic_cast<OGF::MetaEnum*>(
		OGF::Meta::instance()->resolve_meta_type(enum_name)
		);
	    ogf_assert(menum != nullptr);
	    int value = 0;
	    String* value_string = Getattr(n,"enumvalue");

	    if(Char(value_string) == nullptr) {
		value_string = Getattr(n,"enumvalueex");
	    }

	    sscanf(Char(value_string), "%d", &value);
	    String* check = NewStringf("%d", value);

	    // Checks whether the value was an expression
	    // TODO: there is probably a Swig function to get the value.
	    if(Strcmp(value_string, check)) {

		bool ok = false;

		// Default value is previous value + 1
		std::string sym = Char(value_string);

		if(string_to_int(sym, value)) {
		    ok = true;
		} else {
		    if(sym.substr(sym.length() - 4,4) == " + 1") {
			sym = sym.substr(0, sym.length() - 4);
			if(menum->has_value(sym)) {
			    value = menum->get_value_by_name(sym) + 1;
			    ok = true;
			}
		    }
		}

		// If this was a more complex expression, issue a warning
		if(!ok) {
		    value = int(menum->nb_values());
		    OGF::Logger::warn("GomGen")
			<< enum_name << "::" << name << "=" << Char(value_string)
			<< " : unspecified enum value, or complex expression"
			<< std::endl;
		    OGF::Logger::warn("GomGen")
			<< "    -> using " << value
			<< " please check if this is correct" << std::endl;
		}
	    }
	    Delete(check);
	    menum->add_value(name,value);
	    cleanup_attributes();
	    return Language::enumvalueDeclaration(n);
	}

	virtual int enumforwardDeclaration(Node *n) {
	    cleanup_attributes();
	    return Language::enumforwardDeclaration(n);
	}

	virtual int classDeclaration(Node *n) {

	    gom_member_type = HIDDEN;

	    // Check whether file belongs to current package
	    //   If yes, set package attribute to current package,
	    //   else leave package undefined (and it is OK because
	    //   we just need it to decide which gom_classes should
	    //   be passed to the code generator).
	    String* file = Getfile(n);
	    std::string package = "???";

	    std::string filename(Char(file));
	    OGF::FileSystem::flip_slashes(filename);

	    if(OGF::String::string_starts_with(filename, package_directory_)) {
		package = package_name_;
	    }

	    if(gomclass_flag) {
		Setattr(n,"gom:kind","class");
		Setattr(n,"gom:package",package.c_str());
		gomclass_flag = false;
		copy_attributes(n);
		if(Getattr(n,"abstract") != nullptr) {
		    Setattr(n, "gom:abstract", "true");
		} else {
		    //   Force notabstract so that GOM will
		    // try to generate a constructor, this
		    // will allow letting the compiler report
		    // pure virtual functions that are not
		    // implemented (else it is difficult
		    // to figure out what's going on when
		    // when a pure virtual function has been
		    // forgotten).
		    SetFlag(n,"feature:notabstract");
		}
	    }
	    cleanup_attributes();
	    return Language::classDeclaration(n);
	}

	virtual int classforwardDeclaration(Node *n) {
	    cleanup_attributes();
	    return Language::classforwardDeclaration(n);
	}

	virtual int constructorDeclaration(Node *n) {
	    Node* clazz = Getattr(n,"parentNode");
	    if(
		Getattr(clazz,"gom:kind")     != nullptr &&
		Getattr(clazz,"gom:abstract") == nullptr
		) {
		Setattr(n,"gom:kind","constructor");
		copy_attributes(n);
	    }
	    cleanup_attributes();
	    return Language::constructorDeclaration(n);
	}

	virtual int destructorDeclaration(Node *n) {
	    cleanup_attributes();
	    return Language::destructorDeclaration(n);
	}

	virtual int accessDeclaration(Node *n) {
	    if(!gom_directive_flag) {
		gom_member_type = HIDDEN;
	    }
	    gom_directive_flag = false;
	    cleanup_attributes();
	    return Language::accessDeclaration(n);
	}

	virtual int usingDeclaration(Node *n) {
	    cleanup_attributes();
	    return Language::usingDeclaration(n);
	}

	virtual int namespaceDeclaration(Node *n) {
	    cleanup_attributes();
	    return Language::namespaceDeclaration(n);
	}

	virtual int templateDeclaration(Node *n) {
	    cleanup_attributes();
	    return Language::templateDeclaration(n);
	}

	virtual int lambdaDeclaration(Node *n) {
	    cleanup_attributes();
	    return Language::lambdaDeclaration(n);
	}


	/* Handlers */

	virtual int functionHandler(Node *n) {
	    return Language::functionHandler(n);
	}

	virtual int globalfunctionHandler(Node *n) {
	    ogf_argused(n);
	    return SWIG_OK;
	}

	virtual int memberfunctionHandler(Node *n) {
	    std::string name = gom_string(Getattr(n,"name"));
	    String* kind = Getattr(n,"gom:kind");
	    if(kind != nullptr) {
		if(
		    !Strcmp(kind, "signal") ||
		    !Strcmp(kind, "slot")
		) {
		    signalslotHandler(n);
		} else if(!Strcmp(kind, "property")) {
		    propertyHandler(n);
		}
	    }
	    return SWIG_OK;
	}

	virtual int signalslotHandler(Node* n) {
	    String* kind = Getattr(n,"gom:kind");
	    std::string name = gom_string(Getattr(n,"name"));

	    OGF::MetaClass* mclass = gom_class_from_member(n);
	    OGF::MetaMethod* mmethod = nullptr;
	    if(!Strcmp(kind, "signal")) {
		mmethod = new OGF::MetaSignal(name, mclass);
	    } else if(!Strcmp(kind, "slot")) {
		SwigType* type_in = Getattr(n,"type");
		std::string type = gom_type_name(type_in);
		mmethod = new OGF::MetaSlot(name, mclass, type);
	    } else {
		ogf_assert_not_reached;
	    }

	    copy_gom_args(n, mmethod);
	    copy_gom_attributes(n, mmethod);
	    return SWIG_OK;
	}

	virtual int propertyHandler(Node* n) {

	    OGF::MetaClass* mclass = gom_class_from_member(n);
	    std::string name = gom_string(Getattr(n,"name"));

	    if(name.length() < 4) {
		error_flag = true;
		OGF::Logger::err("GomGen")
		    << "malformed property function: "
		    << mclass->name() << "::" << name << std::endl;
		gom_msg("offending node:",n);
		return SWIG_OK;
	    }

	    // "get_" or "set_"
	    std::string getset = name.substr(0,4);

	    // the rest of the method name after "get_"/"set_"
	    name = name.substr(4, name.length() - 4);

	    Parm* parms = Getattr(n,"parms");

	    if(getset == "get_") {
		if(parms != nullptr) {
		    error_flag = true;
		    OGF::Logger::err("GomGen")
			<< mclass->name() << "::" << getset << name
			<< " : malformed property getter, "
			<< "should not take any argument"
			<< std::endl;
		    return SWIG_OK;
		}
		if(!SwigType_isconst(Getattr(n, "decl"))) {
		    error_flag = true;
		    OGF::Logger::err("GomGen")
			<< mclass->name() << "::" << getset << name
			<< " : malformed property getter, should be const"
			<< std::endl;
		    return SWIG_OK;
		}
		OGF::MetaProperty* mprop = mclass->find_property(name);
		if(mprop == nullptr) {
		    std::string type = gom_type_name(Getattr(n,"type"));
		    mprop = new OGF::MetaProperty(
			name, mclass, type, true
			);
		    copy_gom_attributes(n, mprop);
		}
		// copy_gom_attributes(n, mprop->meta_method_get());
                // TODO: here?
	    } else if(getset == "set_") {
		if(parms == nullptr || nextSibling(parms) != nullptr) {
		    error_flag = true;
		    OGF::Logger::err("GomGen")
			<< mclass->name()
			<< "::" << getset << name
			<< " : malformed property setter,"
			<< " should take exactly one argument"
			<< std::endl;
		    return SWIG_OK;
		}
		OGF::MetaProperty* mprop = mclass->find_property(name);
		if(mprop == nullptr) {
		    std::string type = gom_type_name(Getattr(parms,"type"));
		    mprop = new OGF::MetaProperty(
			name, mclass, type, false
			);
		    copy_gom_attributes(n, mprop);
		} else {
		    mprop->set_read_only(false);
		}
		// copy_gom_attributes(n, mprop->meta_method_set()); TODO: here?
	    } else {
		error_flag = true;
		OGF::Logger::err("GomGen") << "malformed property function: "
					   << mclass->name() << "::"
					   << name << std::endl;
		gom_msg("offending node:",n);
		return SWIG_OK;
	    }

	    return SWIG_OK;
	}

	virtual int staticmemberfunctionHandler(Node *n) {
	    ogf_argused(n);
	    return SWIG_OK;
	}

	virtual int callbackfunctionHandler(Node *n) {
	    ogf_argused(n);
	    return SWIG_OK;
	}

	/* Variable handlers */

	virtual int variableHandler(Node *n) {
	    // Dispatches to xxxvariableHandler() functions.
	    return Language::variableHandler(n);
	}

	virtual int globalvariableHandler(Node *n) {
	    ogf_argused(n);
	    return SWIG_OK;
	}

	virtual int membervariableHandler(Node *n) {
	    OGF::MetaBuiltinStruct* mstruct = gom_struct_from_member(n);
	    if(mstruct != nullptr) {
		std::string name = gom_string(Getattr(n,"name"));
		SwigType* type = Getattr(n,"type");
		std::string type_name = gom_type_name(type);
		mstruct->add_field(name, type_name, size_t(-1));
	    }
	    return SWIG_OK;
	}

	virtual int staticmembervariableHandler(Node *n) {
	    ogf_argused(n);
	    return SWIG_OK;
	}

	/* C++ handlers */

	virtual int memberconstantHandler(Node *n) {
	    ogf_argused(n);
	    return SWIG_OK;
	}

	virtual int constructorHandler(Node *n) {
	    if(checkAttribute(n, "gom:kind", "constructor")) {
		OGF::MetaClass* mclass = gom_class_from_member(n);
		OGF::MetaConstructor* mconstructor =
		    new OGF::MetaConstructor(mclass);
		copy_gom_args(n, mconstructor);
		copy_gom_attributes(n, mconstructor);
	    }
	    return SWIG_OK;
	}


	virtual int copyconstructorHandler(Node *n) {
	    return constructorHandler(n);
	}

	virtual int destructorHandler(Node *n) {
	    ogf_argused(n);
	    return SWIG_OK;
	}


	virtual int classHandler(Node *n) {

	    // HERE
	    if(!checkAttribute(n, "gom:kind", "class")) {
		std::string name = gom_type_name_from_string(Getattr(n, "name"));
		OGF::MetaBuiltinStruct* mstruct =
		    new OGF::MetaBuiltinStruct(name);
		OGF::Meta::instance()->bind_meta_type(mstruct);

	    }

	    if(checkAttribute(n, "gom:kind", "class")) {

		bool abstract = (checkAttribute(n, "gom:abstract", "true")!=0);
		std::string name = gom_type_name_from_string(Getattr(n, "name"));

		int nb_superclasses = 0;
		OGF::MetaClass* superclass = nullptr;

		if(Getattr(n, "gom:superclass") != nullptr) {
		    std::string superclass_name =
			Char(Getattr(n, "gom:superclass"));
		    superclass = dynamic_cast<OGF::MetaClass*>(
			OGF::Meta::instance()->resolve_meta_type(superclass_name)
		    );
		    if(superclass == nullptr) {
			error_flag = true;
			OGF::Logger::err("GomGen")
			    << "gom_class " << name
			    << " has invalid GOM superclass: " << superclass_name
			    << std::endl;
		    } else {
			nb_superclasses = 1;
		    }
		} else {
		    List* baselist = Getattr(n,"baselist");
		    if(baselist != nullptr) {
			for(int i=0; i<Len(baselist); i++) {
			    std::string cur_base_name =
				gom_type_name_from_string(Getitem(baselist, i));

			    OGF::MetaClass* cur_base =
				dynamic_cast<OGF::MetaClass*>(
				    OGF::Meta::instance()->
				    resolve_meta_type(cur_base_name)
				);

			    // Swig does not include scope in base class names.
			    //   If cur_base_name is not found, then try getting
			    // the scope from current class name and
			    // prepending it to cur_base_name.
			    if(cur_base == nullptr) {
				std::vector<std::string> scopes;
				find_scopes(name, scopes);
				for(
				    size_t scope=0; scope<scopes.size(); ++scope
				) {
				    std::string scoped_cur_base_name =
					scopes[scope] + "::" + cur_base_name;
				    cur_base = dynamic_cast<OGF::MetaClass*>(
					OGF::Meta::instance()->
					resolve_meta_type(scoped_cur_base_name)
				    );
				    if(cur_base != nullptr) {
					break;
				    }
				}
			    }
			    if(cur_base != nullptr) {
				nb_superclasses++;
				superclass = cur_base;
			    }
			}
		    }
		}

		if(nb_superclasses > 1) {
		    error_flag = true;
		    OGF::Logger::err("GomGen")
			<< "Class \'" << name
			<< "\' has more than one GOM superclass" << std::endl;
		}

		OGF::Meta::instance()->bind_meta_type(
		    new OGF::MetaBuiltinType(name + "*")
		);

		OGF::MetaClass* mclass =
		    new OGF::MetaClass(name, superclass, abstract);
		OGF::Meta::instance()->bind_meta_type(mclass);
		copy_gom_attributes(n, mclass);
		mclass->create_custom_attribute(
		    "package",Char(Getattr(n, "gom:package"))
		);

		{
		    String* file = Getfile(n);
		    char* p = strstr(Char(file), "/OGF");
		    if(p == nullptr) {
			p = strstr(Char(file), "\\OGF");
		    }
		    if(p != nullptr) {
			for(char* pp=p; *pp; pp++) {
			    if(*pp == '\\') *pp = '/';
			}
			mclass->create_custom_attribute("file",p+1);
		    }
		}
		generated_classes_.push_back(mclass);
	    }

	    return Language::classHandler(n);
	}

	virtual int typedefHandler(Node *n) {
	    ogf_argused(n);
	    return SWIG_OK;
	}

	/* wrappers */

	virtual int constantWrapper(Node *n) {
	    ogf_argused(n);
	    return SWIG_OK;
	}

	bool is_system_attribute(String* name) {
	    return (
		!Strcmp(name, "abstract") ||
		!Strcmp(name, "superclass")
		);
	}

	void save_attribute(String* name, String* value) {
	    if(is_system_attribute(name)) {
		if(system_attributes == nullptr) {
		    system_attributes = NewHash();
		}
		Setattr(system_attributes, name, value);
	    } else {
		if(user_attributes == nullptr) {
		    user_attributes = NewHash();
		}
		Setattr(user_attributes, name, value);
	    }
	}

	void copy_attributes(Node* n) {
	    if(system_attributes != nullptr) {
		if(checkAttribute(system_attributes,"abstract","true")) {
		    Setattr(n, "gom:abstract", "true");
		}
		String* superclass = Getattr(system_attributes, "superclass");
		if(superclass != nullptr) {
		    Setattr(n, "gom:superclass", superclass);
		}
		Setattr(n, "gom:system_attributes", system_attributes);
		system_attributes = nullptr;
	    }
	    if(user_attributes != nullptr) {
		Setattr(n, "gom:attributes", user_attributes);
		user_attributes = nullptr;
	    }
	}

	void cleanup_attributes() {
	    Delete(system_attributes);
	    system_attributes = nullptr;
	    Delete(user_attributes);
	    user_attributes = nullptr;
	}

    private:
	GomMemberType gom_member_type;
	bool gomclass_flag;
	bool gom_directive_flag;
	Hash* user_attributes;
	Hash* system_attributes;
	std::vector<OGF::MetaClass*> generated_classes_;
	std::string package_name_;
	std::string package_directory_;
    };

}

/***********************************************************************/

Language* get_swig_gom_language() {
    static Language* instance = nullptr;
    if(instance == nullptr) {
        instance = new GOMSWIG();
    }
    return instance;
}

const std::vector<OGF::MetaClass*>& get_swig_gom_generated_classes() {
    return dynamic_cast<GOMSWIG*>(
        get_swig_gom_language()
    )->get_generated_classes();
}

void set_swig_gom_package(
    const std::string& package_name, const std::string& package_dir
) {
    dynamic_cast<GOMSWIG*>(
        get_swig_gom_language()
    )->set_package(package_name, package_dir);
}

bool swig_gom_error_occured() {
    return error_flag;
}
