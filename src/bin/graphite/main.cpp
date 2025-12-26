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

#include <OGF/scene_graph/skin/preferences.h>
#include <OGF/gom/interpreter/interpreter.h>
#include <OGF/gom/lua/lua_interpreter.h>
#include <OGF/gom/reflection/meta.h>
#include <OGF/basic/modules/modmgr.h>
#include <OGF/basic/os/file_manager.h>

#include <geogram/basic/command_line.h>
#include <geogram/basic/file_system.h>
#include <iostream>

#if defined(GEO_OS_LINUX)
#include <dlfcn.h>
#elif defined(GEO_OS_WINDOWS)
#include <Windows.h>
#endif

// A dlsym-visible variable to check whether
// main Graphite module is loaded.
//   Its presence just changes the behavior of
// the logger (less verbose if absent).
OGF_EXPORT extern int graphite_main;
OGF_EXPORT int graphite_main = 0;

#if defined(GEO_OS_WINDOWS)

// NVidia/Optimus GPU selection under windows:
// GPU selection is enabled  when this symbol is exported by the main program
// (or by a library *statically* linked to it). Then the NVidia GPU is selected
// if this value is non-zero when the OpenGL context is created.
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000000;
}

int main(int argc, char** argv) ;

/**
 * \brief Converts Windows command line into argc/argv format.
 * \details
 *  This function was grabbed from: http://alter.org.ua/docs/win/args/ \n
 *  Windows API already has CommandLineToArgW that works
 *  with Unicode strings, but no equivalent that works with
 *  plain ASCII strings.
 * \param[in] CmdLine a pointer to the string that represents
 *  the command line, as returned by GetCommandLine()
 * \param[out] _argc a pointer to the number of arguments
 * \return an array of pointers to the arguments as strings, that
 *  contains *_argc strings.
 */
LPSTR* CommandLineToArgvA(PCHAR CmdLine, int* _argc) {
    PCHAR* argv;
    PCHAR  _argv;
    size_t len;
    ULONG argc;
    CHAR a;
    size_t i, j;

    BOOLEAN  in_QM;
    BOOLEAN  in_TEXT;
    BOOLEAN  in_SPACE;

    len = strlen(CmdLine);
    i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

    argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
                               i + (len+2)*sizeof(CHAR));

    _argv = (PCHAR)(((PUCHAR)argv)+i);

    argc = 0;
    argv[argc] = _argv;
    in_QM = FALSE;
    in_TEXT = FALSE;
    in_SPACE = TRUE;
    i = 0;
    j = 0;

    while( (a = CmdLine[i]) != '\0' ) {
        if(in_QM) {
            if(a == '\"') {
                in_QM = FALSE;
            } else {
                _argv[j] = a;
                j++;
            }
        } else {
            switch(a) {
            case '\"':
                in_QM = TRUE;
                in_TEXT = TRUE;
                if(in_SPACE) {
                    argv[argc] = _argv+j;
                    argc++;
                }
                in_SPACE = FALSE;
                break;
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                if(in_TEXT) {
                    _argv[j] = '\0';
                    j++;
                }
                in_TEXT = FALSE;
                in_SPACE = TRUE;
                break;
            default:
                in_TEXT = TRUE;
                if(in_SPACE) {
                    argv[argc] = _argv+j;
                    argc++;
                }
                _argv[j] = a;
                j++;
                in_SPACE = FALSE;
                break;
            }
        }
        i++;
    }
    _argv[j] = '\0';
    argv[argc] = nullptr;

    (*_argc) = argc;
    return argv;
}

/**
 * \brief The WinMain function, called by Windows executable
 * \details Gets the command line from Windows API, converts it
 *  into argc/argv format and passes it to main()
 */
int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
) {
    ogf_argused(hInstance);
    ogf_argused(hPrevInstance);
    ogf_argused(lpCmdLine);
    ogf_argused(nCmdShow);
    int argc;
    LPSTR* argv = CommandLineToArgvA(GetCommandLine(), &argc);
    int result = main(argc, argv);
    LocalFree(argv);
    return result;
}
#endif


namespace {

    using namespace OGF;

