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

#include <OGF/gom/codegen/codegen.h>

#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/reflection/meta_builtin.h>
#include <OGF/gom/reflection/meta_enum.h>
#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/reflection/meta_property.h>
#include <OGF/gom/reflection/meta_slot.h>
#include <OGF/gom/reflection/meta_signal.h>
#include <OGF/gom/reflection/meta_struct.h>

namespace OGF {

    GomCodeGenerator::GomCodeGenerator() : out_(nullptr) {
        pass_by_value_.insert("int");
        pass_by_value_.insert("unsigned int");
        pass_by_value_.insert("long");
        pass_by_value_.insert("unsigned long");
        pass_by_value_.insert("float");
        pass_by_value_.insert("double");
        pass_by_value_.insert("bool");
        pass_by_value_.insert("GEO::index_t");
    }

    void GomCodeGenerator::generate(
        std::ostream& out_in,
        std::vector<MetaClass*> classes,
        const std::string& package_name
    ) {
        to_generate_.clear();
        sorted_.clear();
        out_ = &out_in;
        package_name_ = package_name;
        for(unsigned int i=0; i<classes.size(); i++) {
            to_generate_.insert(classes[i]);
        }
        while(to_generate_.size() != 0) {
            MetaClass* cur = *(to_generate_.begin());
            generate(cur);
        }

// MSVC does not see gom_package_initialize if its in namespace OGF
// and g++ does not see it if it's *not* in namespace OGF...
#ifndef GEO_OS_WINDOWS
        out() << "namespace OGF {" << std::endl;
#endif
        out() << "   void gom_package_initialize_"
              << package_name << "() {" << std::endl;
        for(unsigned int i=0; i<sorted_.size(); i++) {
            out() << "      OGF::"
                  << "gom_class_initialize_"
                  << colons_to_underscores(sorted_[i]->name())
                  << "();" << std::endl;
        }
        out() << "   }" << std::endl;

#ifndef GEO_OS_WINDOWS
        out() << "}" << std::endl;
#endif
    }

    void GomCodeGenerator::generate(MetaClass* type) {

        MetaClass* base_class = type->super_class();
        if(to_generate_.find(base_class) != to_generate_.end()) {
            generate(base_class);
        }

        std::string class_package = "???";
        if(type->has_custom_attribute("package")) {
            class_package = type->custom_attribute_value("package");
        }
        if(class_package != package_name_) {
            if(to_generate_.find(type) != to_generate_.end()) {
                to_generate_.erase(to_generate_.find(type));
            }
            return;
        }

        Logger::out("Gom::CodeGen") << type->name() << std::endl;

        out() << "namespace OGF {" << std::endl;
        out() << std::endl;
        out() << std::endl;
        generate_class(type);
        out() << std::endl;
        out() << std::endl;
        out() << "}" << std::endl;

        if(to_generate_.find(type) != to_generate_.end()) {
            to_generate_.erase(to_generate_.find(type));
            sorted_.push_back(type);
        }
    }

    void GomCodeGenerator::generate_builtin(MetaBuiltinType* mbuiltin) {

	// using dynamic_cast<> instead of GOM meta information because
	// GOM meta information is not available yet ! (we are generating
	// it...)
	MetaBuiltinStruct* mbstruct = dynamic_cast<MetaBuiltinStruct*>(mbuiltin);
	bool is_meta_struct = (mbstruct != nullptr);

        out() << "   if(!Meta::instance()->meta_type_is_bound("
              << stringify(mbuiltin->name()) << ")) {" << std::endl;
        if(mbuiltin->is_pointer_type()) {
            out() << "      ogf_declare_pointer_type<";
        } else if(is_meta_struct) {
            out() << "      ogf_declare_struct<";
	} else {
            out() << "      ogf_declare_builtin_type<";
        }
        out() << mbuiltin->name() << ">("
              << stringify(mbuiltin->name())
              << ")";

	if(is_meta_struct) {
	    MetaStruct* mstruct = mbstruct->get_meta_struct();
	    index_t nb_fields = index_t(mstruct->nb_properties(false));
	    for(index_t i=0; i<nb_fields; ++i) {
		MetaProperty* mprop = mstruct->ith_property(i,false);
		out() << std::endl
		      << "         ->ogf_add_field("
		      << mbstruct->name() << "," << mprop->name()
		      << ")";
	    }
	}

	out() << ";" << std::endl;

        out() << "   }" << std::endl;
        out() << std::endl;
    }

