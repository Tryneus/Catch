/*
 *  catch_resultinfo.hpp
 *  Catch
 *
 *  Created by Phil on 28/10/2010.
 *  Copyright 2010 Two Blue Cubes Ltd. All rights reserved.
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef TWOBLUECUBES_CATCH_RESULT_INFO_HPP_INCLUDED
#define TWOBLUECUBES_CATCH_RESULT_INFO_HPP_INCLUDED

#include <string>
#include "catch_result_type.h"

namespace Catch
{    
    class ResultInfo
    {
    public:
        
        ///////////////////////////////////////////////////////////////////////////
        ResultInfo
        ()
        :   m_line( 0 ),
            m_result( ResultWas::Unknown ),
            m_isNot( false )
        {}
        
        ///////////////////////////////////////////////////////////////////////////
        ResultInfo
        (
            const char* expr, 
            ResultWas::OfType result, 
            bool isNot, 
            const char* filename, 
            std::size_t line, 
            const char* macroName 
        )
        :   m_macroName( macroName ),
            m_filename( filename ),
            m_line( line ),
            m_expr( expr ),
            m_result( result ),
            m_isNot( isNot )
        {
            if( isNot )
                m_expr = "!" + m_expr;
        }
        
        ///////////////////////////////////////////////////////////////////////////
        bool ok
        ()
        const
        {
            return ( m_result & ResultWas::FailureBit ) != ResultWas::FailureBit;
        }
        
        ///////////////////////////////////////////////////////////////////////////
        ResultWas::OfType getResultType
        ()
        const
        {
            return m_result;
        }
        
        ///////////////////////////////////////////////////////////////////////////
        bool hasExpression
        ()
        const
        {
            return !m_expr.empty();
        }

        ///////////////////////////////////////////////////////////////////////////
        bool hasMessage
        ()
        const
        {
            return !m_message.empty();
        }

        ///////////////////////////////////////////////////////////////////////////
        std::string getExpression
        ()
        const
        {
            return m_expr;
        }

        ///////////////////////////////////////////////////////////////////////////
        std::string getExpandedExpression
        ()
        const
        {
            if( !hasExpression() )
                return "";

            return m_evaluatedExpr.empty() ? m_expr : m_evaluatedExpr;
        }
        
        ///////////////////////////////////////////////////////////////////////////
        std::string getMessage
        ()
        const
        {
            return m_message;
        }
        
        ///////////////////////////////////////////////////////////////////////////
        std::string getFilename
        ()
        const
        {
            return m_filename;
        }
        
        ///////////////////////////////////////////////////////////////////////////
        std::size_t getLine
        ()
        const
        {
            return m_line;
        }
        
        ///////////////////////////////////////////////////////////////////////////
        std::string getTestMacroName
        ()
        const
        {
            return m_macroName;
        }

        ///////////////////////////////////////////////////////////////////////////
        void setMessage
        (
            const std::string& message
        )
        {
            m_message = message;
        }

        ///////////////////////////////////////////////////////////////////////////
        void setResultType
        (
            ResultWas::OfType result
        )
        {
            // Flip bool results if isNot is set
            if( m_isNot && result == ResultWas::Ok )
                m_result = ResultWas::ExpressionFailed;
            else if( m_isNot && result == ResultWas::ExpressionFailed )
                m_result = ResultWas::Ok;
            else
                m_result = result;
        }

    protected:

        ///////////////////////////////////////////////////////////////////////////
        void setExpressionString
        (
            std::string exprString
        )
        {
            m_evaluatedExpr = exprString;
        }

        ///////////////////////////////////////////////////////////////////////////
        void setResultData
        (
            bool resultData
        )
        {
            setResultType(resultData ? ResultWas::Ok : ResultWas::ExpressionFailed); 
        }
        
    private:
        std::string m_macroName;
        std::string m_filename;
        std::size_t m_line;
        std::string m_expr;
        std::string m_message;
        ResultWas::OfType m_result;
        bool m_isNot;
        std::string m_evaluatedExpr;
    };
    
} // end namespace Catch


#endif // TWOBLUECUBES_CATCH_RESULT_INFO_HPP_INCLUDED