    void parse_command_line(int argc, char** argv) {

	// second arg true -> auto create args from graphite.ini
	CmdLine::set_config_file_name("graphite.ini", true);

        CmdLine::declare_arg(
            "gel", "Lua",
	    "Name of the graphite embedded language runtime (default: uses builtin lua interpreter)"
        );

        CmdLine::declare_arg(
            "skin","", "Name of the skin runtime"
        );

        CmdLine::declare_arg(
            "main","lib/graphite.lua", "Name of the main gel script"
        );

        CmdLine::declare_arg(
            "base_modules", "luagrob;mesh;mesh_gfx;voxel;voxel_gfx",
            "list of plugins to be loaded at startup "
	    "(separator = ';' or 'none' for empty list)"
        );

        CmdLine::declare_arg(
            "batch", false, "batch mode (i.e., no GUI)"
        );

        CmdLine::declare_arg(
            "interactive", false,
	    "interactive mode (i.e., open a GEL shell after startup)"
        );

        CmdLine::declare_arg(
            "shell", false,
	    "shorthand for batch=true and interactive=true"
        );

        std::vector<std::string> filenames;
        if(!CmdLine::parse(argc,argv,filenames,"<inputfile>*")) {
            exit(-1);
        }

// Dynamic selection of NVidia adapter under Windows,
// See comments near NvOptimusEnablement declaration (beginning of this file)
#ifdef GEO_OS_WINDOWS
        NvOptimusEnablement = (CmdLine::get_arg("gfx:adapter") == "nvidia");
#endif

        // Copy command line in Graphite environment variable
        // (used by graphite.lua post_init callback to load objects after
        // the GUI is fully created and ready)
        std::string command_line;
        for(int i=1; i<argc; i++) {
            if(i != 1) {
                command_line += "!";
            }
            command_line += argv[i];
        }
        Environment::instance()->set_value("command_line", command_line);
    }

    void declare_preference_variables() {

        Preferences::declare_preference_variable(
	    "modules","","plugins to be loaded"
	);

	CmdLine::declare_arg_group("gel","graphite embedded language (lua)");
	Preferences::declare_preference_variable(
	    "gel:startup_files", "", "Lua files to be loaded at startup"
	);
        for(index_t i=1; i<=10; ++i) {
	   std::string varname = "gel:F" + String::to_string(i) + "_script";
	   Preferences::declare_preference_variable(
	       varname, "", "Lua script bound to a key (filename)"
	   );
        }

	CmdLine::declare_arg_group("gui","graphite user interface");
        Preferences::declare_preference_variable(
	    "gui:style", "Dark","style"
	);
        Preferences::declare_preference_variable(
	    "gui:font_size", 18, "font size"
	);
        Preferences::declare_preference_variable(
	    "gui:tooltips", true, "Display tooltips on commands and arguments"
	);

        Preferences::declare_preference_variable(
	    "gui:undo", false, "Support undo for all commands"
	);

        Preferences::declare_preference_variable(
	    "gui:undo_depth", 4, "number of memorized states for undo"
	);

        Preferences::declare_preference_variable(
	    "gfx:default_full_screen_effect", "Plain",
	    "full-screen effect enabled by default"
	);
        Preferences::declare_preference_variable("gfx:GL_debug");
        Preferences::declare_preference_variable("gfx:GL_profile");
        Preferences::declare_preference_variable("gfx:GLUP_profile");
	Preferences::declare_preference_variable("gfx:adapter");
	Preferences::declare_preference_variable(
	    "gfx:polygon_offset", true,
	    "avoid Z fighting by slightly shifting lines"
	);
        Preferences::declare_preference_variable("log:file_name");
        Preferences::declare_preference_variable("log:features");
        Preferences::declare_preference_variable("log:features_exclude");
        Preferences::declare_preference_variable("sys:multithread");
        Preferences::declare_preference_variable("sys:FPE");
        Preferences::declare_preference_variable("sys:max_threads");
#ifdef GEO_OS_WINDOWS
	Preferences::declare_preference_variable("sys:show_win32_console");
#endif
	Preferences::declare_preference_variable("gfx:full_screen");

        Preferences::declare_preference_variable(
	      "gui:keyboard_nav",true,"keyboard navigation"
	);
        Preferences::declare_preference_variable(
	      "gui:viewports",false,"individual dockable windows for dialogs"
	);
    }

