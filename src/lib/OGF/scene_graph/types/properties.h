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
 
 
#ifndef H_OGF_SCENE_GRAPH_TYPES_SHADER_PROPERTIES_H
#define H_OGF_SCENE_GRAPH_TYPES_SHADER_PROPERTIES_H

#include <OGF/scene_graph/common/common.h>
#include <OGF/gom/types/arg_list.h>
#include <geogram/image/color.h>

#include <iostream>
#include <string>

/**
 * \file OGF/scene_graph/types/properties.h
 * \details Types used by shader properties and command arguments.
 */

namespace GEO {
    class Image;
}

namespace OGF {

    //________________________________________________________

    /**
     * \brief Drawing style for points.
     */
    struct PointStyle {
        /**
         * \brief PointStyle constructor.
         * \details Initializes the PointStyle with default values.
         */
        PointStyle() :
            visible(false),
            color(0.0,0.0,0.0,1.0),
            size(4) {
        }
        /**
         * \brief true if points are visible, false otherwise.
         */
        bool visible;

        /**
         * \brief Color of the points.
         */
        Color color;

        /**
         * \brief Size of the points.
         */
        index_t size;
    };

    /**
     * \brief Writes a PointStyle to a stream.
     * \param[in,out] out the stream
     * \param[in] ps a const reference to the PointStyle
     * \return the new state of the stream after writing
     */
    SCENE_GRAPH_API std::ostream& operator<<(
        std::ostream& out, const PointStyle& ps
    );

    /**
     * \brief Reads a PointStyle from a stream.
     * \param[in,out] in the stream
     * \param[out] ps a reference to the PointStyle
     * \return the new state of the stream after reading
     */
    SCENE_GRAPH_API std::istream& operator>>(
        std::istream& in, PointStyle& ps
    );

    //________________________________________________________

    /**
     * \brief Drawing style for mesh edges.
     */
    struct EdgeStyle {
        /**
         * \brief EdgeStyle constructor.
         * \details Initializes the EdgeStyle with default values.
         */
        EdgeStyle() :
            visible(false),
            color(0.0,0.0,0.0,1.0),
            width(1) {
        }
        
        /**
         * \brief true if edges are visible, false otherwise.
         */
        bool visible;
        
        /**
         * \brief Color of the edges.
         */
        Color color;
        
        /**
         * \brief width of the edges.
         */
        index_t width;
    };

    /**
     * \brief Writes an EdgeStyle to a stream.
     * \param[in,out] out the stream
     * \param[in] es a const reference to the EdgeStyle
     * \return the new state of the stream after writing
     */
    SCENE_GRAPH_API std::ostream& operator<<(
        std::ostream& out, const EdgeStyle& es
    );

    /**
     * \brief Reads an EdgeStyle from a stream.
     * \param[in,out] in the stream
     * \param[out] es a reference to the EdgeStyle
     * \return the new state of the stream after reading
     */
    SCENE_GRAPH_API std::istream& operator>>(
        std::istream& in, EdgeStyle& es
    );

    //________________________________________________________

    /**
     * \brief Drawing style for polygons.
     */
    struct SurfaceStyle {
        /**
         * \brief SurfaceStyle constructor.
         * \details Initializes the SurfaceStyle with
         *  default values.
         */
        SurfaceStyle() :
            visible(false),
            color(0.0,0.0,0.0,1.0) {
        }
        /**
         * \brief true if polygons are visible, false otherwise.
         */
        bool visible;
        
        /**
         * \brief The Color used to draw the polygons.
         */
        Color color;
    };

    /**
     * \brief Writes a SurfaceStyle to a stream.
     * \param[in,out] out the stream
     * \param[in] ss a const reference to the SurfaceStyle
     * \return the new state of the stream after writing
     */
    SCENE_GRAPH_API std::ostream& operator<<(
        std::ostream& out, const SurfaceStyle& ss
    );

    /**
     * \brief Reads a SurfaceStyle from a stream.
     * \param[in,out] in the stream
     * \param[out] ss a reference to the SurfaceStyle
     * \return the new state of the stream after reading
     */
    SCENE_GRAPH_API std::istream& operator>>(
        std::istream& in, SurfaceStyle& ss
    );
    
    /***********************************************************************/

