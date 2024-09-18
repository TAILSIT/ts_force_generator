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
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <format>
#include <algorithm>

// preCICE ---------------------------------------------------------------------
#include <precice/precice.hpp>

// own -------------------------------------------------------------------------
#include "types.hpp"
#include "ForceGenerator.hpp"
#include "Settings.hpp"
#include "CSVParser.hpp"


//------------------------------------------------------------------------------
namespace app {

  //----------------------------------------------------------------------------
  struct XMLNames
  {
    static constexpr std::string_view solver        = "ts_dummy_adapter";
    static constexpr std::string_view mesh          = "dummy_magnet";
    static constexpr std::string_view displacements = "Displacements";
    static constexpr std::string_view forces        = "Forces";
  }; 
    
  //----------------------------------------------------------------------------
  struct DummyForce
  {
    static constexpr ts::SizeT dim = 3;
    
    void operator()( ts::Real                      time,
                     std::span<const ts::Real,dim> U,
                     std::span<ts::Real>           Feval ) const
    {
      static constexpr ts::Real thresholdZ = 0.07;
      std::ranges::fill( Feval, 0 );
      if( U[2] >= thresholdZ ) {
        static constexpr ts::Real mass = 6e-3; // the magnet's mass is 6 gramms
        static constexpr ts::Real grav = 9.81; 
        static constexpr ts::Real fz = mass * grav;
        Feval[2] = fz;
      }
    }
  };

} // end namespace app

//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
  if( argc != 3 ) {
    std::cerr << "usage: " << std::filesystem::path(argv[0]).filename().string() << " coords.csv settings-precice.xml"
              << std::endl;
    std::exit( EXIT_FAILURE );
  }

  /*
   * set filenames
   */
  const std::filesystem::path csvFile( argv[1] );
  const std::filesystem::path xmlFile( argv[2] );

  /*
   * instantiate dummy solver with point cloud
   */
  static constexpr auto numColsInCSV = 3;
  ts::ForceGenerator solver( ts::CSVParser<numColsInCSV>()( csvFile ),
                             ts::Settings{ .dt = 5e-3, .endt = 3e-1 } );
  
  /*
   * instantiate precice
   */
  static constexpr auto rank = 0;
  static constexpr auto size = 1;
  precice::Participant precice( app::XMLNames::solver, xmlFile.string(), rank, size );

  /*
   * initialization
   */
  solver.start();
  const ts::SizeT numPoints = solver.numCoordinates();
  static constexpr ts::SizeT dim = solver.dim();
  
  std::vector<ts::Int>  vertexIds(numPoints);
  std::vector<ts::Real> coords( numPoints * dim );

  solver.getCoordinates( coords );

  precice.setMeshVertices( app::XMLNames::mesh, coords, vertexIds );

  precice.initialize();

  /*
   * run 'simulation'
   */
  const app::DummyForce force;
  std::vector<ts::Real> displacementBuffer( numPoints * dim );
  std::vector<ts::Real> forcesBuffer( numPoints * dim );
  while( precice.isCouplingOngoing() ) {

    // possibly save state
    if( precice.requiresWritingCheckpoint() )
      solver.saveOldState();

    // handle time step size
    const ts::Real preciceDt = precice.getMaxTimeStepSize();
    const ts::Real solverDt  = solver.beginTimeStep();
    const ts::Real dt        = std::min( preciceDt, solverDt );

    // read data
    precice.readData( app::XMLNames::mesh,
                      app::XMLNames::displacements,
                      vertexIds,
                      dt,
                      displacementBuffer );

    // pass data to solver
    solver.setDisplacements( displacementBuffer );

    // 'solve' for this time step
    solver.solveTimeStep( force );

    // fetch forces on points from solver
    solver.getForces( forcesBuffer );

    // pass forces to precice
    precice.writeData( app::XMLNames::mesh,
                       app::XMLNames::forces,
                       vertexIds,
                       forcesBuffer );

    // advance in time
    precice.advance(dt);

    // possibly load old state
    if( precice.requiresReadingCheckpoint() )
      solver.reloadOldState();
    else 
      solver.endTimeStep();
  }

  /*
   * shutdown
   */
  precice.finalize();
  solver.stop();

  /*
   * done
   */
  return EXIT_SUCCESS;
}
