// Copyright (C) 2013 Vrije Universiteit Brussel, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_lss_LibLSS_DLIB_hpp
#define cf3_lss_LibLSS_DLIB_hpp


#define SANDBOX
//#undef SANDBOX


#include "cf3/common/Library.hpp"
#include "../../../lss/cf3/lss/LibLSS.hpp"


namespace cf3 {
namespace lss {


/**
 * @brief LibLSS_DLIB class: Interface to Dlib linear system solver.
 * @author Pedro Maciel
 */
struct lss_API LibLSS_DLIB :
  public cf3::common::Library
{
  /// Constructor
  LibLSS_DLIB(const std::string& name) : cf3::common::Library(name) {}

  /// @return library namespace, name, description and class name
  static std::string library_namespace()    { return "cf3.lss.dlib"; }
  static std::string library_name()         { return "dlib"; }
  static std::string library_description()  { return "Interface to Dlib linear system solver."; }
  static std::string type_name()            { return "LibLSS_DLIB"; }

  /// Initiate library
  void initiate();
};


}  // lss
}  // cf3


#endif // cf3_lss_LibLSS_DLIB_hpp