    void GomCodeGenerator::generate_enum(MetaEnum* menum) {
        out() << "   if(!Meta::instance()->meta_type_is_bound("
              << stringify(menum->name()) << ")) {" << std::endl;
        out() << "      MetaEnum* menum = ogf_declare_enum<"
              << menum->name()
              << ">(" << stringify(menum->name()) << ");"
              << std::endl;
        for(index_t i=0; i<menum->nb_values(); i++) {
            out() << "      menum->add_value("
                  << stringify(menum->ith_name(i))
                  << "," << menum->ith_value(i)
                  << ");" << std::endl;
        }
        generate_attributes(menum,"menum");
        out() << "   } "<< std::endl;
        out() << std::endl;
    }

    void GomCodeGenerator::generate_class(MetaClass* mclass) {

        std::set<std::string> used_types;
        mclass->get_used_types(used_types, false);
        for(auto& it : used_types) {
            MetaType* cur_type = Meta::instance()->resolve_meta_type(it);
            if(cur_type == nullptr) {
                MetaBuiltinType* cur_builtin = new MetaBuiltinType(it);
                Meta::instance()->bind_meta_type(cur_builtin);
            }
        }


        out() << std::endl;
        out() << "/************* " << mclass->name() << " **************/"
              << std::endl;
        out() << std::endl;


        std::vector<MetaSlot*> meta_slots;
        mclass->get_slots(meta_slots, false);
        for(unsigned int i=0; i<meta_slots.size(); i++) {
            MetaSlot* mslot = meta_slots[i];
            if(
                mslot->nb_args() == 1 &&
                mslot->ith_arg(0)->type_name() == "OGF::ArgList"
            ) {
                generate_method_adapter_arglist(mslot);
            } else {
                generate_method_adapter(mslot);
            }
        }


        std::vector<MetaProperty*> meta_properties;
        mclass->get_properties(meta_properties, false);
        for(unsigned int i=0; i<meta_properties.size(); i++) {
            generate_method_adapter(meta_properties[i]->meta_method_get());
            if(!meta_properties[i]->read_only()) {
                generate_method_adapter(
                    meta_properties[i]->meta_method_set()
                );
            }
        }


        std::vector<MetaConstructor*> meta_constructors;
        mclass->get_constructors(meta_constructors);
        for(unsigned int i=0; i<meta_constructors.size(); i++) {
            generate_factory(meta_constructors[i]);
        }

        std::vector<MetaSignal*> meta_signals;
        mclass->get_signals(meta_signals, false);
        for(unsigned int i=0; i<meta_signals.size(); i++) {
            generate_signal_adapter(meta_signals[i]);
        }


        out() << "void " << "gom_class_initialize_"
              << colons_to_underscores(mclass->name()) << "() {"
              << std::endl;

        out() << "   " << "ogf_declare_pointer_type<" << mclass->name()
              << "*>(" << stringify(mclass->name() + "*" ) << ");"
              << std::endl;

	// Little local function (used twice)
	auto generate_enum_or_builtin = [this](MetaType* mtype) {
            MetaEnum* menum = dynamic_cast<MetaEnum*>(mtype);
            if(menum != nullptr) {
                generate_enum(menum);
		return;
            }
            MetaBuiltinType* mbuiltin = dynamic_cast<MetaBuiltinType*>(mtype);
            if(mbuiltin != nullptr) {
                generate_builtin(mbuiltin);
		return;
            }
	};

	// Get the types used by the struct types
	std::set<std::string> struct_used_types;
        for(auto& it : used_types) {
            MetaType* mtype = Meta::instance()->resolve_meta_type(it);
	    geo_assert(mtype != nullptr);
	    MetaBuiltinStruct* mbstruct=dynamic_cast<MetaBuiltinStruct*>(mtype);
	    if(mbstruct != nullptr) {
		MetaStruct* mstruct = mbstruct->get_meta_struct();
		geo_assert(mstruct != nullptr);
		index_t nb_fields = index_t(mstruct->nb_properties(false));
		for(index_t i=0; i<nb_fields; ++i) {
		    MetaProperty* mprop = mstruct->ith_property(i,false);
		    // Note: use type_name(), not type()->name(), because
		    // type() is not initialized yet here ! (using a
		    // dummy meta info populated by the parser)
		    struct_used_types.insert(mprop->type_name());
		}
	    }
	}

	// Generate first the meta info for the types used as struct fields
        for(auto& it : struct_used_types) {
            MetaType* mtype = Meta::instance()->resolve_meta_type(it);
	    generate_enum_or_builtin(mtype);
        }

	// Now we can generate the rest of the meta info
        for(auto& it : used_types) {
	    // Already generated if in struct_used_types
	    if(struct_used_types.find(it) != struct_used_types.end()) {
		continue;
	    }
            MetaType* mtype = Meta::instance()->resolve_meta_type(it);
	    generate_enum_or_builtin(mtype);
        }

        if(
            mclass->nb_members(false) != 0 ||
            mclass->nb_custom_attributes() != 0
        ) {
            out() << "   MetaClass* mclass = ";
        } else {
            out() << "   ";
        }

        if(mclass->is_abstract()) {
            out() << "ogf_declare_abstract_class<";
        } else {
            out() << "ogf_declare_class<";
        }
        if(mclass->super_class() == nullptr) {
            if(mclass->name() != "OGF::Object") {
                Logger::warn("GomGen") << mclass->name()
                                       << " has no superclass" << std::endl;
            }
            out() << mclass->name() << ">("
                  << stringify(mclass->name()) ;
            out() << ");" << std::endl;
        } else {
            out() << mclass->name() << ">(" << std::endl
                  << "      "
                  << stringify(mclass->name()) << ", " << std::endl
                  << "      "
                  << stringify(mclass->super_class()->name());
            out() << std::endl
                  << "   );" << std::endl;
        }


        for(unsigned int ii=0; ii<meta_constructors.size(); ii++) {
            MetaConstructor* constructor = meta_constructors[ii];
            out() << "   {" << std::endl;
            out() << "      ";
            out() << "MetaConstructor* cur_constructor = new MetaConstructor("
                  << std::endl
                  << "         "
                  << "mclass"
                  << std::endl
                  << "      );" << std::endl;
            for(index_t i=0; i<constructor->nb_args(); i++) {
                const MetaArg* arg = constructor->ith_arg(i);
                const std::string name = arg->name();
                const std::string type = arg->type_name();
                out() << "      {" << std::endl;
                out() << "         MetaArg cur_arg("
                      << stringify(name) << "," << stringify(type)
                      << ");" << std::endl;
                if(arg->has_default_value()) {
                    out() << "         cur_arg.default_value().set_value("
                          << stringify_default_value(arg)
                          << ");" << std::endl;
                }
                generate_attributes(arg,"cur_arg", false);
                out() << "         cur_constructor->add_arg(cur_arg); "
                      << std::endl;
                out() << "      }" << std::endl;
            }
            out() << "      cur_constructor->set_factory(" << std::endl
                  << "         new "
                  << factory_name(constructor) << "()" << std::endl
                  << "      );" << std::endl;
            generate_attributes(constructor, "cur_constructor");
            out() << "   }" << std::endl;
        }


        for(unsigned int i=0; i<meta_properties.size(); i++) {
            MetaProperty* prop = meta_properties[i];
            out() << "   {" << std::endl;
            out() << "      MetaProperty* cur_prop = new MetaProperty("
                  << std::endl
                  << "         "
                  << stringify(prop->name()) << ", "
                  << "mclass" << ","
                  << stringify(prop->type_name()) << ", "
                  << (prop->read_only() ? "true" : "false")
                  << std::endl
                  << "      );" << std::endl;
            out() << "      ";
            out() << "cur_prop->meta_method_get()->set_method_adapter("
                  << std::endl;
            out() << "         "
                  << method_adapter_name(prop->meta_method_get())
                  << std::endl;
            out() << "      );" << std::endl;

            if(!prop->read_only()) {
                out() << "      " <<
                    "cur_prop->meta_method_set()->set_method_adapter("
                      << std::endl;
                out() << "         "
                      << method_adapter_name(prop->meta_method_set())
                      << std::endl;
                out() << "      );" << std::endl;
            }
            generate_attributes(prop, "cur_prop");
            out() << "   }" << std::endl;
        }

        for(unsigned int ii=0; ii<meta_slots.size(); ii++) {
            MetaSlot* slot = meta_slots[ii];
            out() << "   {" << std::endl;
            out() << "      ";
            out() << "MetaSlot* cur_slot = new MetaSlot("
                  << std::endl
                  << "         "
                  << stringify(slot->name()) << ", "
                  << "mclass" << ", "
                  << stringify(slot->return_type_name())
                  << std::endl
                  << "      );" << std::endl;
            for(index_t i=0; i<slot->nb_args(); i++) {
                const MetaArg* arg = slot->ith_arg(i);
                const std::string name = arg->name();
                const std::string type = arg->type_name();
                out() << "      {" << std::endl;
                out() << "         MetaArg cur_arg("
                      << stringify(name) << "," << stringify(type)
                      << ");" << std::endl;
                if(arg->has_default_value()) {
                    out() << "         cur_arg.default_value().set_value("
                          << stringify_default_value(arg)
                          << ");" << std::endl;
                }
                generate_attributes(arg, "cur_arg", false);
                out() << "         cur_slot->add_arg(cur_arg); "
                      << std::endl;
                out() << "      }" << std::endl;
            }
            out() << "      cur_slot->set_method_adapter(" << std::endl
                  << "         " << method_adapter_name(slot) << std::endl
                  << "      );" << std::endl;

            generate_attributes(slot, "cur_slot");

            out() << "   }" << std::endl;
        }


        for(unsigned int ii=0; ii<meta_signals.size(); ii++) {
            MetaSignal* signal = meta_signals[ii];
            out() << "   {" << std::endl;
            out() << "      ";
            if(signal->nb_args() != 0 || signal->nb_custom_attributes() != 0) {
                out() << "MetaSignal* cur_signal = ";
            }
            out() << "new MetaSignal("
                  << std::endl
                  << "         "
                  << stringify(signal->name()) << ", "
                  << "mclass"
                  << std::endl
                  << "      );" << std::endl;
            for(index_t i=0; i<signal->nb_args(); i++) {
                const MetaArg* arg = signal->ith_arg(i);
                const std::string name = arg->name();
                const std::string type = arg->type_name();
                out() << "      {" << std::endl;
                out() << "         MetaArg cur_arg("
                      << stringify(name) << "," << stringify(type)
                      << ");" << std::endl;
                if(arg->has_default_value()) {
                    out() << "         cur_arg.default_value().set_value("
                          << stringify_default_value(arg)
                          << ");" << std::endl;
                }
                generate_attributes(arg, "cur_arg", false);
                out() << "         cur_signal->add_arg(cur_arg); "
                      << std::endl;
                out() << "      }" << std::endl;
            }
            generate_attributes(signal, "cur_signal");
            out() << "   }" << std::endl;
        }

        generate_attributes(mclass, "mclass");

        out() <<"}" << std::endl;
    }