    /**
     * \brief A template class for strings that need to have a 
     *  specific type in the GOM system.
     * \details This class behaves exactly like std::string. It
     *  has a different type, so that GOM can make the difference
     *  and use a specific handler for it in the GUI. There
     *  are several subclasses for, e.g., image file names, object file names
     */
    template <class T, bool create=false> class Name : public NameBase {
    public:
        /**
         * \brief This class type.
         */
        typedef Name<T> thisclass;

        /**
         * \brief Name constructor.
         * \details Construts an uninitialized Name, with
         *  the empty string.
         */
        Name() {
        }

        /**
         * \brief Name copy constructor.
         * \param[in] rhs the Name to be copied
         */
        Name(const thisclass& rhs) : val_(rhs.val_) {
        }

        /**
         * \brief Name copy constructor from std::string.
         * \param[in] rhs the std::string to be copied
         */
        Name(const std::string& rhs) : val_(rhs) {
        }

        /**
         * \brief Name copy constructor from const char*.
         * \param[in] rhs a pointer to the C string to be copied
         */
        Name(const char* rhs) : val_(rhs) {
        }

        /**
         * \brief Assignment operator.
         * \param[in] rhs the Name to be copied
         * \return a reference to this Name after assignment
         */
        thisclass& operator=(const thisclass& rhs) {
            val_ = rhs.val_;
            return *this;
        }

        /**
         * \brief Assignment operator from std::string.
         * \param[in] rhs the std::string to be copied
         * \return a reference to this Name after assignment
         */
        thisclass& operator=(const std::string& rhs) {
            val_ = rhs;
            return *this;
        }

        /**
         * \brief Assignment operator from a C string
         * \param[in] rhs a pointer to the C string to be copied
         * \return a reference to this Name after assignment
         */
        thisclass& operator=(const char* rhs) {
            val_ = std::string(rhs);
            return *this;
        }

        /**
         * \brief Conversion to a const std::string.
         * \return a const reference to the internal string
         */
        operator const std::string&() const {
            return val_;
        }

        /**
         * \brief Conversion to a modifiable std::string.
         * \return a modifiable reference to the internal string
         */
        operator std::string&() {
            return val_;
        }

        /**
         * \brief Tests whether this Name corresponds to a given
         *  string.
         * \param[in] rhs a const reference to the string to be compared.
         * \retval true if this Name corresponds to \p rhs
         * \retval false otherwise
         */
        bool operator==(const std::string& rhs) const {
            return val_ == rhs;
        }

        /**
         * \brief Tests whether this Name differs from a given
         *  string.
         * \param[in] rhs a const reference to the string to be compared.
         * \retval true if this Name differs from \p rhs
         * \retval false otherwise
         */
        bool operator!=(const std::string& rhs) const {
            return val_ != rhs;
        }

    private:
        std::string val_;
    };

    //________________________________________________________
    
    /**
     * \brief Writes a Name to a stream.
     * \param[in,out] out a reference to the stream
     * \param[in] name a const reference to the Name
     * \return the new state of the stream after writing
     */
    template <class T, bool B> inline std::ostream& operator<< (
        std::ostream& out, const Name<T,B>& name
    ) {
        return (out << (const std::string&)(name)) ;
    }

    /**
     * \brief Reads a Name from a stream.
     * \param[in,out] in a reference to the stream
     * \param[out] name a reference to the NameProperty
     * \return the new state of the stream after reading
     */
    template <class T, bool B> std::istream& operator>> (
        std::istream& in, Name<T,B>& name
    ) {
        // Note: we use "std::getline(in, name)" instead of "in >> name" since
        // "in >> name" would truncate the string if it contains whitespace
        // (and under Windows, "My documents" directory contains a
        // whitespace !)
         
        std::getline(in, (std::string&)name) ;
        return in ;
    }
    
    //________________________________________________________

    /**
     * \brief Just a placeholder template to create new 
     *  Name<> types for file names.
     */
    template <class T> class File {
    public:
    };

    /**
     * \brief The name of an existing file.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a textfield and 
     *  a button that opens a FileSelectionBox configured for selecting
     *  an existing file.
     */
    typedef Name<File<Memory::byte> > FileName;

    /**
     * \brief The name of a new file.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a textfield and 
     *  a button that opens a FileSelectionBox configured for selecting
     *  an (existing or not) file.
     */
    typedef Name<File<Memory::byte>,true> NewFileName;
    
    //________________________________________________________

