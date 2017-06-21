/*
 *  recording_device.cpp
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Includes from libnestutil:
#include "compose.hpp"
#include "kernel_manager.h"

#include "recording_device.h"

nest::RecordingDevice::Parameters_::Parameters_()
  : label_()
  , time_in_steps_( false )
{
}

void
nest::RecordingDevice::Parameters_::get( const RecordingDevice& device,
  DictionaryDatum& d ) const
{
  ( *d )[ names::label ] = label_;
  ( *d )[ names::time_in_steps ] = time_in_steps_;
  ( *d )[ names::record_to ] = record_to_;
}

void
nest::RecordingDevice::Parameters_::set( const RecordingDevice&,
					 const DictionaryDatum& d,
					 long n_events )
{
  updateValue< std::string >( d, names::label, label_ );

  bool time_in_steps = time_in_steps_;
  updateValue< bool >( d, names::time_in_steps, time_in_steps );
  if ( time_in_steps != time_in_steps_ and n_events != 0 )
  {
    throw BadProperty("Property /time_in_steps cannot be set if recordings exist. "
		      "Please clear the events first by setting /n_events to 0.");
  }
  time_in_steps_ = time_in_steps;
}

nest::RecordingDevice::State_::State_()
  : n_events_( 0 )
{
}

void
nest::RecordingDevice::State_::get( DictionaryDatum& d ) const
{
  // if we already have the n_events entry, we add to it, otherwise we create it
  if ( d->known( names::n_events ) )
  {
    long n_events = getValue< long >( d, names::n_events );
    ( *d )[ names::n_events ] = n_events + n_events_;
  }
  else
  {
    ( *d )[ names::n_events ] = n_events_;
  }
}

void
nest::RecordingDevice::State_::set( const DictionaryDatum& d,
  const RecordingDevice& rd )
{
  long n_events = n_events_;
  if ( updateValue< long >( d, names::n_events, n_events ) )
  {
    if ( n_events == 0 )
    {
      kernel().io_manager.clear_recording_backends( rd );
      n_events_ = n_events;
    }
    else
    {
      throw BadProperty( "Property /n_events can only be set "
        "to 0 (which clears all stored events)." );
    }
  }
}

void
nest::RecordingDevice::set_status( const DictionaryDatum& d )
{
  State_ stmp = S_;         // temporary copy in case of errors
  stmp.set( d, *this );     // throws if BadProperty
  Parameters_ ptmp = P_;    // temporary copy in case of errors
  ptmp.set( *this, d, stmp.n_events_ ); // throws if BadProperty

  Device::set_status( d );
  kernel().io_manager.set_recording_device_status( *this, d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}
