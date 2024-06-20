/*
 *  Graphite: Geometry and Graphics Programming Library + Utilities
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

#ifndef H_OGF_GOM_TYPES_CALLABLE_H
#define H_OGF_GOM_TYPES_CALLABLE_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/reflection/meta_class.h>

namespace OGF {

    /**
     * \brief A Callable object.
     * \details Callable objects have an invoke() member function.
     */
    gom_attribute(abstract,"true")     
    gom_class GOM_API Callable : public Object {
      public:

	/**
	 * \brief Callable constructor.
	 */
	Callable();
	
	/**
	 * \brief Callable destructor.
	 */
	~Callable() override;
	
        /**
         * \brief Invokes a method with an argument list,
         *  and gets the return value.
         * \param[in] args a const reference to the ArgList
         * \param[out] ret_val the return value as an Any
         * \retval true if the method could be sucessfully invoked
         * \retval false otherwise
         */
	virtual bool invoke(const ArgList& args, Any& ret_val) = 0;

        /**
         * \brief Invokes a method by method name and argument list.
         * \details This variant of invoke() ignores the return value.
         * \param[in] args a const reference to the ArgList
         * \retval true if the method could be sucessfully invoked
         * \retval false otherwise
         */
        bool invoke(const ArgList& args) {
            Any ret_val;
            return invoke(args, ret_val);
        }

        /**
         * \brief Invokes a method by method name.
         * \details This variant of invoke() is for methods with no
         *  argument and void return type.
         * \retval true if the method could be sucessfully invoked
         * \retval false otherwise
         */
        bool invoke() {
            ArgList args;
            Any ret_val;
            return invoke(args, ret_val);
        }
    };

    /**
     * \brief A reference-counted pointer to a Callable.
     */
    typedef SmartPointer<Callable> Callable_var;
    
    /**********************************************************************/
    
    /**
     * \brief A pointer to an object and to a meta-method
     *  of that object (a very limited notion of "closure").
     */
    gom_class GOM_API Request : public Callable {
      public:
	/**
	 * \brief Request constructor.
	 * \param[in] o a pointer to the target object.
	 * \param[in] m a pointer to the target method.
	 * \param[in] managed if true, then reference
	 *  count is increased/decreased for object.
	 */
         Request(
	     Object* o, MetaMethod* m, bool managed = true
	 );

	 /**
	  * \brief Request destructor.
	  */
	  ~Request() override;

	 /**
	  * \copydoc Callable::invoke()
	  */
	  bool invoke(const ArgList& args, Any& ret_val) override;


	 /**
	  * \copydoc Object::get_doc()
	  */
          std::string get_doc() const override;
          
      gom_slots:
	  
	 /**
	  * \brief Gets the target object.
	  * \return a pointer to the target Object.
	  */
	 Object* object() const {
	     return object_;
	 }

	 /**
	  * \brief Gets the method to be invoked.
	  * \return a pointer to the MetaMethod that
	  *  corresponds to the method to be invoked.
	  */
	 MetaMethod* method() const {
	     return method_;
	 }

      private:
	 /**
	  * \brief Forbids copy.
	  */
	 Request(const Request& rhs);
	 
	 /**
	  * \brief Forbids copy.
	  */
	 Request& operator=(const Request& rhs);
	 
      private:
 	 Object* object_;
	 MetaMethod* method_;
	 bool managed_;
    };


    /**
     * \brief A reference-counted pointer to a Request.
     */
    typedef SmartPointer<Request> Request_var;
    
    /**********************************************************************/

}

#endif