    void GomCodeGenerator::generate_method_adapter_arglist(
        MetaMethod* method
    ) {
        out() << "static bool " << method_adapter_name(method)
              << "(" << std::endl
              << "   Object* gom__target_in__, " << std::endl
              << "   const std::string& gom__method_name__, " << std::endl
              << "   const ArgList& gom__args__, " << std::endl
              << "   Any& gom__result_any__" << std::endl
              << ") {"
              << std::endl;

        out() << "   " << method->container_meta_class()->name()
              << "* gom__target__ = "
              << "dynamic_cast<" << method->container_meta_class()->name()
              << "*>(" << std::endl
              << "      gom__target_in__" << std::endl
              << "   );" << std::endl;

        out() << "   if(gom__target__ == nullptr) { return false; }"
              << std::endl;

	out() << "   ";
        if(method->return_type_name() != "void") {
            out() << method->return_type()->name() << " "
                  << " gom__result__ = ";
        }
        out() << "gom__target__->" << method->name() << "(gom__args__);";
        out() << std::endl;


        if(method->return_type_name() != "void") {
            out() << "   gom__result_any__.set_value("
                  << "gom__result__);"
                  << std::endl;
	    out() << "   return true;" << std::endl;
        } else {
            out() << "   return true;" << std::endl;
        }

        out() << "}" << std::endl;
        out() << std::endl;
    }

