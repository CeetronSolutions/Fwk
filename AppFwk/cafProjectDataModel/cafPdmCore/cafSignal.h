//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2020- Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################
#pragma once

#include "cafAssert.h"

#include <functional>
#include <map>
#include <string>
#include <type_traits>

namespace caf
{
class SignalEmitter;
class SignalObserver;

//==================================================================================================
/// Basic delete signal emitted by any observer when deleted.
/// Allows a regular signal to disconnect from the observer.
//==================================================================================================
class DeleteSignal
{
public:
    using DisconnectCallback = std::function<void( SignalObserver* )>;

    DeleteSignal( SignalObserver* observer );

    template <typename ClassType>
    void connect( ClassType* signal )
    {
        static_assert( std::is_convertible<ClassType*, SignalObserver*>::value,
                       "Only classes that inherit SignalObserver can connect as an observer of a Signal." );

        DisconnectCallback disconnectCallback = [=]( SignalObserver* observer ) { ( signal->disconnect )( observer ); };
        auto               signalCasted       = dynamic_cast<SignalObserver*>( signal );

        if ( signalCasted ) m_disconnectCallbacks[signalCasted] = disconnectCallback;
    }
    void disconnect( SignalObserver* observer );
    void send();

private:
    std::map<SignalObserver*, DisconnectCallback> m_disconnectCallbacks;

    SignalObserver* m_observer;
};

//==================================================================================================
/// SignalEmitter
/// Should be inherited by any object that emits signals.
//==================================================================================================
class SignalEmitter
{
public:
    virtual ~SignalEmitter() = default;
};

//==================================================================================================
/// SignalObserver.
/// Should be inherited by any object that observes and reacts to signals.
//==================================================================================================
class SignalObserver
{
public:
    DeleteSignal beingDeleted;

public:
    SignalObserver();
    void sendDeleteSignal();
    virtual ~SignalObserver();

}; // namespace caf

//==================================================================================================
/// General signal class.
/// Connect any member function with the signature void(const Signal*, const SignalData* data)
/// Connect with .connect(this, &Class::nameOfMethod)
/// The method should accept that data may be nullptr
//==================================================================================================
template <typename... Args>
class Signal : public SignalObserver
{
public:
    using MemberCallback              = std::function<void( const SignalEmitter*, Args... args )>;
    using MemberCallbackAndActiveFlag = std::pair<MemberCallback, bool>;

public:
    Signal( const SignalEmitter* emitter )
        : m_emitter( emitter )
    {
    }

    Signal( const Signal& rhs )
        : m_emitter( rhs.m_emitter )
    {
        for ( auto observerCallbackPair : rhs.m_observerCallbacks )
        {
            connect( observerCallbackPair.first, observerCallbackPair.second.first );
        }
    }

    Signal& operator=( const Signal& rhs )
    {
        m_emitter = rhs.m_emitter;
        for ( auto observerCallbackPair : rhs.m_observerCallbacks )
        {
            connect( observerCallbackPair.first, observerCallbackPair.second.first );
        }
        return *this;
    }

    virtual ~Signal()
    {
        for ( auto observerCallbackPair : m_observerCallbacks )
        {
            observerCallbackPair.first->beingDeleted.disconnect( this );
        }
    }

    template <typename ClassType>
    void connect( ClassType* observer, void ( ClassType::*method )( const SignalEmitter*, Args... args ) )
    {
        MemberCallback lambda = [=]( const SignalEmitter* emitter, Args... args ) {
            // Call method
            ( observer->*method )( emitter, args... );
        };
        connect( observer, lambda );
    }

    template <typename ClassType>
    void connect( ClassType* observer, const MemberCallback& callback )
    {
        static_assert( std::is_convertible<ClassType*, SignalObserver*>::value,
                       "Only classes that inherit SignalObserver can connect as an observer of a Signal." );
        m_observerCallbacks[observer] = std::make_pair( callback, true );
        observer->beingDeleted.connect( this );
    }

    // Disconnect an observer from the signal. Do this only when the relationship between the
    // observer and emitter is severed but the object kept alive.
    // There's no need to do this when deleting the observer.
    void disconnect( SignalObserver* observer )
    {
        m_observerCallbacks.erase( observer );
        observer->beingDeleted.disconnect( this );
    }

    void send( Args... args )
    {
        for ( auto observerCallbackPair : m_observerCallbacks )
        {
            if ( observerCallbackPair.second.second )
            {
                observerCallbackPair.second.first( m_emitter, args... );
            }
        }
    }

    void block( SignalObserver* observer )
    {
        auto it = m_observerCallbacks.find( observer );
        CAF_ASSERT( it != m_observerCallbacks.end() );
        it->second.second = false;
    }

    void unblock( SignalObserver* observer )
    {
        auto it = m_observerCallbacks.find( observer );
        CAF_ASSERT( it != m_observerCallbacks.end() );
        it->second.second = true;
    }
    size_t observerCount() const { return m_observerCallbacks.size(); }

private:
    std::map<SignalObserver*, MemberCallbackAndActiveFlag> m_observerCallbacks;
    const SignalEmitter*                                   m_emitter;
};
} // namespace caf
