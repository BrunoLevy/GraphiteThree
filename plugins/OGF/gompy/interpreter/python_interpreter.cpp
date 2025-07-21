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


#include <OGF/gompy/interpreter/python_interpreter.h>
#include <OGF/gompy/interpreter/python.h>
#include <OGF/gompy/interpreter/py_graphite_object.h>
#include <OGF/gompy/interpreter/py_graphite_iterator.h>
#include <OGF/gompy/interpreter/py_graphite_metaclass.h>
#include <OGF/gompy/interpreter/py_graphite_callable.h>
#include <OGF/gompy/interpreter/gom_python_callable.h>
#include <OGF/gompy/interpreter/interop.h>
#include <OGF/gompy/interpreter/nl_vector_interop.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/gom/lua/lua_interpreter.h>
#include <OGF/basic/os/file_manager.h>
#include <OGF/basic/modules/modmgr.h>
#include <OGF/scene_graph/NL/vector.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/command_line.h>

/*************************************************************************/

namespace {
    using namespace OGF;
    using namespace OGF::GOMPY;

    /*****************************************************************/

    PyObject* graphite_interpreter(PyObject* self_in, PyObject* args_in) {
	geo_argused(self_in);
	geo_argused(args_in);
	Interpreter* interpreter = Interpreter::instance_by_language("Python");
	PyObject* result = PyGraphiteObject_New(interpreter);
	Py_INCREF(result);
	return result;
    }

    PyObject* graphite_module_get_attro(PyObject* self, PyObject* name_in) {
	geo_argused(self);
	// - We need this getattro override that returns something
	// when "__path__" is looked up, else Python runtime does not
	// see our module as a package (from which gompy.xxx can be imported).
	// - import gompy.xxx as yyy does not invoke get_attro(), it uses
	// instead the system-wide module dictionary, populated with what we
	// want to be able to import in PythonInterpreter::PythonInterpreter().
	// (but it needs to be also handled here)
	std::string name = python_to_string(name_in);
	if(name == "__path__") {
	    return string_to_python("");
	}
	if(name == "gom") {
	    PyObject* py_gom = PyGraphiteObject_New(
		Interpreter::instance_by_language("Python")
	    );
	    Py_INCREF(py_gom);
	    return py_gom;
	}
	if(name == "types") {
	    PyObject* py_meta_types = PyGraphiteObject_New(
		Interpreter::instance_by_language("Python")->get_meta_types()
	    );
	    Py_INCREF(py_meta_types);
	    return py_meta_types;
	}
	// Positioning this exception indicates that attribute lookup fallsback
	// to default behavior.
	PyErr_SetObject(PyExc_AttributeError, name_in);
	return nullptr;
    }

    PyMethodDef graphite_module_methods[] = {
	{
	    "interpreter",
	    graphite_interpreter,
	    METH_NOARGS,
	    "gets the interpreter"
	},
	{
	    "__getattr__",
	    graphite_module_get_attro,
	    METH_O,
	    "gets an attribute from the interpreter"
	},
        {
            nullptr, /* ml_name */
            nullptr, /* ml_meth */
            0,       /* ml_flags */
            nullptr  /* ml_doc */
        }
    };

    static struct PyModuleDef graphite_moduledef = {
        PyModuleDef_HEAD_INIT,
        "gompy",                 /* m_name */
        "Graphite Object Model", /* m_doc */
        -1,                      /* m_size */
        graphite_module_methods, /* m_methods */
        nullptr,                 /* m_reload */
        nullptr,                 /* m_traverse */
        nullptr,                 /* m_clear */
        nullptr                  /* m_free */
    };

