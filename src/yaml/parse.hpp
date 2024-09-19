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
#include <filesystem>
#include <ostream>
#include <string>

// own -------------------------------------------------------------------------
#include "../Settings.hpp"

// yaml-cpp --------------------------------------------------------------------
#include <yaml-cpp/yaml.h>

//------------------------------------------------------------------------------
namespace ts {
  namespace yaml {

    //--------------------------------------------------------------------------
    struct Parser
    {
      static constexpr std::string_view root = "settings";
      
      Parser() = delete;

      static Settings parse( const std::filesystem::path& yamlFile );

      static std::ostream& dump( std::ostream& out,
                                 const Settings& settings );
      
      static std::string asString( const Settings& settings );
    };
      
    
    //--------------------------------------------------------------------------
    Settings parse( const std::filesystem::path& yamlFile );

  } // end namespace yaml
} // end namespace ts
