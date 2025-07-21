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

#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/reflection/meta_constructor.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/reflection/dynamic_object.h>

/*****************************************************************************/

namespace {
    using namespace OGF;

    /**
     * \brief Gets a member of a specific type by index.
     * \details Indices count only the members of the specified type
     * \param[in] mclass a const pointer to the MetaClass
     * \param[in,out] i the index, on exit, it is reset to 0
     * \param[in] super if true, lookup also in superclasses
     * \return a pointer to the ith member of \p mclass
     * \tparam T the type of the MetaMembers to lookup
     */
    template <class T> T* get_ith_member_recursive(
        const MetaClass* mclass, index_t& i, bool super
    ) {
        for(index_t j=0; j<mclass->nb_members(false); ++j) {
            T* cur = dynamic_cast<T*>(mclass->ith_member(j,false));
            if(cur != nullptr) {
                if(i == 0) {
                    return cur;
                }
                --i;
            }
        }
        if(super && mclass->super_class() != nullptr) {
            return get_ith_member_recursive<T>(
                mclass->super_class(), i, super
            );
        }
        return nullptr;
    }

    /**
     * \brief Gets the number of members of a specific type.
     * \param[in] super if true, lookup also in superclasses
     * \tparam T the type of the members to count
     */
    template <class T> index_t get_nb_members_recursive(
        const MetaClass* mclass, bool super
    ) {
        index_t result = 0;
        for(index_t j=0; j<mclass->nb_members(false); ++j) {
            T* cur = dynamic_cast<T*>(mclass->ith_member(j,false));
            if(cur != nullptr) {
                ++result;
            }
        }
        if(super && mclass->super_class() != nullptr) {
            result += get_nb_members_recursive<T>(mclass->super_class(), super);
        }
        return result;
    }
}

/*****************************************************************************/

namespace OGF {

    MetaClass::MetaClass(
        const std::string& class_name, MetaClass* super_class, bool abstract
    ) : MetaType(class_name),
        super_class_name_(super_class ? super_class->name() : std::string()) {
        abstract_ = abstract;
        if(!abstract) {
            set_factory(new FactoryMetaClass(this));
        }
	instance_ = nullptr;
    }

    MetaClass::MetaClass(
        const std::string& class_name, const std::string& super_class_name,
        bool abstract
    ) : MetaType(class_name), super_class_name_(super_class_name) {
        abstract_ = abstract;
        if(!abstract) {
            set_factory(new FactoryMetaClass(this));
        }
    }

    MetaClass::~MetaClass() {
    }

    Object* MetaClass::create(const ArgList& args) {
        Object* result = nullptr;

        if(is_abstract()) {
            Logger::err("GOM")
                << this->name()
                << " is abstract (cannot create() instances)"
                << std::endl;
            return nullptr;
        }

        MetaConstructor* constructor = best_constructor(args);

        if(constructor == nullptr) {
            Logger::err("GOM")
                << this->name()
                << " does not have a matching constructor"
                << " (missing arg?)"
                << std::endl;
            for(unsigned int i=0; i<args.nb_args(); i++) {
                Logger::err("Interpreter") << "arg " << i << " name= "
                                           << args.ith_arg_name(i)
                                           << " value= "
					   << args.ith_arg_value(i).as_string()
                                           << std::endl;
            }
            return nullptr;
        }

        result = factory()->create(args);

        if(result == nullptr) {
            Logger::err("GOM")
                << this->name() << " : could not create object"
                << std::endl;
            return nullptr;
        }

        // Args that are not used by the constructor are set as properties
        if(constructor != nullptr) {
            for(unsigned int i=0; i<args.nb_args(); i++) {
                if(!constructor->has_arg(args.ith_arg_name(i))) {
                    MetaProperty* mprop = find_property(args.ith_arg_name(i));
                    if(mprop != nullptr && !mprop->read_only()) {
                        result->set_property(
                            args.ith_arg_name(i), args.ith_arg_value(i)
                        );
                    }
                }
            }
        }

        return result;
    }

    void MetaClass::pre_delete() {
	MetaType::pre_delete();
        for(index_t i=0; i<nb_members(false); ++i) {
            ith_member(i,false)->pre_delete();
        }
    }

    MetaClass* MetaClass::super_class() const {
        MetaType* super_type = Meta::instance()->resolve_meta_type(
            super_class_name_
        );
        return dynamic_cast<MetaClass*>(super_type);
    }

    size_t MetaClass::nb_members(bool super) const {
        size_t result = members_.size();
        if(super && super_class() != nullptr) {
            result += super_class()->nb_members(true);
        }
        return result;
    }

    MetaMember* MetaClass::ith_member(index_t i, bool super) const {
        ogf_assert(i < nb_members(super));
        MetaMember* result = nullptr;
        if(super) {
            result = get_ith_member_recursive<MetaMember>(this, i, super);
        } else {
            result = members_[i];
        }
        return result;
    }

