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
#include <string>

// own -------------------------------------------------------------------------
#include "types.hpp"

//------------------------------------------------------------------------------
namespace ts
{

  //----------------------------------------------------------------------------
  struct Settings
  {
    std::string solverName = "ts_dummy_adapter";
    std::string meshName   = "dummy_magnet";
    std::string inField    = "Displacements";
    std::string outField   = "Forces";
    Real        dt         = 5e-3;
    Real        endt       = 3e-1;
  };
  
}
