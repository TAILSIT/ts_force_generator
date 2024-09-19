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
#include "yaml/parse.hpp"

//------------------------------------------------------------------------------
namespace app {

  //----------------------------------------------------------------------------
  struct DummyForce
  {
    static constexpr ts::SizeT dim = 3;
    
    void operator()( ts::Real                      time,
                     std::span<const ts::Real,dim> U,
                     std::span<ts::Real>           Feval ) const
    {
      static constexpr ts::Real thresholdBegZ = -0.038;
      static constexpr ts::Real thresholdEndZ = -0.08;
      std::ranges::fill( Feval, 0 );
      if( U[2] <= thresholdBegZ && U[2] >= thresholdEndZ ) {
        static constexpr ts::Real mass = 6e-3; // the magnet's mass is 6 gramms
        static constexpr ts::Real grav = 9.81; 
        static constexpr ts::Real fz   = mass * grav;
        Feval[2] = fz;
      }
    }
  };

} // end namespace app

//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
  if( argc != 4 ) {
    std::cerr << "usage: " << std::filesystem::path(argv[0]).filename().string() << " coords.csv settings-precice.xml config.yaml"
              << std::endl;
    std::exit( EXIT_FAILURE );
  }

  /*
   * set filenames
   */
  const std::filesystem::path csvFile( argv[1] );
  const std::filesystem::path xmlFile( argv[2] );
  const std::filesystem::path yamlFile( argv[3] );

  /*
   * parse settings from config yaml
   */
  const ts::Settings settings = ts::yaml::parse(yamlFile);
  
  /*
   * instantiate dummy solver with point cloud
   */
  static constexpr auto numColsInCSV = 3;
  ts::ForceGenerator solver( ts::CSVParser<numColsInCSV>()( csvFile ), settings );
  
  /*
   * instantiate precice
   */
  static constexpr auto rank = 0;
  static constexpr auto size = 1;
  precice::Participant precice( settings.solverName, xmlFile.string(), rank, size );

  /*
   * initialization
   */
  solver.start();
  const ts::SizeT numPoints = solver.numCoordinates();
  static constexpr ts::SizeT dim = solver.dim();
  
  std::vector<ts::Int>  vertexIds(numPoints);
  std::vector<ts::Real> coords( numPoints * dim );

  solver.getCoordinates( coords );

  precice.setMeshVertices( settings.meshName, coords, vertexIds );

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
    precice.readData( settings.meshName,
                      settings.inField,
                      vertexIds,
                      dt,
                      displacementBuffer );

    // pass data to solver
    solver.set( settings.inField, displacementBuffer );

    // 'solve' for this time step
    solver.solveTimeStep( force );

    // fetch forces on points from solver
    solver.get( settings.outField, forcesBuffer );

    // pass forces to precice
    precice.writeData( settings.meshName,
                       settings.outField,
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