    PyMODINIT_FUNC PyInit_gom(void);
    PyMODINIT_FUNC PyInit_gom() {
	init_graphite_ObjectType();
	init_graphite_IteratorType();
	init_graphite_CallableType();
	init_graphite_MetaClassType();
        PyObject* m = PyModule_Create(&graphite_moduledef);
        if(m == nullptr) {
	    Py_INCREF(Py_None);
            return Py_None;
        }
        if (PyType_Ready(&graphite_ObjectType) < 0) {
	    Py_INCREF(Py_None);
            return Py_None;
        }
        if (PyType_Ready(&graphite_IteratorType) < 0) {
	    Py_INCREF(Py_None);
            return Py_None;
        }
        if (PyType_Ready(&graphite_CallableType) < 0) {
	    Py_INCREF(Py_None);
            return Py_None;
        }
        if (PyType_Ready(&graphite_MetaClassType) < 0) {
	    Py_INCREF(Py_None);
            return Py_None;
        }
        Py_INCREF(&graphite_ObjectType);
        PyModule_AddObject(m, "Object",  (PyObject *)&graphite_ObjectType);
        return m;
    }

}

   /*****************************************************************/

namespace OGF {

    PythonInterpreter::PythonInterpreter() : main_module_(nullptr) {
	// Tests whether Python is running in Graphite (true) or not (false)
	use_embedded_interpreter_ = (Py_IsInitialized() == 0);

	if(!use_embedded_interpreter_) {
	    // Shortcuts that one can import ... as ...
	    // gompy.interpreter()                -> gompy.gom
	    // gompy.interpreter().meta_types     -> gompy.types
	    // gompy.interpreter().meta_types.OGF -> gompy.types.OGF
	    // (they need also to be handled by module's __getattr__)

	    PyObject *sys_modules = PySys_GetObject("modules");
	    Scope* meta_types = get_meta_types();
	    Scope* OGF = nullptr;
	    meta_types->resolve("OGF").get_value(OGF);
	    geo_assert(OGF != nullptr);

	    PyObject* py_gom = PyGraphiteObject_New(this);
	    PyObject* py_meta_types = PyGraphiteObject_New(meta_types);
	    PyObject* py_OGF = PyGraphiteObject_New(OGF);

	    // Note: PyDict_SetItemString() increases refcount
	    PyDict_SetItemString(sys_modules, "gompy.gom", py_gom);
	    PyDict_SetItemString(sys_modules, "gompy.types", py_meta_types);
	    PyDict_SetItemString(sys_modules, "gompy.types.OGF", py_OGF);
	    return;
	}

	// All the rest is for when we are in Graphite

	bool FPE_bkp = Process::FPE_enabled();
	Process::enable_FPE(false);

	PyConfig config;
	PyConfig_InitPythonConfig(&config);


	// If there are Python subdirectories in OGF_PATH, add them
	// to Python path.
	const std::vector<std::string>& ogf_path =
	    FileManager::instance()->ogf_path();
	if(
	    ogf_path.size() > 0 &&
	    FileSystem::is_directory(ogf_path[0] + "/lib/Python")
	) {
	    Logger::out("GOMpy")
		<< "Found local python lib. directory in Graphite: "
		<< ogf_path[0] + "/lib/Python"
		<< std::endl;
	    Logger::out("GOMpy")
		<< " -> Setting python path there." << std::endl;
	    std::string python_path;
	    for(index_t i=0; i<ogf_path.size(); ++i) {
		if(python_path.length() != 0) {
#ifdef GEO_OS_WINDOWS
		    python_path += ';';
#else
		    python_path += ':';
#endif
		}
		python_path += (ogf_path[i] + "/lib/Python");
	    }
	    config.pythonpath_env =
		Py_DecodeLocale(python_path.c_str(), nullptr);
	}


	// Needed since our module is not in a separate shared object.
	PyImport_AppendInittab("gompy", PyInit_gom);
	Py_InitializeFromConfig(&config);

	// PyMem_RawFree(config.pythonpath_env);
	// TODO: do we need to do that ?

	// What follows is the low-level equivalent to:
	//PyRun_SimpleString("import gompy");
	PyObject* gom_module = PyImport_ImportModule("gompy");
	main_module_ = PyImport_AddModule("__main__");
	PyObject_SetAttrString(main_module_, "gompy", gom_module);
	Py_XDECREF(gom_module);
	PyObject* gom = PyGraphiteObject_New(this, false);
	Py_INCREF(gom);
	PyObject_SetAttrString(main_module_, "gom", gom);

	//   If Python interpreter is embedded in Graphite,
	// redirect output and error to Graphite console.
	PyRun_SimpleString(
	    "class OutGraphiteStream:                   \n"
	    "  def __init__(self):                      \n"
	    "     pass                                  \n"
	    "  def write(self, string):                 \n"
	    "     if string != \'\\n\':                 \n"
	    "        gom.out(string)                    \n"
	    "  def flush(self):                         \n"
	    "     pass                                  \n"
	    "class ErrGraphiteStream:                   \n"
	    "  def __init__(self):                      \n"
	    "     self.buffer = str()                   \n"
	    "  def write(self, string):                 \n"
	    "     for c in string:                      \n"
	    "        self.putc(c)                       \n"
	    "  def putc(self,c):                        \n"
	    "     if c == '\\\n':                       \n"
	    "        self.flush()                       \n"
	    "     else:                                 \n"
	    "        self.buffer += c                   \n"
	    "  def flush(self):                         \n"
	    "     gom.err(self.buffer)                  \n"
	    "     self.buffer = str()                   \n"
	    "                                           \n"
	    "import sys                                 \n"
	    "sys.stdout = OutGraphiteStream()           \n"
	    "sys.stderr = ErrGraphiteStream()           \n"
	    "sys.displayhook = gom.out                  \n"
	);

	Process::enable_FPE(FPE_bkp);
    }

