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

// system ----------------------------------------------------------------------
#include <algorithm>
#include <stdexcept>
#include <format>
#include <fstream>

// own -------------------------------------------------------------------------
#include "ForceGenerator.hpp"

//------------------------------------------------------------------------------
namespace ts {

  //----------------------------------------------------------------------------
  ForceGenerator::ForceGenerator( std::span<const Real> coords,
                                  const Settings&       settings )
    : coords_(coords.begin(),coords.end())
    , currentDisplacements_( coords_.size() )
    , settings_(settings)
    , currentTime_(0)
    , solution_()
    , previousState_(nullptr)
    , samplingForce_( std::make_unique<SamplingForce_>() )
  {
    // empty
  }

  //----------------------------------------------------------------------------
  void ForceGenerator::start( )
  {
    std::ranges::fill( solution_, 0 );
    std::ranges::fill( currentDisplacements_, 0 );
  }

  //----------------------------------------------------------------------------
  void ForceGenerator::stop( )
  {
    static constexpr std::string_view csvOut = "forces.csv";
    if( !(samplingForce_ -> samples).empty() ) {
      const auto& samples = samplingForce_ -> samples;
      std::ofstream out(csvOut.data());
      out << std::format("#{:>14s} {:>15s} {:>15s} {:>15s} {:>15s} {:>15s}\n",
                         "U0", "U1", "U2", "F0", "F1", "F2");
      for( const auto& row : samples ) {
        out << std::format( "{:>14.7e},{:>14.7e},{:>14.7e},{:>14.7e},{:>14.7e},{:>14.7e}\n",
                            row[0], row[1], row[2], row[3], row[4], row[5] );
      }
    }
  }

  //----------------------------------------------------------------------------
  bool ForceGenerator::isRunning( ) const
  {
    return currentTime_ <= settings_.endt;
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
  void ForceGenerator::set( std::string_view fieldname,
                            std::span<const Real> displacements )
  {
    if( fieldname != "Displacements" && fieldname != "DisplacementDeltas" ) {
      const std::string msg =
        std::format( "ts::ForceGenerator::set::fieldname '{}' is invalid",
                     fieldname );
      throw std::runtime_error(msg);
    }
    if( fieldname == "Displacement" )
      std::ranges::copy( displacements, currentDisplacements_.begin() );
    else
      std::ranges::transform( displacements,
                              currentDisplacements_,
                              currentDisplacements_.begin(),
                              []( auto dU, auto Unm1 ) { return dU + Unm1; } );
  }

  //----------------------------------------------------------------------------
  void ForceGenerator::get( std::string_view fieldname,
                            std::span<Real>  forces ) const
  {
    if( fieldname != "Forces" ) {
      const std::string msg =
        std::format( "ts::ForceGenerator::get::fieldname '{}' is invalid",
                     fieldname );
      throw std::runtime_error(msg);
    }

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
