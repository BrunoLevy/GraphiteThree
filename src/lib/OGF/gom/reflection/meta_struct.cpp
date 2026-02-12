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

#include <OGF/gom/reflection/meta_struct.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/interpreter/interpreter.h>

namespace {
    using namespace OGF;

    /**
     * \brief A Serializer that uses a MetaStruct
     */
    class BuiltinStructSerializer : public Serializer {
    public:
	BuiltinStructSerializer(
	    MetaStruct* meta_struct
	) : meta_struct_(meta_struct) {
	}

	/**
	 * \copydoc Serializer::serialize_read()
	 */
	bool serialize_read(std::istream& stream, void* addr) override {
	    index_t nb_fields = index_t(meta_struct_->nb_properties(false));
	    std::string line;
	    std::getline(stream,line);
	    std::vector<std::string> words;
	    String::split_string(line, ';', words);
	    if(words.size() != nb_fields) {
		Logger::err("GOM") << meta_struct_->name() << "serialize read: "
				   << "Invalid number of fields" << std::endl;
		return false;
	    }
	    return for_each_field(
		[addr,&words](
		    index_t field_id, Serializer* serializer, size_t offset
		)->bool	{
		    std::istringstream in(words[field_id]);
		    return serializer->serialize_read(
			in,Memory::pointer(addr)+offset
		    );
		}
	    );
	}

	/**
	 * \copydoc Serializer::serialize_write()
	 */
	bool serialize_write(std::ostream& stream, void* addr) override {
	    index_t nb_fields = index_t(meta_struct_->nb_properties(false));
	    return for_each_field(
		[addr,&stream,nb_fields](
		    index_t field_id, Serializer* serializer, size_t offset
		)->bool	{
		    if(
			!serializer->serialize_write(
			    stream,Memory::pointer(addr)+offset
			)
		    ) return false;
		    if(field_id != nb_fields - 1) {
			stream << ';';
		    }
		    return true;
		}
	    );
	}

    protected:

	/**
	 * \brief Common implementation of serialize_read() and serialize_write()
	 * \details Calls a lambda for each field of the meta struct
	 * \return true on success, false on failure
	 */
	bool for_each_field(
	    std::function<bool(
	       index_t field_id, Serializer* serializer, size_t offset
	    )> do_field
	) {
	    index_t nb_fields = index_t(meta_struct_->nb_properties(false));
	    for(index_t i=0; i<nb_fields; ++i) {
		MetaProperty* mprop = meta_struct_->ith_property(i, false);
		MetaType* mtype = mprop->type();
		Serializer* serializer = mtype->serializer();
		if(serializer == nullptr) {
		    Logger::err("GOM")
			<< meta_struct_->name()
			<< "missing serializer for " << mtype->name()
			<< std::endl;
		    return false;
		}
		size_t offset = 0;
		if(!mprop->has_custom_attribute("offset")) {
		    Logger::err("GOM")
			<< meta_struct_->name()
			<< "missing offset for " << mprop->name()
			<< std::endl;
		    return false;
		}
		if(
		    !String::from_string(
			mprop->custom_attribute_value("offset"), offset
		    )
		) {
		    Logger::err("GOM")
			<< meta_struct_->name()
			<< "invalid offset for " << mprop->name()
			<< std::endl;
		    return false;
		}

		if(!do_field(i, serializer, offset)) {
		    Logger::err("GOM")
			<< meta_struct_->name()
			<< " could not serialize " << mprop->name()
			<< std::endl;
		    return false;
		}
	    }
	    return true;
	}

    private:
	MetaStruct* meta_struct_;
    };
}

/*****************************************************************************/

namespace OGF {

    MetaStruct::MetaStruct(
	const std::string& struct_name
    ) :  MetaClass(struct_name, "OGF::Object") {
    }

    MetaStruct::~MetaStruct() {
    }

    size_t MetaStruct::offset(const MetaProperty* mprop) const {
	geo_assert(mprop->has_custom_attribute("offset"));
	size_t result;
	bool ok = String::from_string(
	    mprop->custom_attribute_value("offset"), result
	);
	geo_assert(ok);
	return result;
    }

    index_t MetaStruct::field_index(const MetaProperty* mprop) const {
	index_t nb_fields = index_t(nb_properties(false));
	for(index_t i=0; i<nb_fields; ++i) {
	    if(ith_property(i,false) == mprop) {
		return i;
	    }
	}
	return NO_INDEX;
    }

    /****************************************************************/