    size_t MetaClass::nb_signals(bool super) const {
        return get_nb_members_recursive<MetaSignal>(this, super);
    }

    MetaSignal* MetaClass::ith_signal(index_t i, bool super) const {
        return get_ith_member_recursive<MetaSignal>(this, i, super);
    }

    size_t MetaClass::nb_slots(bool super) const {
        return get_nb_members_recursive<MetaSlot>(this, super);
    }

    MetaSlot* MetaClass::ith_slot(index_t i, bool super) const {
        return get_ith_member_recursive<MetaSlot>(this, i, super);
    }

    size_t MetaClass::nb_properties(bool super) const {
        return get_nb_members_recursive<MetaProperty>(this, super);
    }

    MetaProperty* MetaClass::ith_property(index_t i, bool super) const {
        return get_ith_member_recursive<MetaProperty>(this, i, super);
    }

    size_t MetaClass::nb_constructors() const {
        return get_nb_members_recursive<MetaConstructor>(this, false);
    }

    MetaConstructor* MetaClass::ith_constructor(index_t i) const {
        return get_ith_member_recursive<MetaConstructor>(this, i, false);
    }

    void MetaClass::get_constructors(
        std::vector<MetaConstructor*>& result
    ) const {
        result.clear();
        for(unsigned int i=0; i<members_.size(); i++) {
            MetaMember* cur = members_[i];
            MetaConstructor* cur_constructor =
                dynamic_cast<MetaConstructor*>(cur);
            if(cur_constructor != nullptr) {
                result.push_back(cur_constructor);
            }
        }
    }

    MetaMember* MetaClass::find_member(
        const std::string& member_name, bool super
    ) const {
        for(unsigned int i=0; i<members_.size(); i++) {
            MetaMember* cur = members_[i];
            if(cur->name() == member_name) {
                return cur;
            }
        }
        if(super && super_class() != nullptr) {
            return super_class()->find_member(member_name, true);
        }
        return nullptr;
    }

    MetaMethod* MetaClass::find_method(
        const std::string& member_name, bool super
    ) const {
        if(String::string_starts_with(member_name, "get_")) {
            std::string property_name =
                member_name.substr(4, member_name.length()-4);
            MetaProperty* mprop = find_property(property_name, super);
            if(mprop != nullptr) {
                return mprop->meta_method_get();
            }
        }
        if(String::string_starts_with(member_name, "set_")) {
            std::string property_name =
                member_name.substr(4, member_name.length()-4);
            MetaProperty* mprop = find_property(property_name, super);
            if(mprop != nullptr) {
                return mprop->meta_method_set();
            }
        }
        return dynamic_cast<MetaMethod*>(find_member(member_name, super));
    }

    MetaSignal* MetaClass::find_signal(
        const std::string& signal_name, bool super
    ) const {
        return dynamic_cast<MetaSignal*>(find_member(signal_name, super));
    }

    MetaSlot* MetaClass::find_slot(
        const std::string& slot_name, bool super
    ) const {
        return dynamic_cast<MetaSlot*>(find_member(slot_name, super));
    }

    MetaProperty* MetaClass::find_property(
        const std::string& property_name, bool super
    ) const {
        return dynamic_cast<MetaProperty*>(find_member(property_name, super));
    }


    std::string MetaClass::new_constructor_name() const {
        std::string num_str;
        int num = int(nb_constructors()) + 1;
        ogf_convert_to_string(num , num_str);
        return "constructor_" + num_str;
    }

    void MetaClass::append_members(
        std::vector<MetaMember*>& result, bool super
    ) const {
        for(unsigned int i=0; i<members_.size(); i++) {
            MetaMember* cur = members_[i];
            result.push_back(cur);
        }
        if(super && super_class() != nullptr) {
            super_class()->append_members(result, true);
        }
    }

    void MetaClass::append_methods(
        std::vector<MetaMethod*>& result, bool super
    ) const {
        for(unsigned int i=0; i<members_.size(); i++) {
            MetaMember* cur = members_[i];
            MetaMethod* cur_method = dynamic_cast<MetaMethod*>(cur);
            if(cur_method != nullptr) {
                result.push_back(cur_method);
            }
        }
        if(super && super_class() != nullptr) {
            super_class()->append_methods(result, true);
        }
    }

    void MetaClass::append_signals(
        std::vector<MetaSignal*>& result, bool super
    ) const {
        for(unsigned int i=0; i<members_.size(); i++) {
            MetaMember* cur = members_[i];
            MetaSignal* cur_signal = dynamic_cast<MetaSignal*>(cur);
            if(cur_signal != nullptr) {
                result.push_back(cur_signal);
            }
        }
        if(super && super_class() != nullptr) {
            super_class()->append_signals(result, true);
        }
    }

    void MetaClass::append_slots(
        std::vector<MetaSlot*>& result, bool super
    ) const {
        for(unsigned int i=0; i<members_.size(); i++) {
            MetaMember* cur = members_[i];
            MetaSlot* cur_slot = dynamic_cast<MetaSlot*>(cur);
            if(cur_slot != nullptr) {
                result.push_back(cur_slot);
            }
        }
        if(super && super_class() != nullptr) {
            super_class()->append_slots(result, true);
        }
    }