    /**
     * \brief The name of an existing file that contains an Image.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a textfield and 
     *  a button that opens a FileSelectionBox configured for selecting
     *  an existing image file. The list of valid extensions for an 
     *  image file is obtained from the ImageLibrary.
     */
    typedef Name<File<Image*> > ImageFileName;

    /**
     * \brief The name of an (existing or not) file that contains an Image.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a textfield and 
     *  a button that opens a FileSelectionBox configured for selecting
     *  an (existing or not) image file. The list of valid extensions for an 
     *  image file is obtained from the ImageLibrary.
     */
    typedef Name<File<Image*>,true> NewImageFileName;
    
    //________________________________________________________

    class Grob;

    /**
     * \brief The name of an existing Grob in the SceneGraph.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a ComboBox
     *  with all the Grob objects found in the SceneGraph.
     */
    typedef Name<Grob*> GrobName;

    /**
     * \brief The name of an (existing or not) Grob in the SceneGraph.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a ComboBox
     *  with all the Grob objects found in the SceneGraph. In additon,
     *  the field can be edited, and the user can enter in it a new
     *  name, not already present in the SceneGraph.
     */
    typedef Name<Grob*,true> NewGrobName;

    //________________________________________________________

    /**
     * \brief The name of an existing file that contains an object.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a textfield and 
     *  a button that opens a FileSelectionBox configured for selecting
     *  an existing object file. The list of valid extensions for an 
     *  image file is obtained from the SceneGraphLibrary.
     */
    typedef Name<File<Grob*> > GrobFileName;

    /**
     * \brief The name of an (existing or not) file that contains an object.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a textfield and 
     *  a button that opens a FileSelectionBox configured for selecting
     *  an (existing or not) object file. The list of valid extensions for an 
     *  image file is obtained from the SceneGraphLibrary.
     */
    typedef Name<File<Grob*>,true> NewGrobFileName;    
    
    //________________________________________________________

    class GrobClass;

    /**
     * \brief The name of a Grob class.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a ComboBox
     *  with a list of existing Grob class names.
     *  The list of Grob class names is obtained from the SceneGraphLibrary.
     */
    typedef Name<GrobClass*> GrobClassName;

    //_______________________________________________________

    class Colormap;

    /**
     * \brief The name of a colormap.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a ComboBox
     *  with a list of colormap names, obtained as the files in the
     *  "lib/colormaps" subdirectory.
     */
    typedef Name<Colormap*> ColormapName;
    
    //________________________________________________________

    /**
     * \brief The style used to display colormapped colors.
     */
    struct ColormapStyle {

        /**
         * \brief ColormapStyle constructor.
         */
        ColormapStyle() :
	   colormap_name("rainbow"),
	   smooth(true),
	   repeat(0),
	   show(false),
	   flip(false) {
        }
        
        /**
         * \brief Name of the colormap file, in "lib/colormaps"
         */
        ColormapName colormap_name;
        
        /**
         * \brief If true, then colormap is smoothed (using linear
         *  interpolation and mipmaps, else GL_NEAREST is used.
         */
        bool smooth;
        
        /**
         * \brief Number of times the colormap should be repeated 
         *  within the [0,1] range of color indices. Zero or 1 both
         *  mean no repetition.
         */
        index_t repeat;

        /**
         * \brief If true, show the colormap and mapped minimum and
         *  maximum value on the top left corner of the rendering
         *  window.
         */
        bool show;

	/**
	 * \brief If true, flip the colormap.
	 */
	bool flip;
    };


    /**
     * \brief Writes a ColormapStyle to a stream.
     * \param[in,out] out the stream
     * \param[in] cms a const reference to the ColormapStyle
     * \return the new state of the stream after writing
     */
    SCENE_GRAPH_API std::ostream& operator<<(
        std::ostream& out, const ColormapStyle& cms
    );

    /**
     * \brief Reads a ColormapStyle from a stream.
     * \param[in,out] in the stream
     * \param[out] cms a reference to the ColormapStyle
     * \return the new state of the stream after reading
     */
    SCENE_GRAPH_API std::istream& operator>>(
        std::istream& in, ColormapStyle& cms
    );
    
    //________________________________________________________

    class FullScreenEffect;

    /**
     * \brief The name of a full screen effect.
     * \details This class can be used as a std::string. The only
     *  difference is that when it is used as a command argument or
     *  a Shader property, AutoGUI will generate for it a ComboBox
     *  with a list of full screen effect names.
     */
    typedef Name<FullScreenEffect*> FullScreenEffectName;
    
}

#endif