    MetaBuiltinStruct::MetaBuiltinStruct(
	const std::string& struct_name
    ) :  MetaBuiltinType(struct_name) {
	meta_struct_ = new MetaStruct(
	    struct_name + "_as_GOM_Object"
	);
	Meta::instance()->bind_meta_type(meta_struct_);
	set_serializer(new BuiltinStructSerializer(meta_struct_));
    }

    MetaBuiltinStruct::~MetaBuiltinStruct() {
	meta_struct_ = nullptr;
    }

    MetaProperty* MetaBuiltinStruct::add_property_by_typeid_name(
	const std::string& property_name,
	const std::string& typeid_name
    ) {
	MetaType* mtype = Meta::instance()->resolve_meta_type_by_typeid_name(
	    typeid_name
	);
	if(mtype == nullptr) {
	    Logger::err("GOM") << "Missing meta information for"
			       << typeid_name
			       << std::endl;
	}
	geo_assert(mtype != nullptr);
	return new MetaProperty(property_name, meta_struct_, mtype);
    }

    /****************************************************************/

    StructPropertyRef::StructPropertyRef(
	Object* object, const std::string& prop_name
    ) : object_(object), property_name_(prop_name) {
	MetaProperty* mprop = object_->meta_class()->find_property(prop_name);
	geo_assert(mprop != nullptr);
	MetaBuiltinStruct* mbstruct = dynamic_cast<MetaBuiltinStruct*>(
	    mprop->type()
	);
	geo_assert(mbstruct != nullptr);
	set_meta_class(mbstruct->get_meta_struct());
    }

    bool StructPropertyRef::set_property(
	const std::string& prop_name, const Any& prop_value
    ) {
	// Always use string version (less elegant, but then we have no
	// type conversion issue)
	return set_property(prop_name, prop_value.as_string());
    }

    bool StructPropertyRef::set_property(
	const std::string& name, const std::string& value
    ) {
	MetaStruct* mstruct = dynamic_cast<MetaStruct*>(meta_class());
	geo_assert(mstruct != nullptr);
	const MetaProperty* mprop = mstruct->find_property(name, false);
	if(mprop == nullptr) {
	    return Object::set_property(name, value);
	}

	std::string struct_value_string;
	if(!object_->get_property(property_name_, struct_value_string)) {
	    return false;
	}

	index_t field_id = mstruct->field_index(mprop);
	if(field_id == NO_INDEX) {
	    Logger::err("GOM") << "Did not find field in struct"
			       << std::endl;
	    return false;
	}
	std::vector<std::string> fields;
	String::split_string(struct_value_string, ';', fields);
	if(fields.size() != index_t(mstruct->nb_properties(false))) {
	    Logger::err("GOM") << "Invalid struct property value in object"
			       << std::endl;
	    return false;
	}
	geo_assert(field_id < index_t(fields.size()));
	fields[field_id] = value;
	bool result = object_->set_property(
	    property_name_, String::join_strings(fields,';')
	);

	Interpreter* interpreter = Interpreter::default_interpreter();
	if(interpreter != nullptr) {
	    Any value_as_any;
	    value_as_any.set_value(value);
	}
	return result;
    }

    bool StructPropertyRef::get_property(
	const std::string& prop_name, std::string& prop_value
    ) const {
	// string version, use baseclass implementation that
	// converts and routes to the version using Any
	return Object::get_property(prop_name, prop_value);
    }

    bool StructPropertyRef::get_property(
	const std::string& prop_name, Any& prop_value
    ) const {
	MetaStruct* mstruct = dynamic_cast<MetaStruct*>(meta_class());
	geo_assert(mstruct != nullptr);
	const MetaProperty* mprop = mstruct->find_property(
	    prop_name, false
	);
	if(mprop == nullptr) {
	    return Object::get_property(prop_name, prop_value);
	}
	Any struct_value;
	if(!object_->get_property(property_name_, struct_value)) {
	    return false;
	}
	size_t offset = mstruct->offset(mprop);
	prop_value.copy_from(struct_value.data() + offset, mprop->type());
	return true;
    }

    std::string StructPropertyRef::to_string() const {
	MetaStruct* mstruct = dynamic_cast<MetaStruct*>(meta_class());
	geo_assert(mstruct != nullptr);
	index_t nb_fields = index_t(mstruct->nb_properties(false));
	std::vector<std::string> field(nb_fields);
	for(index_t i=0; i<nb_fields; ++i) {
	    get_property(
		mstruct->ith_property(i,false)->name(), field[i]
	    );
	}
	return String::join_strings(field,';');
    }

    /****************************************************************/

}
