/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
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

#ifndef H_OGF_SCENE_GRAPH_TYPES_GEOFILE_H
#define H_OGF_SCENE_GRAPH_TYPES_GEOFILE_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/gom/types/arg_list.h>
#include <geogram/basic/geofile.h>

/**
 * \file OGF/scene_graph/types/geofile.h
 * \brief Structured binary files for saving Graphite scene graph.
 */

namespace OGF {

    /***************************************************************/

    /**
     * \brief An extension of InputGeoFile for storing a complete 
     *  Graphite scenegraph in a structured binary file.
     * \details In addition to GeoFile, InputGraphiteFile handles the
     *  following chunk classes:
     *  - SCNE (Scene): an ArgList with the global attributes related with 
     *   the Graphite scene graph (current_object)
     *  - HIST (History): a strings array with all the commands that were
     *   recorded in the Graphite session
     *  - GROB (Graphite Object): an ArgList with the attributes that define
     *   a grob (name and classname)
     *  - SHDR (Shader): an ArgList with the attributes that define a shader
     *   attached to a grob (classname and all the properties)
     */
    class SCENE_GRAPH_API InputGraphiteFile : public InputGeoFile {
    public:
        /**
         * \copydoc InputGeoFile::InputGeoFile()
         */
        InputGraphiteFile(const std::string& filename);

        /**
         * \brief Reads a SceneGraph header
         * \param[out] args a reference to the ArgList that 
         *  defines the grob name, class name and attributes. 
         * \pre current_chunk_class() == "SCNG"
         */
        void read_scene_graph_header(ArgList& args);
            
        /**
         * \brief Reads a grob header
         * \param[out] args a reference to the ArgList that 
         *  defines the grob name, class name and attributes. 
         * \pre current_chunk_class() == "GROB"
         */
        void read_grob_header(ArgList& args);

        /**
         * \brief Readss shader informations.
         * \param[in] args a reference to the ArgList that defines 
         *  the shader class name and properties.
         * \pre current_chunk_class() == "SHDR"
         */
        void read_shader(ArgList& args);

        /**
         * \brief Reads the commands history from the geofile.
         * \param[out] history a reference to a vector of strings 
         *  with the history
         * \pre current_chunk_class() == "HIST"
         */
        void read_history(std::vector<std::string>& history);
        
        /**
         * \brief Reads an ArgList from the GeoFile
         * \param[out] args the read ArgList
         */
        void read_arg_list(ArgList& args);
    };

    /***************************************************************/

    /**
     * \brief An extension of InputGeoFile for storing a complete 
     *  Graphite scenegraph in a structured binary file.
     * \details In addition to GeoFile, InputGraphiteFile handles the
     *  following chunk classes:
     *  - SCNE (Scene): an ArgList with the global attributes related with 
     *   the Graphite scene graph (current_object)
     *  - HIST (History): a strings array with all the commands that were
     *   recorded in the Graphite session
     *  - GROB (Graphite Object): an ArgList with the attributes that define
     *   a grob (name and classname)
     *  - SHDR (Shader): an ArgList with the attributes that define a shader
     *   attached to a grob (classname and all the properties)
     */
    class SCENE_GRAPH_API OutputGraphiteFile : public OutputGeoFile {
    public:
        /**
         * \copydoc OutputGeoFile::OutputGeoFile()
         */
        OutputGraphiteFile(
            const std::string& filename, index_t compression_level=3
        );

        /**
         * \brief Writes the commands history into the geofile.
         * \param[in] history a vector of strings with the history
         */
        void write_history(const std::vector<std::string>& history);

        /**
         * \brief Writes scene graph informations.
         * \param[in] args the ArgList that defines the scene graph
         *  properties to be saved.
         */
        void write_scene_graph_header(const ArgList& args);
        
        /**
         * \brief Writes a grob header
         * \param[in] args the ArgList that defines the grob name, 
         *  class name and attirbutes. It should have at least 
         *  class_name and name
         */
        void write_grob_header(const ArgList& args);

        /**
         * \brief Writes shader informations.
         * \param[in] args the ArgList that defines the shader class
         *  and properties, should have at least class_name.
         */
        void write_shader(const ArgList& args);
        
        /**
         * \brief Writes an ArgList to the GeoFile
         * \param[in] args a const reference to the ArgList
         */
        void write_arg_list(const ArgList& args);
        
        /**
         * \brief Computes the size of an ArgList
         * \param[in] args a const reference to the ArgList
         * \return the size in bytes required to store the ArgList
         *  in a GeoFile.
         */
        size_t arg_list_size(const ArgList& args) const;
    };

    /***************************************************************/    
}

#endif

