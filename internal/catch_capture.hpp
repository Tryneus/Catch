/*
 *  catch_capture.hpp
 *  Catch
 *
 *  Created by Phil on 18/10/2010.
 *  Copyright 2010 Two Blue Cubes Ltd. All rights reserved.
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef TWOBLUECUBES_CATCH_CAPTURE_HPP_INCLUDED
#define TWOBLUECUBES_CATCH_CAPTURE_HPP_INCLUDED

#include "catch_resultinfo.hpp"
#include "catch_result_type.h"
#include "catch_interfaces_capture.h"
#include "catch_debugger.hpp"
#include <sstream>
#include <cmath>

namespace Catch
{
namespace Detail
{
    // The following code, contributed by Sam Partington, allows us to choose an implementation
    // of toString() depending on whether a << overload is available
    
    struct NonStreamable
    {
        // allow construction from anything...
        template<typename Anything> 
        NonStreamable(Anything)
        {}
    };
    
    // a local operator<<  which may be called if there isn't a better one elsewhere...
    inline NonStreamable operator << ( std::ostream&, const NonStreamable& ns )
    {
        return ns;
    }

    template<typename T>
    struct IsStreamable
    {
        static NoType Deduce( const NonStreamable& );
        static YesType Deduce( std::ostream& );

        enum
        {
            value = sizeof( Deduce( Synth<std::ostream&>() << Synth<const T&>() ) ) 
                        == sizeof( YesType )
        };
    };
    
    // << is available, so use it with ostringstream to make the string
    template<typename T, bool streamable>
    struct StringMaker
    {
        ///////////////////////////////////////////////////////////////////////
        static std::string apply
        (
            const T& value
        )
        {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }
    };
      
    // << not available - use a default string
    template<typename T>
    struct StringMaker<T, false>
    {
        ///////////////////////////////////////////////////////////////////////
        static std::string apply
        (
            const T&
        )
        {
            return "{?}";
        }
    };

}// end namespace Detail

///////////////////////////////////////////////////////////////////////////////
template<typename T>
std::string toString
(
    const T& value
)
{
    return Detail::StringMaker<T, Detail::IsStreamable<T>::value>::apply( value );
}
    
// Shortcut overloads

///////////////////////////////////////////////////////////////////////////////
inline std::string toString
(
    const std::string& value
)
{
    return value;
}

///////////////////////////////////////////////////////////////////////////////
inline std::string toString
(
    const char* value
)
{
    return value;
}    

///////////////////////////////////////////////////////////////////////////////
inline std::string toString
(
    int value
)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////
inline std::string toString
(
    const double value 
)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}    
    
class TestFailureException
{
};
class DummyExceptionType_DontUse
{
};
    
class ScopedInfo
{
public:
    ///////////////////////////////////////////////////////////////////////////
    ScopedInfo
    ()
    {
        Hub::getResultCapture().pushScopedInfo( this );
    }
    
    ///////////////////////////////////////////////////////////////////////////
    ~ScopedInfo
    ()
    {
        Hub::getResultCapture().popScopedInfo( this );
    }
    
    ///////////////////////////////////////////////////////////////////////////
    ScopedInfo& operator << 
    (
        const char* str
    )
    {
        m_oss << str;
        return *this; 
    }
    
    ///////////////////////////////////////////////////////////////////////////
    std::string getInfo
    ()
    const
    {
        return m_oss.str();
    }
    
private:
    std::ostringstream m_oss;
};
    
    
// !TBD Need to clean this all up
#define CATCH_absTol 1e-10
#define CATCH_relTol 1e-10

inline double catch_max( double x, double y )
{
    return x > y ? x : y;
}
    
class Approx
{
public:
    ///////////////////////////////////////////////////////////////////////////
    // !TBD more generic
    Approx
    (
        double d
    )
    : m_d( d )
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template<typename T>
    friend bool operator == 
    (
        const T& lhs, 
        const Approx& rhs
    )
    {
        // !TBD Use proper tolerance
        // From: http://realtimecollisiondetection.net/blog/?p=89
        // see also: http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
        return fabs( lhs - rhs.m_d ) <= catch_max( CATCH_absTol, CATCH_relTol * catch_max( fabs(lhs), fabs(rhs.m_d) ) );
    }
    
    ///////////////////////////////////////////////////////////////////////////
    template<typename T>
    friend bool operator != 
    (
        const T& lhs, 
        const Approx& rhs
    )
    {
        return ! operator==( lhs, rhs );
    }
    
    double m_d;
};

///////////////////////////////////////////////////////////////////////////////
template<>
inline std::string toString<Approx>
(
    const Approx& value
)
{
    std::ostringstream oss;
    oss << "Approx( " << value.m_d << ")";
    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////
// This is just here to avoid compiler warnings with macro constants
inline bool isTrue
(
    bool value
)
{
    return value;
}

// Predeclarations for references in ChunkEvaluator and ReverseChunkEvaluator
class ResultBuilder;
class ReverseResultBuilder;

template<typename OperandT>
class ReverseChunkEvaluator
{
public:
    ///////////////////////////////////////////////////////////////////////////
    ReverseChunkEvaluator
    (
        const ReverseResultBuilder& parent,
        const OperandT& operand,
        bool result
    );

    ///////////////////////////////////////////////////////////////////////////
    const OperandT& getOperand
    ()
    const
    {
        return m_operand;
    }

    ///////////////////////////////////////////////////////////////////////////
    ReverseResultBuilder& getParent
    ()
    const
    {
        return m_parent;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool getResult
    ()
    const
    {
        return m_result;
    }

private:
    const OperandT& m_operand;
    ReverseResultBuilder& m_parent;
    bool m_result;
};

template<typename OperandT>
class ChunkEvaluator
{
public:
    // These functions are implemented following the ResultBuilder definition
    ChunkEvaluator(ResultBuilder& parent, const OperandT& operand, bool result);

    // Terminal cases
    ResultBuilder& operator << ( const ReverseResultBuilder& rhs );

    template<typename RhsT>
    ResultBuilder& operator || ( const ReverseChunkEvaluator<RhsT>& rhs);

    template<typename RhsT>
    ResultBuilder& operator && ( const ReverseChunkEvaluator<RhsT>& rhs);

    template<typename RhsT>
    ResultBuilder& operator == ( const ReverseChunkEvaluator<RhsT>& rhs );

    template<typename RhsT>
    ResultBuilder& operator != ( const ReverseChunkEvaluator<RhsT>& rhs );

    template<typename RhsT>
    ResultBuilder& operator < ( const ReverseChunkEvaluator<RhsT>& rhs );

    template<typename RhsT>
    ResultBuilder& operator > ( const ReverseChunkEvaluator<RhsT>& rhs );

    template<typename RhsT>
    ResultBuilder& operator <= ( const ReverseChunkEvaluator<RhsT>& rhs );

    template<typename RhsT>
    ResultBuilder& operator >= ( const ReverseChunkEvaluator<RhsT>& rhs );

    // Forward expression evaluation
    template<typename RhsT>
    ChunkEvaluator<RhsT> operator || ( const RhsT& rhs );

    template<typename RhsT>
    ChunkEvaluator<RhsT> operator && ( const RhsT& rhs );

    template<typename RhsT>
    ChunkEvaluator<RhsT> operator == ( const RhsT& rhs );

    template<typename RhsT>
    ChunkEvaluator<RhsT> operator != ( const RhsT& rhs );

    template<typename RhsT>
    ChunkEvaluator<RhsT> operator < ( const RhsT& rhs );

    template<typename RhsT>
    ChunkEvaluator<RhsT> operator > ( const RhsT& rhs );

    template<typename RhsT>
    ChunkEvaluator<RhsT> operator <= ( const RhsT& rhs );

    template<typename RhsT>
    ChunkEvaluator<RhsT> operator >= ( const RhsT& rhs );

    operator ResultBuilder&();

private:
    const OperandT& m_operand;
    ResultBuilder& m_parent;
    bool m_result;
};

class ReverseResultBuilder
{
public:
    ///////////////////////////////////////////////////////////////////////////
    void append
    (
        const std::string& exprChunk
    )
    {
        m_exprString = exprChunk + m_exprString;
    }

    ///////////////////////////////////////////////////////////////////////////
    const std::string& getExprString
    ()
    const
    {
        return m_exprString;
    }

private:
    std::string m_exprString;
};

class ResultBuilder : public ResultInfo
{
public:
    ///////////////////////////////////////////////////////////////////////////
    ResultBuilder
    (
        const char* expr,
        bool isNot,
        const char* filename,
        std::size_t line,
        const char* macroName
    )
    : ResultInfo( expr, ResultWas::Unknown, isNot, filename, line, macroName ),
      m_incomplete(false)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template<typename LhsT>
    ChunkEvaluator<LhsT> operator <<
    (
        const LhsT& lhs
    )
    {
        return ChunkEvaluator<LhsT>(*this, lhs, true);
    }

    ///////////////////////////////////////////////////////////////////////////
    void setIncomplete
    ()
    {
      m_incomplete = true;
    }

    ///////////////////////////////////////////////////////////////////////////
    void setResult
    (
        bool exprResult,
        const std::string& reverseExprString
    )
    {
        setResultData(exprResult);

        if(!m_incomplete)
          setExpressionString(m_exprString + reverseExprString);
        else
          setExpressionString(m_exprString + reverseExprString + " {can't expand the rest of the expression - consider rewriting it}");
    }

    ///////////////////////////////////////////////////////////////////////////
    void append
    (
        const std::string& exprChunk
    )
    {
        m_exprString += exprChunk;
    }

private:
    std::string m_exprString;
    bool m_incomplete;
};

// Chunk evaluator function definitions are outside the class since they make callbacks to the ResultBuilders

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
ChunkEvaluator<OperandT>::ChunkEvaluator
(
ResultBuilder& parent,
 const OperandT& operand,
 bool result
) :
    m_operand(operand),
    m_parent(parent),
    m_result(result)
{
    m_parent.append( toString(operand) );
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
ReverseChunkEvaluator<OperandT>::ReverseChunkEvaluator
(
const ReverseResultBuilder& parent,
 const OperandT& operand,
 bool result
) :
    m_operand(operand),
    m_parent(const_cast<ReverseResultBuilder&>(parent)), // Don't do this at home, kids
    m_result(result)
{
    m_parent.append( toString(operand) );
}

//Terminal cases, where the chunk evaluators meet

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
ChunkEvaluator<OperandT>::operator ResultBuilder&
()
{
  m_parent.setResult(m_result, "");
  return m_parent;
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
ResultBuilder& ChunkEvaluator<OperandT>::operator <<
(
    const ReverseResultBuilder& rhs
)
{
    // rhs is unused because if we hit this operator, rhs never did anything
    m_parent.setResult(m_result, rhs.getExprString());
    return m_parent;
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ResultBuilder& ChunkEvaluator<OperandT>::operator ==
(
    const ReverseChunkEvaluator<RhsT>& rhs
)
{
    m_parent.append(" == ");
    m_parent.setResult(m_result == rhs.getResult(), rhs.getParent().getExprString());
    return m_parent;
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ResultBuilder& ChunkEvaluator<OperandT>::operator !=
(
    const ReverseChunkEvaluator<RhsT>& rhs
)
{
    m_parent.append(" != ");
    m_parent.setResult(m_result != rhs.getResult(), rhs.getParent().getExprString());
    return m_parent;
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ResultBuilder& ChunkEvaluator<OperandT>::operator <
(
    const ReverseChunkEvaluator<RhsT>& rhs
)
{
    m_parent.append(" < ");
    m_parent.setResult(m_result < rhs.getResult(), rhs.getParent().getExprString());
    return m_parent;
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ResultBuilder& ChunkEvaluator<OperandT>::operator >
(
    const ReverseChunkEvaluator<RhsT>& rhs
)
{
    m_parent.append(" > ");
    m_parent.setResult(m_result > rhs.getResult(), rhs.getParent().getExprString());
    return m_parent;
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ResultBuilder& ChunkEvaluator<OperandT>::operator <=
(
    const ReverseChunkEvaluator<RhsT>& rhs
)
{
    m_parent.append(" <= ");
    m_parent.setResult(m_result <= rhs.getResult(), rhs.getParent().getExprString());
    return m_parent;
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ResultBuilder& ChunkEvaluator<OperandT>::operator >=
(
    const ReverseChunkEvaluator<RhsT>& rhs
)
{
    m_parent.append(" >= ");
    m_parent.setResult(m_result >= rhs.getResult(), rhs.getParent().getExprString());
    return m_parent;
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ResultBuilder& ChunkEvaluator<OperandT>::operator ||
(
    const ReverseChunkEvaluator<RhsT>& rhs
)
{
    m_parent.append(" || ");
    m_parent.setResult(m_result || rhs.getResult(), rhs.getParent().getExprString());
    return m_parent;
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ResultBuilder& ChunkEvaluator<OperandT>::operator &&
(
    const ReverseChunkEvaluator<RhsT>& rhs
)
{
    m_parent.append(" && ");
    m_parent.setResult(m_result && rhs.getResult(), rhs.getParent().getExprString());
    return m_parent;
}

// Operators for consuming from the left (forward chunk evaluation)

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ChunkEvaluator<RhsT> ChunkEvaluator<OperandT>::operator ||
(
    const RhsT& rhs
)
{
    m_parent.append(" || ");
    m_parent.setIncomplete();
    return ChunkEvaluator<RhsT>(m_parent, rhs, m_result || rhs);
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ChunkEvaluator<RhsT> ChunkEvaluator<OperandT>::operator &&
(
    const RhsT& rhs
)
{
    m_parent.append(" && ");
    m_parent.setIncomplete();
    return ChunkEvaluator<RhsT>(m_parent, rhs, m_result && rhs);
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ChunkEvaluator<RhsT> ChunkEvaluator<OperandT>::operator ==
(
    const RhsT& rhs
)
{
    m_parent.append(" == ");
    return ChunkEvaluator<RhsT>(m_parent, rhs, m_result && (m_operand == rhs));
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ChunkEvaluator<RhsT> ChunkEvaluator<OperandT>::operator !=
(
    const RhsT& rhs
)
{
    m_parent.append(" != ");
    return ChunkEvaluator<RhsT>(m_parent, rhs, m_result && (m_operand != rhs));
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ChunkEvaluator<RhsT> ChunkEvaluator<OperandT>::operator <
(
    const RhsT& rhs
)
{
    m_parent.append(" < ");
    return ChunkEvaluator<RhsT>(m_parent, rhs, m_result && (m_operand < rhs));
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ChunkEvaluator<RhsT> ChunkEvaluator<OperandT>::operator >
(
    const RhsT& rhs
)
{
    m_parent.append(" > ");
    return ChunkEvaluator<RhsT>(m_parent, rhs, m_result && (m_operand > rhs));
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ChunkEvaluator<RhsT> ChunkEvaluator<OperandT>::operator <=
(
    const RhsT& rhs
)
{
    m_parent.append(" <= ");
    return ChunkEvaluator<RhsT>(m_parent, rhs, m_result && (m_operand <= rhs));
}

///////////////////////////////////////////////////////////////////////////////
template<typename OperandT>
template<typename RhsT>
ChunkEvaluator<RhsT> ChunkEvaluator<OperandT>::operator >=
(
    const RhsT& rhs
)
{
    m_parent.append(" >= ");
    return ChunkEvaluator<RhsT>(m_parent, rhs, m_result && (m_operand >= rhs));
}

// Operators for consuming from the right (reverse chunk evaluaton)
// Initial ChunkEvaluator creation

///////////////////////////////////////////////////////////////////////////////
template<typename RhsT>
ReverseChunkEvaluator<RhsT> operator <<
(
    const RhsT& rhs,
    const ReverseResultBuilder& rev
)
{
    return ReverseChunkEvaluator<RhsT>(rev, rhs, true);
}

///////////////////////////////////////////////////////////////////////////////
template<typename LhsT, typename RhsT>
ReverseChunkEvaluator<LhsT> operator ==
(
    const LhsT& lhs,
    const ReverseChunkEvaluator<RhsT>& rhs
)
{
    rhs.getParent().append(" == ");
    return ReverseChunkEvaluator<RhsT>(rhs.getParent(), lhs, rhs.getResult() && (lhs == rhs.getOperand()));
}

///////////////////////////////////////////////////////////////////////////////
template<typename LhsT, typename RhsT>
ReverseChunkEvaluator<LhsT> operator !=
(
    const LhsT& lhs,
    const ReverseChunkEvaluator<RhsT>& rhs
)
{
    rhs.getParent().append(" != ");
    return ReverseChunkEvaluator<RhsT>(rhs.getParent(), lhs, rhs.getResult() && (lhs != rhs.getOperand()));
}

///////////////////////////////////////////////////////////////////////////////
template<typename LhsT, typename RhsT>
ReverseChunkEvaluator<LhsT> operator <
(
    const LhsT& lhs,
    const ReverseChunkEvaluator<RhsT>& rhs
)
{
    rhs.getParent().append(" < ");
    return ReverseChunkEvaluator<RhsT>(rhs.getParent(), lhs, rhs.getResult() && (lhs < rhs.getOperand()));
}

///////////////////////////////////////////////////////////////////////////////
template<typename LhsT, typename RhsT>
ReverseChunkEvaluator<LhsT> operator >
(
    const LhsT& lhs,
    const ReverseChunkEvaluator<RhsT>& rhs
)
{
    rhs.getParent().append(" > ");
    return ReverseChunkEvaluator<RhsT>(rhs.getParent(), lhs, rhs.getResult() && (lhs > rhs.getOperand()));
}

///////////////////////////////////////////////////////////////////////////////
template<typename LhsT, typename RhsT>
ReverseChunkEvaluator<LhsT> operator <=
(
    const LhsT& lhs,
    const ReverseChunkEvaluator<RhsT>& rhs
)
{
    rhs.getParent().append(" <= ");
    return ReverseChunkEvaluator<RhsT>(rhs.getParent(), lhs, rhs.getResult() && (lhs <= rhs.getOperand()));
}

///////////////////////////////////////////////////////////////////////////////
template<typename LhsT, typename RhsT>
ReverseChunkEvaluator<LhsT> operator >=
(
    const LhsT& lhs,
    const ReverseChunkEvaluator<RhsT>& rhs
)
{
    rhs.getParent().append(" >= ");
    return ReverseChunkEvaluator<LhsT>(rhs.getParent(), lhs, rhs.getResult() && (lhs >= rhs.getOperand()));
}
    
} // end namespace Catch

///////////////////////////////////////////////////////////////////////////////
#define INTERNAL_CATCH_ACCEPT_RESULT( result, stopOnFailure ) \
    if( Catch::ResultAction::Value action = Catch::Hub::getResultCapture().acceptResult( result )  ) \
    { \
        if( action == Catch::ResultAction::DebugFailed ) DebugBreak(); \
        if( Catch::isTrue( stopOnFailure ) ) throw Catch::TestFailureException(); \
    }

///////////////////////////////////////////////////////////////////////////////
#define INTERNAL_CATCH_TEST( expr, isNot, stopOnFailure, macroName ) \
    { \
        Catch::Hub::getResultCapture().acceptExpression( Catch::ResultBuilder( #expr, isNot, __FILE__, __LINE__, macroName ) << expr << Catch::ReverseResultBuilder()); \
        INTERNAL_CATCH_ACCEPT_RESULT( Catch::Hub::getResultCapture().getCurrentResult().getResultType() , stopOnFailure ) \
    }

///////////////////////////////////////////////////////////////////////////////
#define INTERNAL_CATCH_THROWS( expr, exceptionType, nothrow, stopOnFailure, macroName ) \
    Catch::Hub::getResultCapture().acceptExpression( Catch::ResultBuilder( #expr, false, __FILE__, __LINE__, macroName ) ); \
    try \
    { \
        expr; \
        INTERNAL_CATCH_ACCEPT_RESULT( nothrow, stopOnFailure ) \
    } \
    catch( exceptionType ) \
    { \
        INTERNAL_CATCH_ACCEPT_RESULT( !(nothrow), stopOnFailure ) \
    }

///////////////////////////////////////////////////////////////////////////////
#define INTERNAL_CATCH_THROWS_AS( expr, exceptionType, nothrow, stopOnFailure, macroName ) \
INTERNAL_CATCH_THROWS( expr, exceptionType, nothrow, stopOnFailure, macroName ) \
catch( ... ) \
{ \
    INTERNAL_CATCH_ACCEPT_RESULT( false, stopOnFailure ) \
}

///////////////////////////////////////////////////////////////////////////////
#define INTERNAL_CATCH_MSG( reason, resultType, stopOnFailure, macroName ) \
    { \
        std::ostringstream INTERNAL_CATCH_UNIQUE_NAME( strm ); \
        INTERNAL_CATCH_UNIQUE_NAME( strm ) << reason; \
        Catch::Hub::getResultCapture().acceptExpression( Catch::MutableResultInfo( "", false, __FILE__, __LINE__, macroName ) ); \
        Catch::Hub::getResultCapture().acceptMessage( INTERNAL_CATCH_UNIQUE_NAME( strm ).str() ); \
        INTERNAL_CATCH_ACCEPT_RESULT( resultType, stopOnFailure ) \
    }

///////////////////////////////////////////////////////////////////////////////
#define INTERNAL_CATCH_SCOPED_INFO( log ) Catch::ScopedInfo INTERNAL_CATCH_UNIQUE_NAME( info ); INTERNAL_CATCH_UNIQUE_NAME( info ) << log

#endif // TWOBLUECUBES_CATCH_CAPTURE_HPP_INCLUDED
