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
#include <vector>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <array>

// own -------------------------------------------------------------------------
#include "types.hpp"

//------------------------------------------------------------------------------
namespace ts {

  //----------------------------------------------------------------------------
  template< SizeT NUM_COLS, typename T = Real >
  struct CSVParser
  {
    using Value = T;
    using Vector = std::vector<Value>;

    Vector operator()( const std::filesystem::path& file,
                       char delimiter   = ',',
                       char commentChar = '#' ) const;
    
  }; // end class CSVParser

  //============================================================================
  //
  // IMPLEMENTATION
  //
  //============================================================================
  template< SizeT NUM_COLS, typename T >
  auto CSVParser<NUM_COLS,T>::operator()( const std::filesystem::path& file,
                                          char delimiter,
                                          char commentChar)
    const -> Vector
  {
    std::ifstream is( file );
    if( !is.is_open() ) {
      const std::string msg = "ts::CSVParser::operator():File '" + file.string() + "' not found";
      throw std::runtime_error(msg);
    }

    std::string line;
    Vector resval;
    while( std::getline( is, line ) ) {
      // skip comment lines
      if( !line.empty() && line[0] == commentChar ) continue;

      std::istringstream stream(line);
      std::array<Value,NUM_COLS> row;
      std::string val;

      // Tokenize line
      while( std::getline( stream, val, delimiter ) ) {
        std::istringstream iss(val);
        std::copy( std::istream_iterator<Value>(iss),
                   std::istream_iterator<Value>(),
                   std::back_inserter( resval ) );
      }
    }
    
    return resval;
  }
  
} // end namespace ts