    void GomCodeGenerator::generate_method_adapter(MetaMethod* method) {
        out() << "static bool " << method_adapter_name(method)
              << "(" << std::endl
              << "   Object* gom__target_in__, " << std::endl
              << "   const std::string& gom__method_name__, " << std::endl
              << "   const ArgList& gom__args__, " << std::endl
              << "   Any& gom__result_any__" << std::endl
              << ") {"
              << std::endl;

        out() << "   " << method->container_meta_class()->name()
              << "* gom__target__ = "
              << "dynamic_cast<" << method->container_meta_class()->name()
              << "*>(" << std::endl
              << "      gom__target_in__" << std::endl
              << "   );" << std::endl;

        out() << "   if(gom__target__ == nullptr) { return false; }"
              << std::endl;

        for(index_t i=0; i<method->nb_args(); i++) {
            const MetaArg* arg = method->ith_arg(i);
            std::string name = arg->name();
            std::string type = arg->type()->name();
            out() << "   "  << type << " " << name << "="
                  << "gom__args__.get_arg<" << type << ">("
                  << stringify(name) << ");" << std::endl;
        }


        out() << "   ";
        if(method->return_type_name() != "void") {
            out() << method->return_type()->name() << " "
                  << " gom__result__ = ";
        }
        out() << "gom__target__->" << method->name() << "(";

        for(index_t i=0; i<method->nb_args(); i++) {
            const MetaArg* arg = method->ith_arg(i);
            std::string name = arg->name();
            out() << name;
            if(i < method->nb_args() - 1) {
                out() << ", ";
            }
        }

        out() << "); " << std::endl;

        if(method->return_type_name() != "void") {
            out() << "   gom__result_any__.set_value("
                  << "gom__result__);"
                  << std::endl;
	    out() << "   return true;" << std::endl;
        } else {
            out() << "   return true;" << std::endl;
        }

        out() << "}" << std::endl;
        out() << std::endl;
    }