    PythonInterpreter::~PythonInterpreter() {
	if(use_embedded_interpreter_) {
	    // Py_Finalize(); // <- crashes when jax is imported
	}
	main_module_ = nullptr;
    }

    void PythonInterpreter::reset() {
	if(use_embedded_interpreter_) {
	    Py_Finalize();
	}
	main_module_ = nullptr;
	// TODO: restart
    }

    bool PythonInterpreter::execute(
	const std::string& command, bool save_in_history, bool log
    ) {
        if(log) {
            Logger::out("GOMpy") << command << std::endl;
        }

	bool FPE_bkp = Process::FPE_enabled();
	Process::enable_FPE(false);
        int res = PyRun_SimpleString(const_cast<char*>(command.c_str()));
	Process::enable_FPE(FPE_bkp);

        if(res == -1) {
            return false;
        }
        if(save_in_history) {
            add_to_history(command);
        }

        return true;
    }

    bool PythonInterpreter::execute_file(const std::string& file_name) {

        Environment::instance()->set_value("current_gel_file", file_name);

// We got some problems under Windows, so we use this quick and dirty
// workaround...
#ifdef GEO_OS_WINDOWS
        std::string gel_file(file_name);
        if(!FileManager::instance()->find_file(gel_file)) {
            Logger::err("GOMpy") << "Cannot find file \'"
                               << gel_file << "\'" << std::endl;
            return false;
        }
        std::ifstream in(gel_file.c_str());
        std::stringstream file_buff;
        if(!in) {
            Logger::err("GOMpy") << "cannot open file:" << gel_file
                               << std::endl;
            return false;
        }
        while(in) {
	    std::string buff;
	    std::getline(in,buff);
            file_buff << buff << std::endl;
        }

	bool FPE_bkp = Process::FPE_enabled();
	Process::enable_FPE(false);
        int res = PyRun_SimpleString(file_buff.str().c_str());
	Process::enable_FPE(FPE_bkp);

        if(res == -1){
            return false;
        }
#else
        std::string gel_file(file_name);
        if(!FileManager::instance()->find_file(gel_file)) {
            Logger::err("GOMpy") << "Cannot find file \'"
                               << gel_file << "\'" << std::endl;
            return false;
        }
        FILE* f = fopen(gel_file.c_str(), "rt");
        if(f == nullptr) {
            Logger::err("GOMpy") << "Cannot open file:" << gel_file
                                       << std::endl;
            return false;
        }

	bool FPE_bkp = Process::FPE_enabled();
	Process::enable_FPE(false);
        int res = PyRun_SimpleFile(f, const_cast<char*>(gel_file.c_str()));
	Process::enable_FPE(FPE_bkp);

        fclose(f);
        if(res == -1){
            return false;
        }
#endif
        return true;
    }

