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

#include <OGF/gom/reflection/meta_class.h>
#include <OGF/gom/codegen/codegen.h>
#include <OGF/basic/os/text_utils.h>

#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/file_system.h>


#include <swig/Modules/swigmod.h>
#include <swig/CParse/cparse.h>

#include "gom.h"
#include "doxyparse.h"

//   A dlsym-visible variable to check whether
// main Graphite module is loaded.
//   Its presence just changes the behavior of
// the logger (less verbose if absent).
OGF_EXPORT extern int graphite_main;
OGF_EXPORT int graphite_main = 0;

namespace {

    // Get the SWIG version value in format 0xAABBCC from package version
    // expected to be in format A.B.C
    String* get_swig_version() {
	String *package_version = NewString(PACKAGE_VERSION);
	char *token = strtok(Char(package_version), ".");
	String *vers = NewString("SWIG_VERSION 0x");
	int count = 0;
	while (token) {
	    size_t len = strlen(token);
	    assert(len == 1 || len == 2);
	    Printf(vers, "%s%s", (len == 1) ? "0" : "", token);
	    token = strtok(nullptr, ".");
	    count++;
	}
	Delete(package_version);
	assert(count == 3); // Check version format is correct
	return vers;
    }


    // ------------------------------------------------------------------------
    // install_opts(int argc, char *argv[])
    // Install all command line options as preprocessor symbols
    // ------------------------------------------------------------------------


    void install_opts(int argc, char *argv[]) {
	int i;
	int noopt = 0;
	char *c;
	for (i = 1; i < (argc-1); i++) {
	    if (argv[i]) {
		if ((*argv[i] == '-') && (!isupper(*(argv[i]+1)))) {
		    String *opt = NewStringf("SWIGOPT%(upper)s", argv[i]);
		    Replaceall(opt,"-","_");
		    c = Char(opt);
		    noopt = 0;
		    while (*c) {
			if (!(isalnum(*c) || (*c == '_'))) {
			    noopt = 1;
			    break;
			}
			c++;
		    }
		    if (
			((i+1) < (argc-1)) && (argv[i+1]) &&
			(*argv[i+1] != '-')
		    ) {
			Printf(opt," %s", argv[i+1]);
			i++;
		    } else {
			Printf(opt," 1");
		    }
		    if (!noopt) {
			Preprocessor_define(opt, 0);
		    }
		}
	    }
	}
    }

    std::vector<std::string> include_path;
    std::string input_path;
    std::string output_path;
    bool dependencies=false;
    bool save_preprocessor_output=false;

    void parse_command_line(int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
	    if (argv[i]) {
		if (!strncmp(argv[i], "-deps", 5)) {
		    dependencies = true;
		    Swig_mark_arg(i);
		} else if(!strcmp(argv[i], "-E")) {
		    save_preprocessor_output=true;
		    Swig_mark_arg(i);
		} else if (!strncmp(argv[i], "-I", 2)) {
		    // Add a new directory search path
		    std::string include_dir = OGF::FileSystem::normalized_path(
			std::string(argv[i]+2)
		    );
		    // TODO: understand that: sometimes having /usr/include
		    // breaks some file parsing (problems in math.h)
		    if(include_dir != "/usr/include") {
			include_path.push_back(include_dir);
			Swig_add_directory((DOH *)(include_dir.c_str()));
		    }
		    Swig_mark_arg(i);
		} else if(!strncmp(argv[i], "-D", 2)) {
		    // TODO: add support for -Dxxx=yyy
		    std::string def = std::string(argv[i]+2) + " 1";
		    Preprocessor_define((DOH *)(def.c_str()), 0);
		    Swig_mark_arg(i);
		} else if(!strncmp(argv[i], "-i", 2)) {
		    input_path = OGF::FileSystem::normalized_path(
			std::string(argv[i] + 2)
		    );
		    Swig_mark_arg(i);
		} else if(!strncmp(argv[i], "-o", 2)) {
		    output_path = OGF::FileSystem::normalized_path(
			std::string(argv[i] + 2)
		    );
		    Swig_mark_arg(i);
		} else if (
		    !strcmp(argv[i],"-version") ||
		    !strcmp(argv[i],"-h")       ||
		    !strcmp(argv[i],"-help")
		    ) {
		    fprintf(stderr,"\nSWIG Version %s\n", PACKAGE_VERSION);
		    fprintf(stderr,"Modified for Graphite/GOM\n");
		    fprintf(stderr,"Copyright (c) 1995-1998\n");
		    fprintf(stderr,"University of Utah and the Regents of \n");
		    fprintf(stderr,"the University of California\n");
		    fprintf(stderr,"Copyright (c) 1998-2003\n");
		    fprintf(stderr,"University of Chicago\n");
		    fprintf(stderr,"This is a *modified* version of SWIG\n");
		    exit(EXIT_SUCCESS);
		}
	    }
	}


