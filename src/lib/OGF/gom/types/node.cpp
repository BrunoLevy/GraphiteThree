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


#include <OGF/gom/types/node.h>
#include <OGF/gom/reflection/meta_class.h>

namespace OGF {

//_________________________________________________________

    Node::Node(Node* parent) : Object() {
        parent_ = parent ;
        if(parent_ != nullptr) {
            parent_->add_child(this) ;
        }
    }

    Node::~Node() {
    }

    void Node::add_child(Node* child) {
        children_.push_back(child) ;
	child->parent_ = this;
    }

    void Node::remove_child(Node* child) {
        for(auto it=children_.begin(); it!=children_.end(); ++it) {
            if(*it == child) {
                children_.erase(it) ;
                return ;
            }
        }
        bool found = false ;
        ogf_assert(found) ;
    }

//___________________________________________________________________________

}