    void PythonInterpreter::bind(const std::string& id, const Any& value) {
	PyObject* obj = graphite_to_python(value);
	Py_INCREF(obj);
        PyObject_SetAttrString(main_module_, id.c_str(), obj);
    }

    Any PythonInterpreter::resolve(const std::string& id, bool quiet) const {
	Any any_result;
	PyObject* result =
	    PyObject_GetAttrString(main_module_, id.c_str());
	PyErr_Clear();
	if(result == nullptr) {
	    if(!quiet) {
		Logger::err("GOMpy") << id << ":no such global object"
				     << std::endl;
	    }
	    return any_result;
	}
	any_result = python_to_graphite(result);
	return any_result;
    }

    Any PythonInterpreter::eval(
	const std::string& expression, bool quiet
    ) const {
	// return resolve(expression, quiet);
	Any any_result;
	PyCodeObject* code = (PyCodeObject*) Py_CompileString(
	    expression.c_str(), "immediate", Py_eval_input
	);
	PyErr_Clear();
	if(code == nullptr) {
	    if(!quiet) {
		Logger::err("GOMpy") << expression << ":could not interpret"
				     << std::endl;
	    }
	    return any_result;
	}

	PyObject* global_dict = PyModule_GetDict(main_module_);
	Py_INCREF(global_dict);
	PyObject* local_dict = PyDict_New();
	Py_INCREF(local_dict);
	bool FPE_bkp = Process::FPE_enabled();
	Process::enable_FPE(false);
	PyObject* result = PyEval_EvalCode(
	    (PyObject*)code, global_dict, local_dict
	);
	Process::enable_FPE(FPE_bkp);
	Py_XINCREF(result);
	any_result = python_to_graphite(result);
	Py_XDECREF(result);
	Py_DECREF(local_dict);
	Py_DECREF(global_dict);

	return any_result;
    }

    void PythonInterpreter::display_error_message(const std::string& msg) {
	Logger::err("Python") << msg << std::endl;
    }

    void PythonInterpreter::list_names(std::vector<std::string>& names) const {
	names.clear();
	PyObject* globals = PyModule_GetDict(main_module_);
	Py_ssize_t nb = PyDict_Size(globals);
	PyObject* keys = PyDict_Keys(globals);
	Py_INCREF(keys);
	for(int i=0; i<nb; ++i) {
	    names.push_back(python_to_string(PyList_GetItem(keys,i)));
	}
	Py_DECREF(keys);
    }

    /*****************************************************************/

    void PythonInterpreter::get_keys(
	const std::string& context, std::vector<std::string>& keys
    ) {
	return Interpreter::get_keys(context, keys);
    }


}

/**
 * \brief Initializing function dlsym-ed by the Python interpreter
 *  when Graphite is loaded in an existing Python interpreter (i.e.,
 *  not using the main Graphite application.
 */
extern "C" gompy_API PyObject* PyInit_libgompy(void);
extern "C" gompy_API PyObject* PyInit_libgompy() {
    CmdLine::declare_arg("gel", "Python",
	    "Name of the graphite embedded language runtime"
    );
    CmdLine::set_arg("log:pretty",false);
    PyObject* result = PyInit_gom();
    Interpreter::initialize(new PythonInterpreter,"Python","py");
    Interpreter::initialize(new LuaInterpreter, "Lua", "lua");
    return result;
}

/**
 * \brief An alias for PyInit_libgompy(), because under Windows
 *  DLL names are not prefixed by "lib".
 */
extern "C" gompy_API PyObject* PyInit_gompy(void);
extern "C" gompy_API PyObject* PyInit_gompy() {
    return PyInit_libgompy();
}
