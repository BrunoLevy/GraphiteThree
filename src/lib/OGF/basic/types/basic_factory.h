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
 

#ifndef H_OGF_BASIC_TYPES_BASIC_FACTORY_H
#define H_OGF_BASIC_TYPES_BASIC_FACTORY_H

#include <OGF/basic/common/common.h>

#include <map>
#include <vector>
#include <string>

/**
 * \file OGF/basic/types/basic_factory.h
 * \brief Base classes for factories.
 */
namespace OGF {

//______________________________________________________________________________

    template<class BASE> class BasicFactory : public Counted {
    public:
        virtual BASE* create() = 0 ;
    } ;

    template <class BASE, class T> 
    class GenericBasicFactory : public BasicFactory<BASE> {
    public:
    virtual BASE* create() { return new T() ; }
    } ;
    
    template <class T> class BasicFactories {
    private:
    typedef std::map<std::string, SmartPointer<BasicFactory<T> > > FactoriesMap ;
    public:
        void register_factory(const std::string& name, BasicFactory<T>* f) {
            ogf_assert(map_.find(name) == map_.end()) ;
            map_[name] = f ;
        }
        void unregister_factory(const std::string& name) {
            auto it = map_.find(name) ;
            ogf_assert(it != map_.end()) ;
            map_.erase(it) ;
        }
        bool factory_is_bound(const std::string& name) const {
            return(map_.find(name) != map_.end()) ;
        }
        void list_factory_names(std::vector<std::string>& names) const {
            names.clear() ;
            for(auto& it : map_) {
                names.push_back(it.first) ;
            }
        }
        std::string factory_names() const {
            std::string result = "";
            for(auto& it : map_) {
                if(result.length() != 0) {
                    result += ";" ;
                }
                result += it.first ;
            }
            return result ;
        }
        T* create(const std::string& name) {
            auto it = map_.find(name) ;
            ogf_assert(it != map_.end()) ;
            return it->second->create() ;
        }
    private:
        FactoriesMap map_ ;
    } ;

//_____________________________________________________________________________________

    template<class BASE, class ARG> class BasicFactoryWithArg : public Counted {
    public:
        virtual BASE* create(ARG x) = 0 ;
    } ;

    template <class BASE, class T, class ARG> 
        class GenericBasicFactoryWithArg : public BasicFactoryWithArg<BASE,ARG> {
    public:
    virtual BASE* create(ARG x) { return new T(x) ; }
    } ;
    
    template <class T, class ARG> class BasicFactoriesWithArg {
    private:
    typedef std::map<std::string, SmartPointer<BasicFactoryWithArg<T,ARG> > > FactoriesMap ;
    public:
        void register_factory(const std::string& name, BasicFactoryWithArg<T,ARG>* f) {
            ogf_assert(map_.find(name) == map_.end()) ;
            map_[name] = f ;
        }
        void unregister_factory(const std::string& name) {
            auto it = map_.find(name) ;
            ogf_assert(it != map_.end()) ;
            map_.erase(it) ;
        }
        bool factory_is_bound(const std::string& name) const {
            return(map_.find(name) != map_.end()) ;
        }
        void list_factory_names(std::vector<std::string>& names) const {
            names.clear() ;
            for(auto& it : map_) {
                names.push_back(it.first) ;
            }
        }
        std::string factory_names() const {
            std::string result = "";
            for(auto& it : map_) {
                if(result.length() != 0) {
                    result += ";" ;
                }
                result += it.first ;
            }
            return result ;
        }
        T* create(const std::string& name, ARG x) {
            auto it = map_.find(name) ;
            ogf_assert(it != map_.end()) ;
            return it->second->create(x) ;
        }
    private:
        FactoriesMap map_ ;
    } ;

//_____________________________________________________________________________________

}

#endif
