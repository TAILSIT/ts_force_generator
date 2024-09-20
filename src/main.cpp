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
#include <cmath>
#include <chrono>

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

    ts::Real eval( ts::Real s ) const
    {
      const auto e1 = std::exp(20. - 1000.*s);
      const auto e2 = std::exp(25. - 1000.*s);
      const auto e3 = std::exp(85. - 1000.*s);
      const auto scale =  2./(1.+e1) - 1./(1.+e2) - 1./(1.+e3);
      static constexpr ts::Real m = 6e-3;
      static constexpr ts::Real g = 9.81;
      return m*g*scale;
      //return 0;
    }
    
    void operator()( ts::Real                      time,
                     std::span<const ts::Real,dim> U,
                     std::span<ts::Real>           Feval ) const
    {
      std::ranges::fill( Feval, 0 );
      Feval[2] = this->eval( std::abs(U[2]) );
    }
  };

} // end namespace app

//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
  const std::filesystem::path appname = std::filesystem::path{ argv[0] }.filename();
  if( argc != 4 ) {
    std::cerr << "usage: " << appname.string()
              << " coords.csv settings-precice.xml config.yaml"
              << std::endl;
    std::exit( EXIT_FAILURE );
  }

  /*
   * start timing
   */
  const auto startTime = std::chrono::high_resolution_clock::now();
  
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
  ts::yaml::Parser::dump(std::cout,settings);
  
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
    const bool sampleForce = true;
    solver.solveTimeStep( force, sampleForce );

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
  const auto endTime = std::chrono::high_resolution_clock::now();
  using Seconds = std::ratio<1>;
  const std::chrono::duration<ts::Real,Seconds> diff = endTime - startTime;
  std::cout << std::format( "{}: Total runtime = {}\n",
                            appname.string(),
                            diff );
                            
  return EXIT_SUCCESS;
}
