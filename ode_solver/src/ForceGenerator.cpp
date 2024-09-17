//------------------------------------------------------------------------------
// <preamble>
//
//  ______
// |
// | TailSiT GmbH
// | Graz, Austria
//   www.tailsit.com
//
// </preamble>
//------------------------------------------------------------------------------

//! @author    Lars Kielhorn, Thomas Rüberg, Jürgen Zechner
//! @date      2024
//! @copyright TailSiT GmbH
#pragma once

// system ----------------------------------------------------------------------
#include <algorithm>
#include <stdexcept>

// own -------------------------------------------------------------------------
#include "ForceGenerator.hpp"

//------------------------------------------------------------------------------
namespace ts {

  
//  std::vector<Real>         coords_;
//  std::vector<Real>         currentDisplacements_;
//  Settings                  settings_;
//  Real                      currentTime_;
//  std::array<Real,dimMesh_> solution_; // well, not really a solution just the evaluated force
//
//  struct SavedState_
//  {
//    std::vector<Real>         displacements;
//    std::array<Real,dimMesh_> solution;
//    Real                      time;
//  };
//  std::unique_ptr<SavedState_> previousState_;

  //----------------------------------------------------------------------------
  ForceGenerator::ForceGenerator( std::span<const Real> coords,
                                  const Settings&       settings )
    : coords_(coords)
    , currentDisplacements_( coords_.size(), 0 )
    , settings_(settings)
    , currentTime_(0)
    , solution_()
    , previousState_(nullptr)
  {
    // empty
  }

  //----------------------------------------------------------------------------
  void ForceGenerator::start( )
  {
    std::ranges::fill( solution_, 0 );    
  }

  void stop( );

  //----------------------------------------------------------------------------
  bool ForceGenerator::isRunning( ) const
  {
    return currentTime_ <= settings.endt;
  }

  //----------------------------------------------------------------------------
  Real ForceGenerator::beginTimeStep( )
  {
    return settings_.dt;
  }

  //----------------------------------------------------------------------------
  void ForceGenerator::endTimeStep( )
  {
    previousState_ = nullptr;
  }

  //----------------------------------------------------------------------------
  SizeT ForceGenerator::numCoordinates( ) const
  {
    return coords_.size() / dimMesh_;
  }
    
  //----------------------------------------------------------------------------
  void ForceGenerator::getCoordinates( std::span<Real> coords ) const
  {
    std::ranges::copy( coords_, coords.begin() );
  }

  //----------------------------------------------------------------------------
  void ForceGenerator::setDisplacements( std::span<const Real> displacements )
  {
    std::ranges::copy( displacements, currentDisplacements_.begin() );
  }

  //----------------------------------------------------------------------------
  void ForceGenerator::getForces( std::span<Real> forces ) const
  {
    const SizeT numForces = forces.size() / dimMesh_;
    if( numForces * dimMesh_ - forces.size() != 0 )
      throw std::runtime_error( "ForceGenerator::getForces():Invalid size" );
    std::array<Real,dimMesh_> avgSol;
    std::ranges::transform( solution_, avgSol.begin(),
                            [numForces]( Real f ) { return f/numForces; } );
    for( SizeT n = 0; n < numForces; ++n )
      std::ranges::copy( avgSol, forces.begin() + n*dimMesh_ );
  }

  //----------------------------------------------------------------------------
  void ForceGenerator::saveOldState( )
  {
    SavedState_ ss;
    ss.displacements = currentDisplacements_;
    ss.solution      = solution_;
    ss.time          = currentTime_;
    previousState_ = std::make_unique<SavedState_>(std::move(ss));
  }
  
  //----------------------------------------------------------------------------
  void ForceGenerator::reloadOldState( )
  {
    if( !previousState_ )
      throw std::runtime_error( "ForceGenerator::reloadOldState::State not available!" );
    currentDisplacements_ = previousState_ -> displacements;
    currentTime_          = previousState_ -> time;
    solution_             = previousState_ -> solution;
  }
  
}; // end class ForceGenerator
