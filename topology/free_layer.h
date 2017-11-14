/*
 *  free_layer.h
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

#ifndef FREE_LAYER_H
#define FREE_LAYER_H

// C++ includes:
#include <algorithm>
#include <limits>
#include <sstream>

// Includes from sli:
#include "dictutils.h"

// Includes from topology:
#include "layer.h"
#include "ntree_impl.h"
#include "topology_names.h"

namespace nest
{

/**
 * Layer with free positioning of neurons, positions specified by user.
 */
template < int D >
class FreeLayer : public Layer< D >
{
public:
  Position< D > get_position( index sind ) const;
  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

protected:
  /**
   * Communicate positions across MPI processes
   * @param iter Insert iterator which will receive pairs of Position,GID
   */
  template < class Ins >
  void communicate_positions_( Ins iter );

  void insert_global_positions_ntree_( Ntree< D, index >& tree );
  void insert_global_positions_vector_(
    std::vector< std::pair< Position< D >, index > >& vec );

  /// Vector of positions.
  std::vector< Position< D > > positions_;

  /// This class is used when communicating positions across MPI procs.
  class NodePositionData
  {
  public:
    index
    get_gid() const
    {
      return gid_;
    }
    Position< D >
    get_position() const
    {
      return Position< D >( pos_ );
    }
    bool operator<( const NodePositionData& other ) const
    {
      return gid_ < other.gid_;
    }
    bool operator==( const NodePositionData& other ) const
    {
      return gid_ == other.gid_;
    }

  private:
    double gid_;
    double pos_[ D ];
  };
};

template < int D >
void
FreeLayer< D >::set_status( const DictionaryDatum& d )
{
  Layer< D >::set_status( d );

  // Read positions from dictionary
  if ( d->known( names::positions ) )
  {
    TokenArray pos = getValue< TokenArray >( d, names::positions );
    if ( this->gid_collection_->size() != pos.size() )
    {
      std::stringstream expected;
      std::stringstream got;
      expected << "position array with length "
               << this->gid_collection_->size();
      got << "position array with length" << pos.size();
      throw TypeMismatch( expected.str(), got.str() );
    }

    positions_.clear();
    positions_.reserve( this->gid_collection_->size() );

    for ( Token* it = pos.begin(); it != pos.end(); ++it )
    {
      Position< D > point = getValue< std::vector< double > >( *it );
      if ( not( ( point >= this->lower_left_ )
             and ( point < this->lower_left_ + this->extent_ ) ) )
      {
        throw BadProperty( "Node position outside of layer" );
      }

      positions_.push_back( point );
    }
  }
}

template < int D >
void
FreeLayer< D >::get_status( DictionaryDatum& d ) const
{
  Layer< D >::get_status( d );

  TokenArray points;
  for ( typename std::vector< Position< D > >::const_iterator it =
          positions_.begin();
        it != positions_.end();
        ++it )
  {
    points.push_back( it->getToken() );
  }
  def2< TokenArray, ArrayDatum >( d, names::positions, points );
}

template < int D >
Position< D >
FreeLayer< D >::get_position( index lid ) const
{
  return positions_.at( lid );
}

template < int D >
template < class Ins >
void
FreeLayer< D >::communicate_positions_( Ins iter )
{
  // This array will be filled with GID,pos_x,pos_y[,pos_z] for local nodes:
  std::vector< double > local_gid_pos;

  GIDCollection::const_iterator gc_begin =
    this->gid_collection_->MPI_local_begin();
  GIDCollection::const_iterator gc_end = this->gid_collection_->end();

  local_gid_pos.reserve( ( D + 1 ) * this->gid_collection_->size() );

  for ( GIDCollection::const_iterator gc_it = gc_begin; gc_it < gc_end;
        ++gc_it )
  {
    // Push GID into array to communicate
    local_gid_pos.push_back( ( *gc_it ).gid );
    // Push coordinates one by one
    for ( int j = 0; j < D; ++j )
    {
      local_gid_pos.push_back( positions_[ ( *gc_it ).lid ][ j ] );
    }
  }

  // This array will be filled with GID,pos_x,pos_y[,pos_z] for global nodes:
  std::vector< double > global_gid_pos;
  std::vector< int > displacements;
  kernel().mpi_manager.communicate(
    local_gid_pos, global_gid_pos, displacements );

  // To avoid copying the vector one extra time in order to sort, we
  // sneakishly use reinterpret_cast
  NodePositionData* pos_ptr;
  NodePositionData* pos_end;
  pos_ptr = reinterpret_cast< NodePositionData* >( &global_gid_pos[ 0 ] );
  pos_end = pos_ptr + global_gid_pos.size() / ( D + 1 );

  // Get rid of any multiple entries
  std::sort( pos_ptr, pos_end );
  pos_end = std::unique( pos_ptr, pos_end );

  // Unpack GIDs and coordinates
  for ( ; pos_ptr < pos_end; pos_ptr++ )
  {
    *iter++ = std::pair< Position< D >, index >(
      pos_ptr->get_position(), pos_ptr->get_gid() );
  }
}

template < int D >
void
FreeLayer< D >::insert_global_positions_ntree_( Ntree< D, index >& tree )
{

  communicate_positions_( std::inserter( tree, tree.end() ) );
}

// Helper function to compare GIDs used for sorting (Position,GID) pairs
template < int D >
static bool
gid_less( const std::pair< Position< D >, index >& a,
  const std::pair< Position< D >, index >& b )
{
  return a.second < b.second;
}

template < int D >
void
FreeLayer< D >::insert_global_positions_vector_(
  std::vector< std::pair< Position< D >, index > >& vec )
{

  communicate_positions_( std::back_inserter( vec ) );

  // Sort vector to ensure consistent results
  std::sort( vec.begin(), vec.end(), gid_less< D > );
}

} // namespace nest

#endif
