#include "cafTristate.h"

#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::Tristate::Tristate()
    : m_state( State::False )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::Tristate::operator=( const State& state )
{
    m_state = state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::Tristate::operator==( const Tristate& other ) const
{
    return m_state == other.m_state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::Tristate::operator==( State state ) const
{
    return m_state == state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::Tristate::operator!=( const Tristate& other ) const
{
    return !( m_state == other.m_state );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::Tristate::State caf::Tristate::state() const
{
    return m_state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::Tristate::isTrue() const
{
    return m_state == State::True;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::Tristate::isPartiallyTrue() const
{
    return m_state == State::PartiallyTrue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::Tristate::isFalse() const
{
    return m_state == State::False;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString caf::Tristate::text() const
{
    QString txt;

    switch ( m_state )
    {
        case Tristate::State::False:
            txt = "False";
            break;
        case Tristate::State::PartiallyTrue:
            txt = "PartiallyTrue";
            break;
        case Tristate::State::True:
            txt = "True";
            break;
        default:
            break;
    }

    return txt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::Tristate::setFromText( const QString& valueText )
{
    QString lowerCase = valueText.toLower();

    if ( lowerCase == "false" )
    {
        m_state = State::False;
    }
    else if ( lowerCase == "partiallytrue" )
    {
        m_state = State::PartiallyTrue;
    }
    else if ( lowerCase == "true" )
    {
        m_state = State::True;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTextStream& operator>>( QTextStream& str, caf::Tristate& triplet )
{
    QString text;
    str >> text;
    triplet.setFromText( text );

    return str;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTextStream& operator<<( QTextStream& str, const caf::Tristate& triplet )
{
    str << triplet.text();

    return str;
}
