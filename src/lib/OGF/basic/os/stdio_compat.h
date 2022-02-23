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
 
#ifndef H_OGF_BASIC_IO_STDIO_COMPAT_H
#define H_OGF_BASIC_IO_STDIO_COMPAT_H

#include <OGF/basic/common/common.h>

#include <iostream>
#include <stdio.h>

/**
 * \file OGF/basic/os/stdio_compat.h
 * \brief Functions to facilitate porting old C code,
 *  that provide an stdio-like API acting on C++ streams.
 */

namespace OGF {

//____________________________________________________________________________

    /**
     * \brief Reads elements from a stream
     * \param[in] ptr_in where to load the elements. Should point to
     *  a block of \p nbelt * \p size bytes of memory.
     * \param[in] size size of an element to read
     * \param[in] nbelt number of elements to read
     * \param[in] in a pointer to a stream where to read the elements
     * \return the number of elements successfully read
     */
    inline size_t fread(
        void* ptr_in, size_t size, size_t nbelt, std::istream* in
    ) {
        char* ptr = (char *)ptr_in ;
        size_t j ;
        for(j=0; j<nbelt; j++) {
            for(size_t i=0; i<size; i++) {
                if(in->eof()) {
                    return j ;
                }
                in->get(*ptr) ;
                ptr++ ;
            }
        }
        return j ;
    } 

    /**
     * \brief reads a character from a stream.
     * \param[in] in a pointer to a stream where to read the elements
     * \return the read characted, as an int
     */
    inline int fgetc(std::istream* in) {
        char result ;
        in->get(result) ;
        return int(result) ;
    }

    
    /**
     * \brief moves the current position within a stream
     * \param[in] in a pointer to the stream
     * \param[in] offset offset where to move, interpreted
     *  differently in function of \p whence
     * \param[in] whence SEEK_CUR only is supported
     *  - SEEK_CUR: \p offset is relative to current position
     * \return 0 on success
     */
    inline int fseek(
        std::istream* in, long offset, int whence
    ) {
        if(whence == SEEK_CUR) {
            offset += in->tellg() ;
        }
        ogf_assert(whence != SEEK_END) ;
        in->seekg(offset) ;
        return 0 ;
    }

    /**
     * \brief gets the current position in a stream.
     * \param[in] in a pointer to the stream
     * \return the current position in the stream
     */
    inline size_t ftell(std::istream* in) {
        return size_t(in->tellg()) ;
    }

    /**
     * \brief closes a stream.
     * \details Does nothing, exists for source-level compatibility
     *  of some legacy code. 
     * \param[in] in a pointer to the stream to be closed
     */
    inline void fclose(std::istream* in) {
        ogf_argused(in);
    }


//____________________________________________________________________________

}

#endif
