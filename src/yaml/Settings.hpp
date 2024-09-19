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

// yaml-cpp --------------------------------------------------------------------
#include <yaml-cpp/yaml.h>

// own -------------------------------------------------------------------------
#include "../Settings.hpp"

//------------------------------------------------------------------------------
namespace YAML {

  //----------------------------------------------------------------------------
  template< >
  struct convert< ts::Settings >
  {
    static Node encode( const ts::Settings& );
    static bool decode( const Node&, ts::Settings& );
  };
  
} // end namespace YAML