    void GomCodeGenerator::generate_signal_adapter(MetaSignal* signal) {
        out() << "void "
              << signal->container_meta_class()->name()
              << "::" << signal->name() << "(";

        for(index_t i=0; i<signal->nb_args(); i++) {
            const MetaArg* arg = signal->ith_arg(i);
            std::string type = arg->type()->name();
            std::string name = arg->name();
            if(pass_by_value(arg->type_name())) {
                out() << type << " " << name;
            } else {
                out() << "const " << type << "& " << name;
            }
            if(i < signal->nb_args() - 1) {
                out() << ", ";
            }
        }
        out() << ") {" << std::endl;
        out() << "   ArgList gom__args__;" << std::endl;
        for(index_t i=0; i<signal->nb_args(); i++) {
            const MetaArg* arg = signal->ith_arg(i);
            std::string name = arg->name();
            out() << "   gom__args__.create_arg("
                  << stringify(name)
                  << "," << name
                  << ");" << std::endl;
        }
        out() << "   emit_signal("
              << stringify(signal->name())
              << ", gom__args__);" << std::endl;
        out() << "}" << std::endl;
        out() << std::endl;
    }

    void GomCodeGenerator::generate_factory(MetaConstructor* constructor) {
        out() << "class " << factory_name(constructor) << " : public Factory {"
              << std::endl;
        out() << "   public:" << std::endl;
        out() << "   Object* create(const ArgList& gom__args__) override {"
              << std::endl;

        for(index_t i=0; i<constructor->nb_args(); i++) {
            const MetaArg* arg = constructor->ith_arg(i);
            std::string name = arg->name();
            std::string type = arg->type()->name();

            out() << "      " << type << " " << name << "="
                  << "gom__args__.get_arg<" << type << ">("
                  << stringify(name) << ");" << std::endl;
        }

        out() << "      Object* gom__result__ = new "
              << constructor->container_meta_class()->name()
              << "("
              << std::endl;

        out() << "         ";
        for(index_t i=0; i<constructor->nb_args(); i++) {
            const MetaArg* arg = constructor->ith_arg(i);
            std::string name = arg->name();
            out() << name;
            if(i < constructor->nb_args() - 1) {
                out() << ", ";
            }
        }

        out() << std::endl;
        out() << "      );" << std::endl;
        out() << "      return gom__result__;" << std::endl;
        out() << "   }" << std::endl;
        out() << "};" << std::endl;
        out() << std::endl;
    }

