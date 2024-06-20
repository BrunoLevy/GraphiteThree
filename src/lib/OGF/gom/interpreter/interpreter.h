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

#ifndef H_OGF_GOM_INTERPRETER_H
#define H_OGF_GOM_INTERPRETER_H

#include <OGF/gom/common/common.h>
#include <OGF/gom/types/object.h>
#include <geogram/basic/numeric.h>
#include <string>
#include <vector>
#include <map>

/**
 * \file OGF/gom/interpreter/interpreter.h
 * \brief The abstract base class for the GOM interpreter.
 */

namespace OGF {

    class Interpreter;
    class Object;
    class MetaClass;
    class MetaMethod;
    class ArgList;
    class Request;
    class Callable;
    class Connection;
    
    /*************************************************************************/ 

    /**
     * \brief A naming scope in an Interpreter.
     */
    gom_attribute(abstract,"true")
    gom_class GOM_API Scope : public Object {
      public:
	/**
	 * \brief Scope constructor.
	 * \param[in] object a pointer to the object where this
	 *  Scope accesses variables.
	 */
        Scope(Object* object);

	/**
	 * \brief Scope destructor.
	 */
	~Scope() override;

	/**
	 * \brief Finds a variable by id.
	 * \param[in] name the name, can be either a previously bound id
	 * \return an Any with the variable, or a null Any if name is
	 *  not bound to a variable.
	 */
	virtual Any resolve(const std::string& name) = 0;

	/**
	 * \brief Lists all the variable names available in this scope.
	 * \param[out] names the variable names.
	 */
	virtual void list_names(std::vector<std::string>& names) const;

        /**
         * \copydoc Object::search()
         */
        void search(const std::string& needle, const std::string& path="") override;
        
      protected:
	Object* object_;
    };
    
    typedef SmartPointer<Scope> Scope_var;
    
    /*************************************************************************/

    /**
     * \brief The Scope that contains all global variables of an Interpreter.
     */
    gom_class GOM_API GlobalScope : public Scope {
      public:
	/**
	 * \brief GlobalScope constructor.
	 * \param[in] interpreter the Interpreter this GlobalScope is associated
	 *  with.
	 */
	GlobalScope(Interpreter* interpreter);

	/**
	 * \brief GlobalScope destructor.
	 */
	~GlobalScope() override;

	/**
	 * \copydoc Scope::resolve()
	 */
	Any resolve(const std::string& name) override;

	/**
	 * \copydoc Scope::list_names()
	 */
	void list_names(std::vector<std::string>& names) const override;
    };
    
    /************************************************************************/ 
    
    /**
     * \brief The Scope that contains all interfaces of an object.
     * \details Encapsulates query_interface() in such a way that 
     *  automatic completion works when using object.I.
     */
    gom_class GOM_API InterfaceScope : public Scope {
      public:
	/**
	 * \brief InterfaceScope constructor.
	 * \param[in] object a pointer to the Object where this
	 *   scope accesses variables.
	 */
	InterfaceScope(Object* object);

	/**
	 * \brief InterfaceScope destructor.
	 */
	~InterfaceScope() override;

	/**
	 * \copydoc Scope::resolve()
	 */
	Any resolve(const std::string& name) override;

	/**
	 * \copydoc Scope::list_names()
	 */
	void list_names(std::vector<std::string>& names) const override;
    };

    /*************************************************************************/ 

    /**
     * \brief The Scope that contains all interfaces of an object.
     * \details Encapsulates query_interface() in such a way that 
     *  automatic completion works when using object.I.
     */
    gom_class GOM_API MetaTypesScope : public Scope {
      public:
	/**
	 * \brief MetaTypesScope constructor.
	 * \param[in] prefix optional prefix for object names, 
         *   for instance "OGF::" or "std::"
	 */
	MetaTypesScope(const std::string& prefix = "");

	/**
	 * \brief MetaTypesScope destructor.
	 */
	~MetaTypesScope() override;

	/**
	 * \copydoc Scope::resolve()
	 */
	Any resolve(const std::string& name) override;

	/**
	 * \copydoc Scope::list_names()
	 */
	void list_names(std::vector<std::string>& names) const override;

    gom_slots:
        MetaTypesScope* create_subscope(const std::string& name);
        
    private:
        std::string prefix_;
        std::map<std::string, Scope_var> subscopes_;
    };