    void load_skin() {
        std::string skin = CmdLine::get_arg("skin");
        if(skin == "none") {
            return;
        }
	if(skin == "") {
	    skin = "imgui";
	}

        if(skin != "") {
	    if(!String::string_starts_with(skin, "skin_")) {
		skin = "skin_" + skin;
		CmdLine::set_arg("skin",skin);
	    }
            ModuleManager::instance()->load_module(skin);
	    if(
		skin == "skin_imgui" &&
		CmdLine::get_arg("main") == "lib/graphite.lua"
	    ) {
		CmdLine::set_arg("main","lib/graphite_imgui.lua");
	    }

            if(Meta::instance()->meta_type_is_bound("OGF::Application")) {
                return;
            } else {
                Logger::err("Graphite")
                    << "Could not load skin module " << skin << std::endl;
                exit(-1);
            }
        }

        if(!Meta::instance()->meta_type_is_bound("OGF::Application")) {
            Logger::err("Fatal") << "Could not create any skin" << std::endl;
            exit(-1);
        }

    }

    void load_modules(const std::string& modules_str) {
        std::vector<std::string> modules;
        String::split_string(modules_str, ';', modules);
        for(unsigned int i=0; i<modules.size(); i++) {
            ModuleManager::instance()->load_module(modules[i]);
        }
    }

    void load_base_modules() {
        std::string base_modules = CmdLine::get_arg("base_modules");
        Logger::out("ModuleMgr")
            << "base_modules=" << base_modules << std::endl;
        if(base_modules != "none") {
            load_modules(base_modules);
        }
    }

    void load_plugin_modules() {
        if(Environment::instance()->has_value("modules")) {
            std::string modules_str =
                Environment::instance()->get_value("modules");
            load_modules(modules_str);
        }
    }

    Interpreter* get_interpreter() {
	static Interpreter* result = nullptr;
	if(result == nullptr) {
	    std::string gel = CmdLine::get_arg("gel");
	    result = Interpreter::instance_by_language(gel);
	    if(result == nullptr) {
		Logger::err("GEL") << "Fatal: " << gel << ": no such language"
				   << std::endl;
		exit(-1);
	    }
	}
	return result;
    }


    void load_interpreter() {
	Interpreter::initialize(new LuaInterpreter, "Lua", "lua");
	Interpreter* interp = get_interpreter();
	std::string main_ext = FileSystem::extension(CmdLine::get_arg("main"));
	if(main_ext != "" && main_ext != interp->get_filename_extension()) {
	    CmdLine::set_arg("main","none");
	}
    }

    /**
     * \brief To avoid some DLL nightmare with Windows,
     *  in particular when using gompy,
     *  try to predeclare path where Python lib is likely to be.
     */
    void add_libpath() {
#ifdef GEO_OS_WINDOWS
	std::vector<std::string> path;

	// The first one is deduced from where the python lib was found when compiling
	// (see CMakeLists.txt). This ensures at least that the user who compiled it will
	// be able to use it.
	path.push_back(FileSystem::normalized_path(MODMGR_APPEND_LIBPATH));

	// Then search for Anaconda in its standard path (note: we need to go "../" because
	// FileSystem::home_directory() returns the path to the "My documents" folder.
	path.push_back(FileSystem::normalized_path(FileSystem::home_directory() + "/Anaconda3"));
	path.push_back(FileSystem::normalized_path(FileSystem::home_directory() + "/../Anaconda3"));

	// Not sure it goes there when installed "for all users" (to be checked)
	path.push_back("C:/Anaconda3");
	path.push_back("C:/ProgramData/Anaconda3");

	// Try all of them, take the first that works.
	for(index_t i=0; i<path.size(); ++i) {
	    if(FileSystem::is_directory(path[i])) {
		Logger::out("ModuleMgr") << "Declaring libpath: " << path[i] << std::endl;
		ModuleManager::append_dynamic_libraries_path(path[i]);
		Logger::out("ModuleMgr") << "Setting PYTHONHOME as " << path[i] << std::endl;
		SetEnvironmentVariable("PYTHONHOME",path[i].c_str());
  	        break;
	    } else {
		Logger::out("ModuleMgr") << "Ignored libpath (does not exist): "
					 << path[i] << std::endl;
	    }
	}

#endif
    }

}

void simple_command_line_interpreter_loop(void);

/**
 * \brief Default interactive interpreter loop,
 *  uses C++ I/O.
 */
void simple_command_line_interpreter_loop() {
    std::string line;
    while(std::getline(std::cin,line)) {
	std::cout << "GEL>> " << std::flush;
	get_interpreter()->execute(line,false,false);
    }
}