    void MetaClass::append_properties(
        std::vector<MetaProperty*>& result, bool super
    ) const {
        for(unsigned int i=0; i<members_.size(); i++) {
            MetaMember* cur = members_[i];
            MetaProperty* cur_prop = dynamic_cast<MetaProperty*>(cur);
            if(cur_prop != nullptr) {
                result.push_back(cur_prop);
            }
        }
        if(super && super_class() != nullptr) {
            super_class()->append_properties(result, true);
        }
    }

    bool MetaClass::is_subclass_of(const MetaClass* other) const {
        if(this == other) {
            return true;
        }
        if(super_class() != nullptr) {
            return super_class()->is_subclass_of(other);
        }
        return false;
    }

    Object* MetaClass::get_instance() const {
	return instance_;
    }

    bool MetaClass::is_subtype_of(const MetaType* other) const {
        const MetaClass* other_as_mclass = dynamic_cast<const MetaClass*>(other);
        if(other_as_mclass == nullptr) {
            return false;
        }
        return is_subclass_of(other_as_mclass);
    }

    void MetaClass::get_used_types(
        std::set<std::string>& used_types, bool super
    ) const {
        for(unsigned int i=0; i<members_.size(); i++) {
            MetaMember* cur_member = members_[i];

            MetaMethod* cur_method = dynamic_cast<MetaMethod*>(cur_member);
            if(cur_method != nullptr) {
                used_types.insert(cur_method->return_type_name());
                for(index_t j=0; j<cur_method->nb_args(); j++) {
                    used_types.insert(cur_method->ith_arg(j)->type_name());
                }
                continue;
            }

            MetaProperty* cur_property = dynamic_cast<MetaProperty*>(
                cur_member
            );

            if(cur_property != nullptr) {
                used_types.insert(cur_property->type_name());
            }
        }
        if(super && super_class() != nullptr) {
            super_class()->get_used_types(used_types);
        }
    }

    MetaConstructor* MetaClass::best_constructor(const ArgList& args) {
        MetaConstructor* best_so_far = nullptr;
        index_t nb_used_args = 0;
        std::vector<MetaConstructor*> constructors;
        get_constructors(constructors);
        if(args.has_unnamed_args()) {
            for(MetaConstructor* cur: constructors) {
                if(args.nb_args() == cur->nb_args()) {
                    best_so_far = cur;
                }
            }
        } else {
            for(MetaConstructor* cur: constructors) {
                if(cur->check_args(args)) {
                    index_t cur_nb_used_args = cur->nb_used_args(args);
                    if(cur_nb_used_args >= nb_used_args) {
                        best_so_far = cur;
                        nb_used_args = cur_nb_used_args;
                    }
                }
            }
        }
        return best_so_far;
    }


    MetaClass* MetaClass::create_subclass(
        const std::string& name, bool is_abstract
    ) {
        MetaClass* result = new DynamicMetaClass(
            name, this->name(), is_abstract
        );
        Meta::instance()->bind_meta_type(result);
        return result;
    }

    void MetaClass::search(const std::string& needle, const std::string& path) {
        MetaInformation::search(needle, path);
        if(path.find(needle) != std::string::npos) {
            return;
        }
        std::vector<MetaMember*> members;
        get_members(members, true);
        for(MetaMember* member: members) {
            member->search(needle, path + "." + member->name());
        }
    }

    std::string MetaClass::get_doc() const {
        std::string result;
        // Discard uninteresting doc about the MetaClass
        // of the MetaClass !!
        if(this == ogf_meta<MetaClass>::type()) {
            return result;
        }
        result = name();
        if(has_custom_attribute("help")) {
            result += "\n";
            result += custom_attribute_value("help");
        }
        return result;
    }

/****************************************************************************/

    Object* FactoryMetaClass::create(const ArgList& args) {
        MetaConstructor* best_constructor =
            meta_class()->best_constructor(args);

        if(best_constructor == nullptr) {
            return nullptr;
        }

        ogf_assert(best_constructor->factory() != nullptr);
        Object* result = nullptr;

        if(args.has_unnamed_args()) {
            geo_assert(args.nb_args() == best_constructor->nb_args());
            ArgList named_args;
            for(index_t i=0; i<args.nb_args(); ++i) {
                named_args.create_arg(
                    best_constructor->ith_arg_name(i),
                    args.ith_arg_value(i)
                );
            }
            result = best_constructor->factory()->create(named_args);
        } else {
            if(best_constructor->nb_default_args(args) != 0) {
                ArgList all_args = args;
                best_constructor->add_default_args(all_args);
                result = best_constructor->factory()->create(all_args);
            } else
                result = best_constructor->factory()->create(args);
        }
        result->meta_class(); // initializes metaclass information.

        return result;
    }



}