    void GomCodeGenerator::generate_attributes(
        const CustomAttributes* info,
        const std::string& variable_name, bool is_pointer
    ) {
        for(index_t i=0; i<info->nb_custom_attributes(); i++) {
            if(is_pointer) {
                out() << "         " <<
                    variable_name << "->create_custom_attribute(" << std::endl;
            } else {
                out() << "         " <<
                    variable_name << ".create_custom_attribute(" << std::endl;
            }

            out() << "             "
                  << stringify(info->ith_custom_attribute_name(i))
                  << "," << std::endl;
            out() << "             "
                  << stringify(info->ith_custom_attribute_value(i))
                  << std::endl;
            out() << "         );" << std::endl;
        }
    }


    std::string GomCodeGenerator::stringify(const std::string& s) {
        return "\"" + s + "\"";
    }

    std::string GomCodeGenerator::stringify_default_value(
	const MetaArg* marg
    ) {
	std::string result = marg->default_value().as_string();
	MetaType* mtype = marg->type();

	// String literals need quotes.
	// If meta type is std::string, or if meta type name
	// contains Name, then we guess that we need quotes.
	if(
	    mtype == ogf_meta<std::string>::type() ||
	    mtype->name().find("Name") != std::string::npos
	) {
	    result = stringify(result);
	}

	// Make sure nullptr pointer constants have the correct
	// pointer type.
	if(
	    (result == "0" || result == "nullptr" || result == "nil") &&
	    mtype->name()[mtype->name().length()-1] == '*'
	) {
	    result = "(" + mtype->name() + ")(nullptr)";
	}
	return result;
    }


    std::string GomCodeGenerator::colons_to_underscores(const std::string& s) {
        std::string result = s;
        for(size_t i=0; i<s.size(); ++i) {
            if(result[i] == ':') {
                result[i] = '_';
            }
        }
        return result;
    }

    std::string GomCodeGenerator::method_adapter_name(MetaMethod* method) {
        return "gom__" +
            colons_to_underscores(method->container_meta_class()->name()) +
            "__" + method->name() + "__";
    }

    std::string GomCodeGenerator::factory_name(MetaConstructor* method) {
        return "GOM__" +
            colons_to_underscores(method->container_meta_class()->name()) +
            "__" + method->name() + "__";
    }

    bool GomCodeGenerator::pass_by_value(const std::string& type_name) {
        if(type_name[type_name.length() - 1] == '*') {
            return true;
        }
        if(pass_by_value_.find(type_name) != pass_by_value_.end()) {
            return true;
        }
        return false;
    }

}
