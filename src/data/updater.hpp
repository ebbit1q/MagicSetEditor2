//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make card games                |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <util/io/package.hpp>

DECLARE_POINTER_TYPE(Updater);

// ----------------------------------------------------------------------------- : Updater

/// A description of an updater for the app (There should really only ever be one updater)
class Updater : public Packaged {
public:
  Updater() {};

  String updater_name;

  // Close MSE and run the updater exe
  void updateApplication(String argv);

  static UpdaterP byName(const String& name);

  static String typeNameStatic();
  String typeName() const override;
  Version fileVersion() const override;

protected:
  void validate(Version) override;

  DECLARE_REFLECTION();
};

inline String type_name(const Updater&) {
  return _TYPE_("updater");
}

