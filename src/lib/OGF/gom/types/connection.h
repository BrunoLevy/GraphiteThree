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

#ifndef H_OGF_GOM_TYPES_CONNECTION_H
#define H_OGF_GOM_TYPES_CONNECTION_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/types/callable.h>

#include <string>
#include <vector>
#include <set>

/**
 * \file OGF/gom/types/connection.h
 * \brief Implementation of the signal-slot connection mechanism.
 */
namespace OGF {

    class ConnectionList;

    /******************************************************************/

    /**
     * A connection connects a signal with a function. The arguments
     * are matched between the signal and callable by their names.
     * If argument names mismatch between the signal and the slot,
     * a connection can rename/add/remove arguments before calling
     * the target's slot. In addition, the target's slot can be
     * invoked conditionally, depending of the value of some arguments.
     */
    gom_attribute(abstract, "true")
    gom_class GOM_API Connection : public Callable {
    public:

        /**
         * \brief Connection constructor.
         * \param[in] source a pointer to the source object
         * \param[in] sig_name the name of the source's signal
         */
        Connection(
	    Object* source, const std::string& sig_name
	);

        /**
         * \brief Connection destructor
         */
        ~Connection() override;

        /**
         * \brief Invokes the target with an arguments list.
         * \details The arguments are translated before being sent
         *  to the target.
         * \param[in] args a const reference to the arguments list
	 * \param[out] ret_val on exit, contains "void".
         */
	bool invoke(const ArgList& args, Any& ret_val) override;


        /**
         * \brief Directly invokes the target with an arguments list,
	 *   without any argument translation.
         * \param[in] args a const reference to the arguments list
	 * \param[out] ret_val on exit, contains "void".
         */
	virtual bool invoke_target(
	    const ArgList& args, Any& ret_val
	) = 0;

	/**
	 * \brief Gets the source.
	 * \return a pointer to the source object.
	 */
	Object* source() const {
	    return source_;
	}

	/**
	 * \brief Gets the signal name.
	 * \return the name of the source signal.
	 */
	const std::string& signal_name() const {
	    return signal_name_;
	}

      gom_slots:

        /**
         * \brief Adds a condition on an argument.
         * \details An argument condition tests whether
         *  one of the signal's argument is equal to a
         *  specified value. The slot of the target is
         *  called only if all argument conditions are satisfied.
         * \param[in] name name of the argument
         * \param[in] value to be tested, it can be either the
         *   value to be tested for equality, or "==value", or "!=value".
         * \return a pointer to this Connection.
         */
        Connection* if_arg(
            const std::string& name, const std::string& value
        ) {
            if(conditions_.has_arg(name)) {
		Logger::err("GOM")
		    << "if_arg(): duplicate condition for " << name
		    << std::endl;
                return this;
            }
            conditions_.create_arg(name, value);
	    return this;
        }

        /**
         * \brief Adds an argument.
         * \details Each time the signal is triggered, all the arguments
         *  specified by this function are added to the ArgList before
         *  being sent to the target's slot.
         * \param[in] name name of the argument
         * \param[in] value value of the argument
	 * \return a pointer to this Connection.
         */
        Connection* add_arg(const std::string& name, const std::string& value) {
            if(args_.has_arg(name)) {
		Logger::err("GOM")
		    << "add_arg(): argument " << name
		    << " already exists"
		    << std::endl;
                return this;
            }
            args_.create_arg(name, value);
            return this;
        }

        /**
         * \brief Renames an argument.
         * \details Each time the signal is triggered, all the arguments
         *  specified by this function are renamed before being sent
         *  to the target's slot.
         * \param[in] name name of the argument
         * \param[in] new_name new name of the argument
         * \return a pointer to this Connection.
         */
        Connection* rename_arg(const std::string& name, const std::string& new_name) {
            if(rename_args_.has_arg(name)) {
		Logger::err("GOM")
		    << "rename_arg(): duplicate rename for " << name
		    << std::endl;
                return this;
            }
            rename_args_.create_arg(name, new_name);
            return this;
        }

        /**
         * \brief Discards an argument.
         * \details Each time the signal is triggered, all the arguments
         *  specified by this function are discarded before the ArgList
         *  is sent to the target's slot.
         * \param[in] name name of the argument to be discarded
         * \return a pointer to this Connection.
         */
        Connection* discard_arg(const std::string& name) {
            discard_args_.insert(name);
            return this;
        }

        /**
         * \brief Removes this connection from the slot it is
         *  connected to
         */
        void remove();

    protected:
        /**
         * \brief Tests whether an argument list satisfies the
         *  argument conditions.
         * \param[in] args the ArgList to be tested
         * \retval true if \p args satisfies all argument conditions
         * \retval false otherwise
         * \see add_arg_condition()
         */
        bool test_arg_conditions(const ArgList& args) const;

        /**
         * \brief Tests whether an argument satisfies a condition
         * \param[in] value name of the argument
         * \param[in] condition to be tested, it can be either the
         *   value to be tested for equality, or "==value", or "!=value".
         * \retval true if \p args satisfies the condition
         * \retval false otherwise
         * \see add_arg_condition()
         */
        bool test_arg_condition(
            const std::string& value, const std::string& condition
        ) const;

        /**
         * \brief Applies all the argument list transformations.
         * \param[in] args_in a const reference to the input ArgList
         * \param[out] args_out a reference to the transformed ArgList
         * \see add_arg(), discard_arg(), rename_arg()
         */
        void translate_args(const ArgList& args_in, ArgList& args_out);

    private:
        Object* source_;
        std::string signal_name_;
        ArgList conditions_;
        ArgList args_;
        ArgList rename_args_;
        std::set<std::string> discard_args_;
    };

