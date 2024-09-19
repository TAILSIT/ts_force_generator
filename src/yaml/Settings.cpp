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

// own -------------------------------------------------------------------------
#include "../types.hpp"
#include "Settings.hpp"

//------------------------------------------------------------------------------
namespace YAML {

  //----------------------------------------------------------------------------
  Node convert< ts::Settings >::encode( const ts::Settings& settings )
  {
    Node node;
    node["solverName"] = settings.solverName;
    node["meshName"]   = settings.meshName;
    node["inField"]    = settings.inField;
    node["outField"]   = settings.outField;
    node["dt"]         = settings.dt;
    node["endt"]       = settings.endt;
    return node;
  }
    
  //----------------------------------------------------------------------------
  bool convert< ts::Settings >::decode( const Node& node, ts::Settings& settings )
  {
    settings.solverName = node["solverName"].as<std::string>();
    settings.meshName   = node["meshName"].as<std::string>();
    settings.inField    = node["inField"].as<std::string>();
    settings.outField   = node["outField"].as<std::string>();
    settings.dt         = node["dt"].as<ts::Real>();
    settings.endt       = node["endt"].as<ts::Real>();
    return true;
  }
  
} // end namespace YAML