#ifdef GEO_OS_LINUX

typedef char* (*FPTR_readline)(const char*);
typedef void (*FPTR_add_history)(const char*);
typedef char** (*FPTR_completion_entry)(const char*, int, int);
typedef char* (*FPTR_completion_generator)(const char* text, int state);
typedef char** (*FPTR_completion_matches)(
    const char*, FPTR_completion_generator
);

static FPTR_readline readline = nullptr;
static FPTR_add_history add_history = nullptr;
static FPTR_completion_entry* p_rl_attempted_completion_function = nullptr;
static FPTR_completion_entry* p_rl_completion_entry_function = nullptr;
static FPTR_completion_matches rl_completion_matches = nullptr;
static int* p_rl_attempted_completion_over = nullptr;
static char** p_rl_line_buffer = nullptr;
static const char** p_rl_completer_word_break_characters = nullptr;
static char* p_rl_completion_append_character = nullptr;

static void* HNDL_libreadline = nullptr;
static char* line_read = nullptr;
static std::string line_read_str;

static index_t compl_start = index_t(-1);
static index_t compl_end = index_t(-1);

/**
 * \brief Deallocates everything associated with libreadline.
 */
void terminate_readline(void);

void terminate_readline() {
    free(line_read);
    dlclose(HNDL_libreadline);
}

char* completion_generator(const char* text, int state);

char* completion_generator(const char* text, int state) {
    // This function is called with state=0 the first time; subsequent calls are
    // with a nonzero state. state=0 can be used to perform one-time
    // initialization for this completion session.
    static std::vector<std::string> matches;
    static size_t match_index = 0;
    char* result = nullptr;

    if (state == 0) {
	// During initialization, compute the actual matches for 'text' and keep
	// them in a static vector.
	matches.clear();
	match_index = 0;

	get_interpreter()->automatic_completion(
	    std::string(*p_rl_line_buffer),
	    compl_start, compl_end,
	    std::string(text),
	    matches
	);
    }

    if (match_index >= matches.size()) {
	// We return nullptr to notify the caller no more matches are available.
	result = nullptr;
    } else {
	// Return a malloc'd char* for the match. The caller frees it.
	result = strdup(matches[match_index++].c_str());
    }
    return result;
}

char** completer(const char* text, int start, int end);
char** completer(const char* text, int start, int end) {
    compl_start = index_t(start);
    compl_end = index_t(end);

    // Don't do filename completion even if our generator finds no matches.
    *p_rl_attempted_completion_over = 1;

    // Do not append trailing space.
    *p_rl_completion_append_character = '\0';

    // Note: returning nullptr here will make readline use the default filename
    // completer.
    return rl_completion_matches(text, completion_generator);
}

/**
 * \brief Tentatively loads libreadline dynamically and finds
 *  functions in it.
 * \retval true if libreadline could be loaded and initialized.
 * \retval false otherwise.
 */
bool init_readline() {
    HNDL_libreadline = dlopen("libreadline.so.8", RTLD_NOW);
    if(HNDL_libreadline == nullptr) {
	return false;
    }
    atexit(terminate_readline);
    readline = (FPTR_readline)dlsym(HNDL_libreadline, "readline");
    add_history = (FPTR_add_history)dlsym(HNDL_libreadline, "add_history");
    p_rl_attempted_completion_function = (FPTR_completion_entry*)dlsym(
	HNDL_libreadline, "rl_attempted_completion_function"
    );
    p_rl_completion_entry_function = (FPTR_completion_entry*)dlsym(
	HNDL_libreadline, "rl_completion_entry_function"
    );
    rl_completion_matches = (FPTR_completion_matches)dlsym(
	HNDL_libreadline, "rl_completion_matches"
    );
    p_rl_attempted_completion_over = (int*)dlsym(
	HNDL_libreadline, "rl_attempted_completion_over"
    );
    p_rl_line_buffer = (char**)dlsym(
	HNDL_libreadline, "rl_line_buffer"
    );
    p_rl_completer_word_break_characters = (const char**)dlsym(
	HNDL_libreadline, "rl_completer_word_break_characters"
    );
    p_rl_completion_append_character = (char*)dlsym(
	HNDL_libreadline, "rl_completion_append_character"
    );
    return (
	readline != nullptr &&
	add_history != nullptr &&
	p_rl_attempted_completion_function != nullptr &&
	p_rl_completion_entry_function != nullptr &&
	rl_completion_matches != nullptr &&
	p_rl_attempted_completion_over != nullptr &&
	p_rl_line_buffer != nullptr &&
	p_rl_completer_word_break_characters != nullptr &&
	p_rl_completion_append_character != nullptr
    );
}