	if(!OGF::FileSystem::is_directory(input_path)) {
	    OGF::Logger::err("Gom::CodeGen")
		<< "Specified input path \'"
		<< input_path << "\': no such directory"
		<< std::endl;
	    exit(EXIT_FAILURE);
	}

	for(size_t i=0; i<include_path.size(); ++i) {
	    if(!OGF::FileSystem::is_directory(include_path[i])) {
		OGF::Logger::err("Gom::CodeGen")
		    << "Specified include path \'"
		    << include_path[i] << "\': no such directory"
		    << std::endl;
		exit(EXIT_FAILURE);
	    }
	}

	if(output_path == "") {
	    OGF::Logger::err("Gom::CodeGen")
		<< "Output file was not specified (missing -oFileName.cpp)"
		<< std::endl;
	    exit(EXIT_FAILURE);
	}
    }

    bool is_gom_header(const std::string& filename) {
	return (
	    OGF::FileSystem::extension(filename) == "h" &&
	    OGF::TextUtils::file_contains_string(filename, "gom_class")
	    );
    }

    void remove_path(std::string& filename) {
	size_t prefix_length=0;
	for(size_t i=0; i<include_path.size(); ++i) {
	    if(OGF::String::string_starts_with(filename, include_path[i])) {
		size_t this_prefix_length = include_path[i].length();
		if(include_path[i][include_path[i].length()-1] != '/') {
		    ++this_prefix_length;
		}
		prefix_length=std::max(prefix_length, this_prefix_length);
	    }
	}
	if(prefix_length != 0) {
	    filename = filename.substr(
		prefix_length,filename.length()-prefix_length
		);
	}
    }

    void find_gom_headers(
	const std::string& path, std::vector<std::string>& gom_headers
    ) {
	GEO::Logger::out("gom_classes") << "<< " << path << std::endl;
	std::vector<std::string> remaining_directories;
	remaining_directories.push_back(path);
	while(remaining_directories.size() > 0) {
	    std::string current_directory = *remaining_directories.rbegin();
	    remaining_directories.pop_back();
	    if (!OGF::FileSystem::is_file(
		    current_directory + "/" + "gomgen.skip")
	    ) {
		OGF::FileSystem::get_subdirectories(
		    current_directory, remaining_directories , false
		);

		std::vector<std::string> files_in_directory;
		OGF::FileSystem::get_directory_entries(
		    current_directory, files_in_directory
		);
		for(GEO::index_t i=0; i<files_in_directory.size(); ++i) {
		    if(is_gom_header(files_in_directory[i])) {
			std::string cur_file = files_in_directory[i];
			if(!dependencies) {
			    remove_path(cur_file);
			}
			gom_headers.push_back(cur_file);
		    }
		}
	    }
	}
    }

    void assemble_swig_source(const std::vector<std::string>& gom_headers) {
	input_file = const_cast<char*>("tmp_gomgen.cpp");
	std::ofstream out((char*)input_file);
	for(size_t i=0; i<gom_headers.size(); ++i) {
	    out << "#include <" << gom_headers[i] << ">" << std::endl;
	}
    }

    void cleanup_swig_source() {
	OGF::FileSystem::delete_file((char*)input_file);
    }

    DOH* run_preprocessor(int argc, char** argv) {
	String *fs = NewString("");
	FILE *df = Swig_open(input_file);
	if (!df) {
	    Printf(stderr,"Unable to find '%s'\n", input_file);
	    exit (EXIT_FAILURE);
	}
	fclose(df);
	Printf(fs,"%%include \"%s\"\n", Swig_last_file());
	Seek(fs,0,SEEK_SET);
	DOH* result = Preprocessor_parse(fs);
	if (Swig_error_count()) {
	    exit(EXIT_FAILURE);
	}
	Seek(result, 0, SEEK_SET);
	if(save_preprocessor_output) {
	    FILE* cppout = fopen((output_path + ".I").c_str(),"w");
	    fprintf(cppout, "command line:\n");
	    for(int i=0; i<argc; ++i) {
		fprintf(cppout, "   %s\n", argv[i]);
	    }
	    char* result_string = Char(result);
	    fprintf(cppout,"%s",result_string);
	    fclose(cppout);
	}
	return result;
    }

    std::string get_package_name(const std::string& input_path_in) {
	std::string input_path_tmp = input_path_in;
	if(input_path_tmp[input_path_tmp.length()-1] == '/') {
	    input_path_tmp = input_path_tmp.substr(
		0, input_path_tmp.length()-1
	    );
	}
	std::string result=OGF::FileSystem::base_name(input_path_tmp);
	return result;
    }

    void run_generator(
	Language* lang, const std::vector<std::string>& sources,
	DOH* cpps, std::ofstream& out,
	int argc, char** argv
    ) {
	Node *top = Swig_cparse(cpps);
	Swig_process_types(top);
	Swig_default_allocators(top);

	if (top) {
	    if (Swig_contract_mode_get()) {
		Swig_contracts(top);
	    }
	    lang->top(top);

	    out << "// GOMGEN automatically generated code" << std::endl;
	    out << "// Do not edit." << std::endl;
	    out << std::endl;
	    out << "// Command line:" << std::endl;
	    for(int i=0; i<argc; ++i) {
		out << "//  " << argv[i] << std::endl;
	    }
	    out << std::endl;
	    out << std::endl;
	    out << "// Include path:" << std::endl;
	    for(GEO::index_t i=0; i<include_path.size(); ++i) {
		out << "//   " << include_path[i] << std::endl;
	    }
	    out << "// Input path:" << std::endl;
	    out << "//   " << input_path << std::endl;
	    out << "// Output file:" << std::endl;
	    out << "//   " << output_path << std::endl;
	    out << std::endl;
	    out << std::endl;

	    for(size_t i =0; i<sources.size(); ++i) {
		out << "#include <" << sources[i] << ">" << std::endl;
	    }
	    out << "#include <OGF/gom/types/gom_implementation.h>" << std::endl;
	    out << std::endl;
	    out << std::endl;


	    out << "#ifdef GEO_COMPILER_GCC" << std::endl;
	    out << "#pragma GCC diagnostic ignored \"-Wunused-parameter\""
		<< std::endl;
	    out << "#pragma GCC optimize (\"O0\")"
		<< std::endl;
	    out << "#endif" << std::endl;
	    out << std::endl;

	    out << "#ifdef GEO_COMPILER_MSVC" << std::endl;
	    out << "#pragma warning(disable: 4100)" << std::endl;
	    out << "#endif" << std::endl;
	    out << std::endl;

	    out << "#ifdef GEO_COMPILER_CLANG" << std::endl;
	    out << "#pragma GCC diagnostic ignored \"-Wweak-vtables\""
		<< std::endl;
	    out << "#pragma GCC diagnostic ignored \"-Wmissing-prototypes\""
		<< std::endl;
	    out << "#pragma GCC diagnostic ignored \"-Wunused-parameter\""
		<< std::endl;
	    out << "#endif" << std::endl;


	    out << std::endl;
	    out << std::endl;

	    const std::vector<OGF::MetaClass*> classes =
		get_swig_gom_generated_classes();

	    OGF::GomCodeGenerator generator;
	    OGF::Logger::out("Gom::CodeGen") << ">>" << std::endl;
	    generator.generate(out, classes, get_package_name(input_path));
	}
    }
}

