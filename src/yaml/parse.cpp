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
#include "parse.hpp"
#include "Settings.hpp"

//------------------------------------------------------------------------------
namespace ts {
  namespace yaml {

    //--------------------------------------------------------------------------
    Settings Parser::parse( const std::filesystem::path& yamlFile )
    {
      YAML::Node node = ::YAML::LoadFile( yamlFile.string() );
      return node[root.data()].as<ts::Settings>();
    }

    //--------------------------------------------------------------------------
    std::ostream& Parser::dump( std::ostream& out,
                                const Settings& settings )
    {
      YAML::Node node;
      node[root.data()] = settings;
      out << "# ============================================================"
          << std::endl;
      out << "# SETTINGS: " << std::endl
          << "# ------------------------------------------------------------"
          << std::endl
          << ::YAML::Dump(node) << std::endl
          << "# ============================================================"
          << std::endl;
      return out;
    }

    //--------------------------------------------------------------------------
    std::string Parser::asString( const Settings& settings )
    {
      std::stringstream ss;
      dump( ss, settings );
      return ss.str();
    }
    
    //--------------------------------------------------------------------------
    Settings parse( const std::filesystem::path& yamlFile )
    {
      return Parser::parse(yamlFile);
    }
  } // end namespace yaml
} // end namespace ts
