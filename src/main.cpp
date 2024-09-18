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

// own -------------------------------------------------------------------------
#include "types.hpp"
#include "ForceGenerator.hpp"
#include "Settings.hpp"
#include "CSVParser.hpp"

//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
  if( argc != 2 ) {
    std::cerr << "usage: " << std::filesystem::path(argv[0]).filename().string() << " coords.csv"
              << std::endl;
    std::exit( EXIT_FAILURE );
  }

  static constexpr auto numColsInCSV = 3;
  ts::ForceGenerator solver( ts::CSVParser<numColsInCSV>()( std::filesystem::path(argv[1])),
                             ts::Settings{ .dt = 5e-3, .endt = 3e-1 } );

  
  
  return EXIT_SUCCESS;
}