/****************************************************************************/

int main(int argc, char** argv) {

    Swig_init_args(argc, argv);
    Swig_init();
    Swig_warnfilter("201,202,309,401,403,501,502,503,512,321,322",1);
    Preprocessor_init();
    Language* lang = get_swig_gom_language();

    Preprocessor_define((DOH *) "SWIG 1", 0);
    Preprocessor_define((DOH *) "__STDC__ 1", 0);
    Preprocessor_define((DOH *) "GOMGEN 1", 0);

    // Let's pretend its a specific architecture to
    // make some include files happy.
    Preprocessor_define((DOH *) "__linux__ 1", 0);
    Preprocessor_define((DOH *) "__GNUC__ 1", 0);
    Preprocessor_define((DOH *) "__x86_64__ 1", 0);

    // Turn on contracts (TODO: figure out what it is)
    Swig_contract_mode_set(1);
    Preprocessor_define(get_swig_version(),0);

    // Turn on director protected mode (TODO: figure out what it is)
    Wrapper_director_protected_mode_set(0);

    // Turn on C++ mode
    CPlusPlus=1;
    Preprocessor_define((DOH *) "__cplusplus 1", 0);
    Swig_cparse_cplusplus(1);

    // Report preprocessor errors as warnings
    Preprocessor_error_as_warning(1);

    // Turn on -includeall and -ignoremissing flags
    Preprocessor_include_all(1);
    Preprocessor_ignore_missing(1);

    parse_command_line(argc, argv);

    set_swig_gom_package(get_package_name(input_path),input_path);

    // Define the __cplusplus symbol
    if (CPlusPlus) {
        Preprocessor_define((DOH *) "__cplusplus 1", 0);
    }

    // Parse language dependent options
    lang->main(argc,argv);

    // Check all of the options to make sure we're cool.
    if (argc > 1) {
        // 0: do not check for an input file
        Swig_check_options(0);
    }

    // Install options as preprocessor defines
    install_opts(argc, argv);

    std::vector<std::string> gom_headers;
    find_gom_headers(input_path, gom_headers);

    if(dependencies) {
        std::ofstream out(output_path.c_str());
        for(size_t i=0; i<gom_headers.size(); ++i) {
            out << gom_headers[i] << std::endl;
        }
    } else {
        assemble_swig_source(gom_headers);
        Preprocessor_register_comment_CB(doxyparse);
        DOH* cpps = run_preprocessor(argc,argv);
        cleanup_swig_source();
        // Register a null file with the file handler
        Swig_register_filebyname("null", NewString(""));
        std::ofstream out(output_path.c_str());
        run_generator(lang,gom_headers,cpps,out,argc,argv);
    }

    return swig_gom_error_occured() ? -1 : 0;
}