void command_line_interpreter_loop(void);

/**
 * \brief Interactive interpreter loop that uses libreadline
 *  if it is detected.
 */
void command_line_interpreter_loop() {
    if(init_readline()) {
	Logger::out("GEL")
	    << "[...Using libreadline - Activating <tab> completion...]"
	    << std::endl;
	*p_rl_attempted_completion_function = completer;
	*p_rl_completer_word_break_characters = " .(){},+-*/=";
	for(;;) {
	    if(line_read != nullptr) {
		free(line_read);
		line_read = nullptr;
	    }
	    line_read = readline("GEL>> ");
	    if(line_read == nullptr) {
		break;
	    }
	    if(line_read[0] != '\0') {
		add_history(line_read);
	    }
	    get_interpreter()->execute(
		std::string(line_read),false,false
	    );
	}
    } else {
	Logger::out("GEL") << "Using default input" << std::endl;
	simple_command_line_interpreter_loop();
    }
}

#else

void command_line_interpreter_loop(void);

/**
 * \brief Interactive interpreter loop.
 * \details Uses the default implementation on non-unix platforms.
 */
void command_line_interpreter_loop() {
    simple_command_line_interpreter_loop();
}

#endif

int main(int argc, char** argv) {
    using namespace OGF;

    declare_preference_variables();
    parse_command_line(argc, argv);
    add_libpath();


#ifdef GEO_OS_WINDOWS
    if(!CmdLine::get_arg_bool("sys:show_win32_console")) {
	HWND h = GetConsoleWindow(); // May be nullptr if compiled as win app.
	if(h != nullptr) {
	    ShowWindow(h, SW_HIDE);
	}
    }
#endif

    if(CmdLine::get_arg_bool("shell")) {
	CmdLine::set_arg("batch", true);
	CmdLine::set_arg("interactive", true);
    }

    if(CmdLine::get_arg_bool("batch")) {
        CmdLine::set_arg("skin", "none");
        CmdLine::set_arg("main", "lib/graphite_batch.lua");
    }

    load_base_modules();
    load_skin();
    load_plugin_modules();
    load_interpreter();

    Logger::out("Graphite///") << "Hello, world !!" << std::endl;
    Logger::out("Graphite///") << "Starting main GEL script" << std::endl;

    if(get_interpreter() != nullptr) {
        std::string gel_filename = CmdLine::get_arg("main");
        if(gel_filename != "none") {
            if(!FileManager::instance()->find_file(gel_filename)) {
                Logger::err("Graphite///")
                    << "Could not find main GEL script: "
                    << gel_filename << std::endl;
                exit(-1);
            }
            get_interpreter()->execute_file(gel_filename);
        }

        if(
	    CmdLine::get_arg_bool("batch") &&
	    CmdLine::get_arg("gel") == "Lua"
	) {
            get_interpreter()->execute("post_init()",false,false);
        }

	// Keep a reference to the Graphite application object
	// so that we can make sure it is the last object destroyed
	// (it will be destroyed *after* the Lua context).
	Object_var app = get_interpreter()->resolve_object("main");

	// Start the application.
	if(!app.is_null()) {
	    app->invoke_method("start");
	}

        if(
            gel_filename == "none" ||
            CmdLine::get_arg_bool("interactive")
        ) {
	    if(CmdLine::get_arg_bool("log:pretty")) {
		CmdLine::ui_close_separator();
		CmdLine::set_arg("log:pretty", false);
	    }
	    command_line_interpreter_loop();
	    std::cout << std::endl;
        }

	// Clear all variables from the interpreter.
	// This destroys all Graphite shaders and
	// graphics objects.
	get_interpreter()->reset();

	// Now we can destroy the app by resetting the
	// smart pointer to nil. (this destroys the
	// window and GL context in turn).
	// Note: this would be called automatically even if
	//  this line was not there, but I keep it so that
	//  what is done is clearer).
	app.reset();
    }

    Logger::out("Graphite///") << "Goodbye, world !!"  << std::endl;
}