    /*************************************************************************/ 

    
    /**
     * \brief Abstract base class for the GOM interpreter.
     * \details It is completely independent of the used
     *  interpreted language. It is implemented in the gel_lua
     *  (and upcoming gel_python libraries).
     */
    gom_attribute(abstract,"true")         
    gom_class GOM_API Interpreter : public Object {

    public:

	/**
	 * \brief Interpreter constructor.
	 */
	Interpreter();

	/**
	 * \brief Clears all variables, restarts from initial state.
	 */
	virtual void reset()=0;
	
	/**
	 * \brief Finds a variable by id.
	 * \param[in] id the id, can be either a previously bound id
	 *  defined in the interpreter, or an id of the form 
	 *  "@ClassName::#instance", where ClassName is the name of a 
	 *  class registered to GOM, and instance an integer that refers 
	 *  to a specific instance of that
	 *  class (the latter form of ids is used by the VCR system for 
	 *  recording events received by the GUI widgets).
	 * \param[in] quiet if true, do not display error messages.
	 * \return an any with the variable attached to \p id in
	 *  the current Scope, or nil if there is no such a
	 *  variable.
	 */
	virtual Any resolve(const std::string& id, bool quiet=true) const;

	/**
	 * \brief Binds a value to a variable.
	 * \details The variable is then visible from the interpreter in the
	 *  global scope.
	 * \param[in] id the name of the variable.
	 * \param[in] value the value of the variable to be bound.
	 */
	virtual void bind(const std::string& id, const Any& value) = 0;

	/**
	 * \brief Lists the global variable names in this Interpreter..
	 * \param[out] names a vector of strings with the names 
	 *  of the global variables.
	 */
	virtual void list_names(std::vector<std::string>& names) const;
	
	/**
	 * \brief Evaluates a string with an expression.
	 * \param[in] expression a string with an expression.
	 * \param[in] quiet if true, do not display error messages.
	 * \return an Any with the computed value.
	 */
	virtual Any eval(
	    const std::string& expression, bool quiet=true
	) const;

	
    gom_slots:
	/**
	 * \brief Outputs to the logger the methods, 
	 *  slots, properties, property values
	 *  of a given object.
	 * \param[in] object a pointer to the object.
	 */
	void inspect(Object* object);

	/**
	 * \brief Outputs to the logger the methods, 
	 *  slots, properties of a given MetaClass.
	 * \param[in] meta_type a pointer to the MetaType.
	 */
	void inspect_meta_type(MetaType* meta_type);

	/**
	 * \brief Outputs to the logger the names of all
	 *  classes registered to GOM.
	 */
	void list_classes();

	/**
	 * \brief Binds an object to a variable.
	 * \details The object is then visible from the interpreter in the
	 *  global scope.
	 * \param[in] id the name of the variable.
	 * \param[in] object the object to be bound
	 */
	void bind_object(const std::string& id, Object* object);
	
	/**
	 * \brief Finds an objet by id.
	 * \param[in] id the id, can be either a previously bound id
	 *  defined in the interpreter, or an id of the form 
	 *  "@ClassName::#instance", where ClassName is the name of a 
	 *  class registered to GOM, and instance an integer that refers 
	 *  to a specific instance of that class.
	 * \param[in] quiet if true, do not display error messages.
	 * \return a pointer to the Object attached to \p id in
	 *  the current Scope, or nil if there is no such a
	 *  variable.
	 */
	Object* resolve_object(const std::string& id, bool quiet=true) const;


	/**
	 * \brief Finds an objet by global id.
	 * \param[in] id the id in the form "@ClassName::#instance",
	 *  where ClassName is the name of a class registered to GOM, and
	 *  instance an integer that refers to a specific instance of that
	 *  class.
	 * \param[in] quiet if true, do not display error messages.
	 * \return a pointer to the Object attached to \p id in
	 *  the current Scope, or nil if there is no such a
	 *  variable.
	 */
	Object* resolve_object_by_global_id(
	    const std::string& id, bool quiet=true
	) const;
	
	/**
	 * \brief Evaluates a string with an expression.
	 * \param[in] expression a string with an expression.
	 * \param[in] quiet if true, do not display error messages.
	 * \return a pointer to the object with the result of the
	 *  expression, or nil if the result is not an object.
	 */
	Object* eval_object(
	    const std::string& expression, bool quiet=true
	) const;

	/**
	 * \brief Evaluates a string with an expression.
	 * \param[in] expression a string with an expression.
	 * \param[in] quiet if true, do not display error messages.
	 * \return the result of the expression converted to a string.
	 */
	std::string eval_string(
	    const std::string& expression, bool quiet=true
	) const;

	/**
	 * \brief Finds a MetaType by name.
	 * \param[in] type_name the name of the MetaType 
	 * (with 'OGF::' scope if need be).
	 * \return a pointer to the MetaType or nil if there where 
	 *  no such MetaType.
	 */
	virtual MetaType* resolve_meta_type(const std::string& type_name) const;

        /**
         * \brief Binds a MetaType
         * \param[in] mtype the MetaType to be bound
         * \retval true if the MetaType could be bound
         * \retval false otherwise. This can happen if a MetaType with the same
         *   name was already bound
         */
        virtual bool bind_meta_type(MetaType* mtype);

	/**
	 * \brief Creates an object.
	 * \param[in] args the ArgList, with at least an argument classname. 
	 * The other arguments
	 *  are passed to the constructor.
	 * \return a pointer to the created object or nil if an error 
	 *  was encountered.
	 */
	virtual Object* create(const ArgList& args);

        /**
         * \brief Gets the value of a Geogram environment value.
         * \param[in] name name of the variable
         * \return the value of the variable
         * \see GEO::Environment
         */
        virtual std::string get_environment_value(const std::string& name);

        /**
         * \brief Sets the value of an environment variable.
         * \param[in] name name of the variable
         * \param[in] value new value of the environment variable
         */
	virtual void set_environment_value(
	    const std::string& name, const std::string& value
	);

        /**
         * \brief Executes a single line of code in the interpreted language.
         * \param[in] command the line of code
         * \param[in] save_in_history true if the command should be saved
         *  in the history
         * \param[in] log true if the command should be displayed to the
         *  logging console
         * \retval true if the command could be sucessfully executed
         * \retval false otherwise
         */
        virtual bool execute(
            const std::string& command, 
            bool save_in_history = true,
            bool log = true
        ) = 0;

        /**
         * \brief Executes commands from a given file.
         * \param[in] file_name name of the file that contains the
         *  commands to be executed
         * \retval true if the file could be sucessfully executed
         * \retval false otherwise
         */
        virtual bool execute_file(const std::string& file_name);

	/**
	 * \brief Displays a message in the terminal or console.
	 * \param[in] message the message to be displayed.
	 * \param[in] tag an optional tag.
	 */
	virtual void out(
	    const std::string& message, const std::string& tag = "GOM"
	);

	/**
	 * \brief Displays an error message in the terminal or console.
	 * \param[in] message the message to be displayed.
	 * \param[in] tag an optional tag.
	 */
	virtual void err(
	    const std::string& message, const std::string& tag = "GOM"
	);

	/**
	 * \brief Displays a warning message in the terminal or console.
	 * \param[in] message the message to be displayed.
	 * \param[in] tag an optional tag.
	 */
	virtual void warn(
	    const std::string& message, const std::string& tag = "GOM"
	);
	
	/**
	 * \brief Displays a status message.
	 * \param[in] message the message.
	 */
	virtual void status(const std::string& message);

	/**
	 * \brief Adds a path where dynamic libraries can be loaded.
	 * \details Under Windows, adds the path to the PATH environment
	 *  variable. Under other OSes, does nothing.
	 * \param[in] path the path to be added.
	 */
	virtual void append_dynamic_libraries_path(const std::string& path);
	
	/**
	 * \brief Loads a plug-in.
	 * \param[in] module_name the name of the plug-in.
	 * \retval true if the module was successfully loaded.
	 * \retval false otherwise.
	 */
	virtual bool load_module(const std::string& module_name);

	/**
	 * \brief Connects a signal to a callable.
	 * \param[in] from a pointer to a Request with a reference to the source
	 *  object and the signal.
	 * \param[out] to a pointer to a Callable.
	 * \return a pointer to the new connection, or nil if the connection
	 *  could not be created.
	 */
	virtual Connection* connect(Request* from, Callable* to);


	/**
	 * \brief Gets an interpreter for a given language.
	 * \param[in] language "Lua" for lua, "Python" for python...
	 */
	Interpreter* interpreter(const std::string& language) {
	    return instance_by_language(language);
	}

	/**
	 * \brief Gets an interpreter for a given language by file extension.
	 * \param[in] extension "lua" for lua, "py" for python...
	 */
	Interpreter* interpreter_by_file_extension(
	    const std::string& extension
	) {
	    return instance_by_file_extension(extension);
	}

        /**
         * \copydoc Object::search()
         */
        void search(const std::string& needle, const std::string& path="");
        
      public:
        /**
         * \brief Gets the default interpreter.
         * \return a pointer to the first created interpreter.
         */
        static Interpreter* default_interpreter() {
            return default_interpreter_;
        }
	
      gom_properties:
	/**
	 * \brief Gets the history.
	 * \return a string with the history.
	 */
	virtual std::string get_history() const;

	/**
	 * \brief Gets the name of the interpreted language.
	 * \return The name of the interpreted language ("Lua" for
	 *  Lua, "Python" for Python ...).
	 */
	const std::string& get_language() const {
	    return language_;
	}

	/**
	 * \brief Gets the filename extensions for the interpreted language.
	 * \return the extension without the dot ("lua" for Lua, 
	 *  "py" for Python).
	 */
	const std::string& get_filename_extension() const {
	    return extension_;
	}

	/**
	 * \brief Gets the Scope with the global variables.
	 * \return a pointer to the Scope.
	 */
	Scope* get_globals() const {
	    return globals_;
	}

        /**
         * \brief Gets the Scope with the meta types.
         * \return a pointer to the Scope
         */
        Scope* get_meta_types() const {
            return meta_types_;
        }
        
      public:
	
        /**
         * \brief Gets the instance of the interpreter that interprets
	 *  a given language.
	 * \param[in] language the interpreted language.
         * \return a pointer to the instance of the interpreter.
         */
        static Interpreter* instance_by_language(const std::string& language);

        /**
         * \brief Gets the instance of the interpreter by file extension.
	 * \param[in] extension file extension without the dot.
         * \return a pointer to the instance of the interpreter.
         */
        static Interpreter* instance_by_file_extension(
	    const std::string& extension
	);
	
        /**
         * \brief Interpreter destructor.
         */
        virtual ~Interpreter();
        
        /**
         * \brief Initializes the interpreter subsystem, and 
         *  defines the interpreter to be used.
         * \details This function is called at Graphite startup by
         *  the library that creates the interpreter (gel_python2 or
         *  gel_python3). Client code is not supposed to call this
         *  function directly.
         * \param[in] instance the interpreter to be used.
	 * \param[in] language the name of the interpreted language.
	 * \param[in] extension file extension for the interpreted language, 
	 *  without the point.
         */
        static void initialize(
	    Interpreter* instance, const std::string& language,
	    const std::string& extension
	);


	/**
	 * \brief Terminates an interpreter and removes a given language 
	 *  from the list of interpreters.
	 * \param[in] language the name of the language to be terminated.
	 * \param[in] extension file extension for the interpreted language, 
	 *  without the point.
	 */
	static void terminate(
	    const std::string& language, const std::string& extension
	);
	
        /**
         * \brief Terminates the interpreter subsystem, and deallocates
         *  the interpreter.
         * \details This function is called at Graphite shutdown.
         *  Client code is not supposed to call this function directly.
         */
        static void terminate();

	/**
	 * \brief Creates an object from a classname and arguments list.
	 * \param[in] classname the classname, with the 'OGF::' scope.
	 * \param[in] args the arguments list.
	 * \return a pointer to the created object or nil on error.
	 */
	Object* create(const std::string& classname, const ArgList& args);

	/**
	 * \brief Gets the possible automatic completions from a partial 
	 *  command entered by the user in the command line.
	 * \param[in] line the command line to be completed.
	 * \param[in] startw , endw the position of the word to be completed.
	 * \param[in] cmpword the word to be completed
	 * \param[out] matches the possible completions.
	 */
	virtual void automatic_completion(
	    const std::string& line, index_t startw, index_t endw,
	    const std::string& cmpword, std::vector<std::string>& matches
	);
	
        
        /**
         * \brief Saves the history to a file.
         * \param[in] file_name name of the file where the history should
         *  be saved.
         */
        virtual void save_history(const std::string& file_name) const;

        /**
         * \brief Clears the history.
         */
        virtual void clear_history();

        /**
         * \brief Adds a command line to the history.
         * \param[in] command the command line to be added to the history
         */
        void add_to_history(const std::string& command);

        /**
         * \brief Gets the size of the history.
         * \return the number of command lines in the history.
         */
        size_t history_size() const {
            return history_.size();
        }

        /**
         * \brief Gets one of the commands in the history by line index.
         * \param[in] l the index of the line in the history
         * \return the l th command in the history, as a string
         */
        std::string history_line(unsigned int l) const {
            return l < history_size() ? history_[l] : std::string("");
        }

      protected:
	/**
	 * \brief Gets all possible filenames starting from a certain
	 *  prefix.
	 * \param[in] prefix either en empty string, or an escaped
	 *  string with the beginning of a filename.
	 * \param[out] completions all the filenames that match the
	 *  input prefix.
	 */
	void filename_completion(
	    const std::string& prefix,
	    std::vector<std::string>& completions
	);

	/**
	 * \brief Keeps in a list of completion only those that start with
	 *  a given prefix.
	 * \param[in] prefix the prefix
	 * \param[in,out] completions the vector of strings to be filtered. On
	 *  exit, only those that start with \p prefix are kept.
	 */
	void filter_completion_candidates(
	    const std::string& prefix, std::vector<std::string>& completions
	);
	
	/**
	 * \brief Gets all possible keys in a certain context.
	 * \param[in] context the name of a variable.
	 * \param[out] keys the name of all the data members and function 
	 *  members of the variable.
	 */
	virtual void get_keys(
	    const std::string& context, std::vector<std::string>& keys
	);

	/**
	 * \brief Gets all possible keys in a certain context.
	 * \param[in] context the name of a variable.
	 * \param[out] keys the name of all the data members and function 
	 *  members of the variable.
	 */
	virtual void get_keys(
	    Any& context, std::vector<std::string>& keys
	);

	/**
	 * \brief Displays the prototype of a given method of an object.
	 * \details The prototype is displayed by the Logger.
	 * \param[in] object the object
	 * \param[in] mmethod the method
	 */
	void inspect_method(Object* object, MetaMethod* mmethod);

	/**
	 * \brief Displays a meta class.
	 * \details The meta class is displayed by the Logger.
	 * \param[in] object the object.
	 * \param[in] mclass the meta class.
	 */
	void inspect_meta_class(Object* object, MetaClass* mclass);
	
	/**
	 * \brief Outputs to the logger the signature of a method.
	 * \param[in] mmethod a pointer to the MetaMethod.
	 */
	void inspect_meta_method(MetaMethod* mmethod);

	/**
	 * \brief Sets the name of the interpreted language.
	 * \param[in] language the name of the interpreted 
	 *  language.
	 */
	void set_language(const std::string& language) {
	    language_ = language;
	}

	/**
	 * \brief Sets the filename extensions for the interpreted language.
	 * \param[in] extension the extension.
	 */
	void set_filename_extension(const std::string& extension) {
	    extension_ = extension;
	}

	/**
	 * \brief Transforms a string into a string constant in the interpreted
	 *  language.
	 * \details Default implementation adds single quotes.
	 * \param[in] str the string to be transformed.
	 * \return the string with quotes.
	 */
	virtual std::string stringify(const std::string& str) const;

	/**
	 * \brief Transforms a list of name-value pairs arguments.
	 * \details Some interpreters (e.g., Lua) need special syntax for
	 *  name-value pair calls. Default implementation returns the input
	 *  string.
	 * \param[in] args a set of comma-separated name-value pairs.
	 * \return the input args in a form that can be understood as name-
	 *  value pairs by the target language.
	 */
	virtual std::string name_value_pair_call(const std::string& args) const;
	
    protected:
        std::vector<std::string> history_;
	std::string language_;
	std::string extension_;
	
    private:
        static std::map<
	    std::string, SmartPointer<Interpreter>
	> instance_;
	static std::map<std::string, Interpreter*>
	    instance_by_file_extension_;
        static Interpreter* default_interpreter_;
	
	Scope_var globals_;
        Scope_var meta_types_;
    };

    typedef SmartPointer<Interpreter> Interpreter_var;
    
    /*************************************************************************/

} 

#endif 
