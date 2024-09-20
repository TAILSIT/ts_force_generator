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
#include <span>
#include <array>
#include <vector>
#include <memory>

// own -------------------------------------------------------------------------
#include "types.hpp"
#include "Settings.hpp"

//------------------------------------------------------------------------------
namespace ts {

  class ForceGenerator;

}
  
//------------------------------------------------------------------------------
class ts::ForceGenerator
{
  static constexpr Int dimMesh_ = 3;
  
private:
  std::vector<Real>         coords_;
  std::vector<Real>         currentDisplacements_;
  Settings                  settings_;
  Real                      currentTime_;
  std::array<Real,dimMesh_> solution_; // well, not really a solution just the evaluated force
  
  struct SavedState_
  {
    std::vector<Real>         displacements;
    std::array<Real,dimMesh_> solution;
    Real                      time;
  };
  std::unique_ptr<SavedState_> previousState_;

  struct SamplingForce_
  {
    using Row = std::array< Real,6 >;
    using Table = std::vector<Row>;

    Table samples;
  };
  std::unique_ptr<SamplingForce_> samplingForce_;
  
public:
  ForceGenerator( std::span<const Real> coords,
                  const Settings& settings );


  /** @name static data */
  //@{
  static constexpr SizeT dim() { return dimMesh_; }
  //@}
  
  /** @name solver controls */
  //@{
  void start( );

  void stop( );

  bool isRunning( ) const;

  Real beginTimeStep( );
  
  template< typename FORCE >
  void solveTimeStep( FORCE&& force, bool sampleForce = false );

  void endTimeStep( );
  //@}

  /** @name mesh information */
  //@{
  SizeT numCoordinates( ) const;
  void getCoordinates( std::span<Real> coords ) const;
  //@}

  /** @name set displacements / get forces ... nothing else */
  //@{
  void set( std::string_view fieldname, std::span<const Real> displacements );
  
  void get( std::string_view fieldname, std::span<Real> forces ) const;
  //@}

  /** @name save/load state [for implicit schemes] */
  //@{
  void saveOldState( );
  void reloadOldState( );
  //@}
  
}; // end class ForceGenerator

//============================================================================
//
// IMPLEMENTATION
//
//============================================================================
template< typename FORCE >
void ts::ForceGenerator::solveTimeStep( FORCE&& force, bool sampleForce ) 
{
  // this is all about rigid body movement (without rotations!)
  // Thus, the very first displacement is as good as any other
  auto U = std::span<const Real,3>{ currentDisplacements_.data(), dimMesh_ };
  force( currentTime_, U, solution_ );
  if( sampleForce ) {
    auto& samples = samplingForce_ -> samples;
    samples.push_back( { U[0], U[1], U[2], solution_[0], solution_[1], solution_[2] } );
  }
}