    /**
     * \brief An automatic reference-counted pointer to a Connection.
     */
    typedef SmartPointer<Connection> Connection_var;

    /**********************************************************************/

    /**
     * \brief A Connection between a signal and a slot.
     */
    gom_class GOM_API SlotConnection : public Connection {
      public:
	/**
	 * \brief SlotConnection constructor.
         * \param[in] source a pointer to the source object
         * \param[in] sig_name the name of the source's signal
	 * \param[in] target a pointer to the target object
	 * \param[in] slot_name the name of the target's slot
	 * \details Target is not reference-counted.
	 */
	SlotConnection(
	    Object* source, const std::string& sig_name,
	    Object* target, const std::string& slot_name
	);

	/**
	 * \copydoc Connection::invoke_target()
	 */
	bool invoke_target(
	    const ArgList& args, Any& ret_val
	) override;

      private:
	Object* target_;
	std::string slot_name_;
    };

    /**********************************************************************/

    /**
     * \brief A Connection between a signal and an abstract Callable object.
     * \details It is not used for signal-slot connection because both
     *  CallableConnection and Request do reference counting on their arguments,
     *  thus this would prevent objects from being garbage-collected. For
     *  signal-slot connections, SlotConnection is used instead.
     */
    gom_class GOM_API CallableConnection : public Connection {
      public:
	/**
	 * \brief CallableConnection constructor.
         * \param[in] source a pointer to the source object
         * \param[in] sig_name the name of the source's signal
	 * \param[in] target a pointer to the target Callable object
	 * \details Target is reference-counted.
	 */
	CallableConnection(
	    Object* source, const std::string& sig_name, Callable* target
	);

	/**
	 * \copydoc Connection::invoke_target()
	 */
	bool invoke_target(
	    const ArgList& args, Any& ret_val
	) override;

      private:
	Callable_var target_;
    };


    /**********************************************************************/

    /**
     * \brief A list of connections.
     * \details Each slot of an object has an associated ConnectionList.
     */
    class ConnectionList : public std::vector<Connection_var> {
    public:
        /**
         * \brief Invokes all the connected slots.
         * \param[in] args a const reference to the arguments list.
         */
        void invoke(const ArgList& args) {
	    Any ret_val;
            for(unsigned int i=0; i<size(); i++) {
                (*this)[i]->invoke(args, ret_val);
            }
        }

        /**
         * \brief Removes a connection from a ConnectionList
         * \param[in] conn the connection to remove
         * \pre \p conn is in the connection list
         */
        void remove(Connection* conn) {
            std::vector<Connection_var>::iterator it = this->begin();
            while(it != this->end()) {
                if(it->get() == conn) {
                    erase(it);
                    return;
                }
                ++it;
            }
            geo_assert_not_reached;
        }
    };

    /******************************************************************/

}

#endif
