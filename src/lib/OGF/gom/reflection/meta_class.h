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

#ifndef H_OGF_META_TYPES_META_CLASS_H
#define H_OGF_META_TYPES_META_CLASS_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/reflection/meta_type.h>
#include <OGF/gom/reflection/meta_arg.h>
#include <OGF/gom/reflection/meta_signal.h>
#include <OGF/gom/reflection/meta_slot.h>
#include <OGF/gom/reflection/meta_constructor.h>
#include <OGF/gom/reflection/meta_property.h>
#include <OGF/gom/services/factory.h>

#include <set>
#include <map>

/**
 * \file OGF/gom/reflection/meta_class.h
 * \brief MetaType for classes
 */

namespace OGF {

    class ArgList;

    /**
     * \brief The representation of a class in the Meta repository.
     */
    gom_class GOM_API MetaClass : public MetaType {
    public:

        /**
         * \brief Constructs a new MetaClass
         * \param[in] class_name the C++ class name
         * \param[in] super_class a pointer to the super class, or nullptr
         *  if there is no super class
         * \param[in] is_abstract indicates whether the class is abstract
         *  (e.g. with pure virtual methods) or not. Abstract classes
         *  cannot be constructed.
         */
        explicit MetaClass(
            const std::string& class_name, MetaClass* super_class = nullptr,
            bool is_abstract = false
        );

        /**
         * \brief Constructs a new MetaClass
         * \param[in] class_name the C++ class name
         * \param[in] super_class_name the C++ name of the super class
         * \param[in] is_abstract indicates whether the class is abstract
         *  (e.g. with pure virtual methods) or not. Abstract classes
         *  cannot be constructed.
         */
        explicit MetaClass(
            const std::string& class_name,
            const std::string& super_class_name,
            bool is_abstract = false
        );

        /**
         * \brief MetaClass destructor.
         */
	~MetaClass() override;

	/**
	 * \copydoc MetaType::pre_delete()
	 */
	void pre_delete() override;

    gom_slots:

        /**
         * \brief Gets the super class.
         * \return a pointer to the super class, or nullptr if there
         *  is no super class.
         */
        MetaClass* super_class() const;

        /**
         * \brief Gets the name of the super class.
         * \return the C++ name of the super class
         */
        const std::string& super_class_name() const {
            return super_class_name_;
        }

        /**
         * \brief Tests if the class is abstract.
         * \retval true if the class is abstract
         * \retval false otherwise
         */
        bool is_abstract() const {
            return abstract_;
        }

        /**
         * \brief Creates an object of this class.
         * \param args the parameters to be passed to the constructor.
         *  The constructor that best matches the parameters list is selected.
         * \return the created object, or nullptr if no matching constructor was
         *  found or if the class is abstract.
         */
        Object* create(const ArgList& args);

        /**
         * \brief Gets the number of class members.
         * \details Class members are constructors,
         *  signals, slots or properties.
         * \param[in] super if true, counts also the
         *  inherited members, else only
         *  the members declared in this class
         * \return the number of members
         */
        size_t nb_members(bool super = true) const;

        /**
         * \brief Gets a MetaMember by index.
         * \param[in] i index of the MetaMember
         * \param[in] super if true, lookup also in superclasses
         * \pre i < nb_members(super)
         */
        MetaMember* ith_member(index_t i, bool super = true) const;

        /**
         * \brief Finds a MetaMember by name
         * \param[in] member_name name of the member
         * \param[in] super if true, search also in
         *  members inherited from super class
         * \return a pointer to the found MetaMember or nullptr
         */
        MetaMember* find_member(
            const std::string& member_name, bool super = true
        ) const;


        /**
         * \brief Gets the number of signals.
         * \param[in] super if true, counts also the
         *  inherited signals, else only
         *  the signals declared in this class
         * \return the number of signals
         */
        size_t nb_signals(bool super = true) const;

        /**
         * \brief Gets a MetaSignal by index.
         * \param[in] i index of the signal
         * \param[in] super if true, lookup also in superclasses
         * \pre i < nb_signals(super)
         */
        MetaSignal* ith_signal(index_t i, bool super = true) const;

        /**
         * \brief Gets the number of slots.
         * \param[in] super if true, counts also the
         *  inherited slots, else only
         *  the slots declared in this class
         * \return the number of slots
         */
        size_t nb_slots(bool super = true) const;

        /**
         * \brief Gets a MetaSlot by index.
         * \param[in] i index of the slot
         * \param[in] super if true, lookup also in superclasses
         * \pre i < nb_slots(super)
         */
        MetaSlot* ith_slot(index_t i, bool super = true) const;

        /**
         * \brief Gets the number of properties.
         * \param[in] super if true, counts also the
         *  inherited properties, else only
         *  the slots declared in this class
         * \return the number of properties
         */
        size_t nb_properties(bool super = true) const;


        /**
         * \brief Gets a MetaProperty by index.
         * \param[in] i index of the property
         * \param[in] super if true, lookup also in superclasses
         * \pre i < nb_properties(super)
         */
        MetaProperty* ith_property(index_t i, bool super = true) const;

        /**
         * \brief Gets the number of constructors.
         * \return the number of constructors
         */
        size_t nb_constructors() const;

        /**
         * \brief Gets a MetaConstructor by index.
         * \param[in] i index of the property
         * \pre i < nb_constructors()
         */
        MetaConstructor* ith_constructor(index_t i) const;

        /**
         * \brief Finds a MetaMethod by name.
         * \details Both signals, slots and constructors
         *  are methods.
         * \param[in] method_name name of the method
         * \param[in] super if true, search also in
         *  members inherited from super class
         * \return a pointer to the found MetaMethod or nullptr
         */
        MetaMethod* find_method(
            const std::string& method_name, bool super = true
        ) const;

        /**
         * \brief Finds a MetaSlot by name
         * \param[in] slot_name name of the slot
         * \param[in] super if true, search also in
         *  members inherited from super class
         * \return a pointer to the found MetaMethod or nullptr
         */
        MetaSlot* find_slot(
            const std::string& slot_name, bool super = true
        ) const;

        /**
         * \brief Finds a MetaSignal by name
         * \param[in] signal_name name of the signal
         * \param[in] super if true, search also in
         *  members inherited from super class
         * \return a pointer to the found MetaSignal or nullptr
         */
        MetaSignal* find_signal(
            const std::string& signal_name, bool super = true
        ) const;

        /**
         * \brief Finds a MetaProperty by name
         * \param[in] property_name name of the property
         * \param[in] super if true, search also in
         *  members inherited from super class
         * \return a pointer to the found MetaProperty or nullptr
         */
        MetaProperty* find_property(
            const std::string& property_name, bool super = true
        ) const;

        /**
         * \brief Creates a new subclass dynamically
         * \param[in] name the name of the subclass to be created
         * \param[in] is_abstract true if the class is abstract, that is, if
         *  no object of this class can be created
         * \return the newly created subclass
         * \details To be used in scripts that create new classes dynamically
         */
        virtual MetaClass* create_subclass(
            const std::string& name, bool is_abstract=false
        );

        /**
         * \brief Tests whether this MetaClass is a subclass of another MetaClass
         * \param[in] other a pointer to a MetaClass
         * \retval true if this MetaClass is the same or is a subtype of \p other
         * \retval false otherwise
         */
        virtual bool is_subclass_of(const MetaClass* other) const;

    public:

        /**
         * \brief Indicate that the class is abstract.
         * \param[in] b true if the class is abstract,
         *  false otherwise
         */
        void set_abstract(bool b) {
            abstract_ = b;
        }


        /**
         * \brief Adds a new MetaMember to this class
         * \param[in] member a pointer to the MetaMember
         *  to be added. Ownership is transfered to this
         *  MetaClass.
         */
        void add_member(MetaMember* member) {
            members_.push_back(member);
        }

        /**
         * \brief Gets all the members
         * \param[out] result a vector of MetaMember pointers
         * \param[in] super if true, gets also the MetaMembers
         *  inherited from super classes
         */
        void get_members(
            std::vector<MetaMember*>& result, bool super = true
        ) const {
            result.clear();
            append_members(result, super);
        }

        /**
         * \brief Gets all the methods.
         * \details Both signals, slots and constructors
         *  are methods.
         * \param[out] result a vector of MetaMethod pointers
         * \param[in] super if true, gets also the MetaMethods
         *  inherited from super classes
         */
        void get_methods(
            std::vector<MetaMethod*>& result, bool super = true
        ) const {
            result.clear();
            append_methods(result, super);
        }

        /**
         * \brief Gets all the signals.
         * \param[out] result a vector of MetaSignal pointers
         * \param[in] super if true, gets also the MetaSignals
         *  inherited from super classes
         */
        void get_signals(
            std::vector<MetaSignal*>& result, bool super = true
        ) const {
            result.clear();
            append_signals(result, super);
        }

        /**
         * \brief Gets all the slots.
         * \param[out] result a vector of MetaSlot pointers
         * \param[in] super if true, gets also the MetaSlots
         *  inherited from super classes
         */
        void get_slots(
            std::vector<MetaSlot*>& result, bool super = true
        ) const {
            result.clear();
            append_slots(result, super);
        }

        /**
         * \brief Gets all the properties.
         * \param[out] result a vector of MetaProperty pointers
         * \param[in] super if true, gets also the MetaProperties
         *  inherited from super classes
         */
        void get_properties(
            std::vector<MetaProperty*>& result, bool super = true
        ) const {
            result.clear();
            append_properties(result, super);
        }

        /**
         * \brief Gets all the constructors.
         * \param[out] result a vector of MetaConstructor pointers
         */
        void get_constructors(std::vector<MetaConstructor*>& result) const;

        /**
         * \copydoc MetaType::is_subtype_of()
         */
        bool is_subtype_of(const MetaType* other) const override;

        /**
         * \brief Gets all the types used by this class.
         * \details A type is used by this class if it appears as an
         *  argument of a method or if the class has a property of this
         *  type.
         * \param[out] used_types a set of strings with the C++ type names
         *  of the used types
         * \param[in] super if true, gets also the types used by super classes
         */
        void get_used_types(
            std::set<std::string>& used_types, bool super = true
        ) const;

        /**
         * \brief Gets the best constructors for the specified
         *  arguments.
         * \details The best constructor is the one that uses the largest
         *  number of arguments.
         * \param[in] args the arguments, as name-value pairs
         * \return a pointer to the MetaConstructor that corresponds to the
         *  best constructor.
         */
        MetaConstructor* best_constructor(const ArgList& args);

        /**
         * \brief Gets the factory.
         * \return a pointer to the factory associated with this class,
         *  or nullptr if there is no factory.
         *  The returned factory is typically a FactoryMetaClass,
         *  that selects the best constructor according to the arguments.
         */
        Factory* factory() const {
            return factory_;
        }

        /**
         * \brief Sets the factory.
         * \param[in] f a pointer to the factory to be associated with
         *  this class.
         */
        void set_factory(Factory* f) {
            factory_ = f;
        }

        /**
         * \copydoc Object::search()
         */
        void search(const std::string& needle, const std::string& path = "") override;

        /**
         * \copydoc Object::get_doc()
         */
        std::string get_doc() const override;


    protected:

        /**
         * \brief Generates a dummy name for a constructor.
         * \details Each MetaMember is supposed to have a name,
         *  this function generates a unique name for each
         *  constructor ("constructor_nnn").
         * \return a unique constructor name.
         */
        std::string new_constructor_name() const;

        /**
         * \brief Gets all the members and appends them to a vector.
         * \param[in,out] result a vector of MetaMember pointers
         * \param[in] super if true, gets also the MetaMembers
         *  inherited from super classes
         */
        void append_members(
            std::vector<MetaMember*>& result, bool super = true
        ) const;

        /**
         * \brief Gets all the methods and appends them to a vector.
         * \details Both signals, slots and constructors
         *  are methods.
         * \param[in,out] result a vector of MetaMethod pointers
         * \param[in] super if true, gets also the MetaMethods
         *  inherited from super classes
         */
        void append_methods(
            std::vector<MetaMethod*>& result, bool super = true
        ) const;

        /**
         * \brief Gets all the signals and appends them to a vector.
         * \param[in,out] result a vector of MetaSignal pointers
         * \param[in] super if true, gets also the MetaSignals
         *  inherited from super classes
         */
        void append_signals(
            std::vector<MetaSignal*>& result, bool super = true
        ) const;

        /**
         * \brief Gets all the slots and appends them to a vector.
         * \param[in,out] result a vector of MetaSlot pointers
         * \param[in] super if true, gets also the MetaSlots
         *  inherited from super classes
         */
        void append_slots(
            std::vector<MetaSlot*>& result, bool super = true
        ) const;

        /**
         * \brief Gets all the properties.
         * \param[in,out] result a vector of MetaProperty pointers
         * \param[in] super if true, gets also the MetaProperties
         *  inherited from super classes
         */
        void append_properties(
            std::vector<MetaProperty*>& result, bool super = true
        ) const;

    private:
        std::string super_class_name_;
        std::vector<MetaMember_var> members_;
        bool abstract_;
        Factory_var factory_;
        friend class ::OGF::MetaConstructor;
    };

    /**
     * \brief An automatic reference-counted pointer to a MetaClass.
     */
    typedef SmartPointer<MetaClass> MetaClass_var;

    /*******************************************************************/

    /**
     * \brief A Factory that uses a MetaClass.
     */
    class GOM_API FactoryMetaClass : public Factory {
    public:
        /**
         * \brief FactoryMetaClass constructor
         * \param[in] mclass a pointer to the meta class
         */
        FactoryMetaClass(MetaClass* mclass) : meta_class_(mclass) {
        }

        /**
         * \brief Creates an objet.
         * \details Selects the best constructor according to the
         *  arguments.
         * \param[in] args a const reference to the list of arguments
         * \return the created object
         * \see MetaClass::best_constructor()
         */
        virtual Object* create(const ArgList& args);

        /**
         * \brief Gets the MetaClass.
         * \return a pointer to the MetaClass.
         */
        MetaClass* meta_class() const {
            return meta_class_;
        }

    private:
        MetaClass* meta_class_;
    };

    /*******************************************************************/
}
#endif
