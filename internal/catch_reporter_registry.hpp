/*
 *  catch_reporter_registry.hpp
 *  Catch
 *
 *  Created by Phil on 29/10/2010.
 *  Copyright 2010 Two Blue Cubes Ltd. All rights reserved.
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef TWOBLUECUBES_CATCH_REPORTER_REGISTRY_HPP_INCLUDED
#define TWOBLUECUBES_CATCH_REPORTER_REGISTRY_HPP_INCLUDED

#include "catch_interfaces_reporter.h"

#include <map>

namespace Catch
{
    class ReporterRegistry : public IReporterRegistry
    {
    public:
        
        ///////////////////////////////////////////////////////////////////////
        ~ReporterRegistry
        ()
        {
            deleteAllValues( m_factories );
        }
        
        ///////////////////////////////////////////////////////////////////////
        virtual IReporter* create
        (
            const std::string& name, 
            const IReporterConfig& config 
        )
        const
        {
            FactoryMap::const_iterator it =  m_factories.find( name );
            if( it == m_factories.end() )
                return NULL;
            return it->second->create( config );
        }
        
        ///////////////////////////////////////////////////////////////////////
        void registerReporter
        (
            const std::string& name, 
            IReporterFactory* factory 
        )
        {
            m_factories.insert( std::make_pair( name, factory ) );
        }        

        ///////////////////////////////////////////////////////////////////////
        const FactoryMap& getFactories
        ()
        const
        {
            return m_factories;
        }

    private:
        FactoryMap m_factories;
    };
}

#endif // TWOBLUECUBES_CATCH_REPORTER_REGISTRY_HPP_INCLUDED