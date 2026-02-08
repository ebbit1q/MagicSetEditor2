//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make card games                |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/updater.hpp>
#include <util/io/package_manager.hpp>

// ----------------------------------------------------------------------------- : Updater

void Updater::updateApplication(String argv) {
  String path = absoluteFilename() + wxFileName::GetPathSeparator() + updater_name;
  path = path + _(".exe");
  if (wxFileExists(path)) {
    wxExecute(path + _(" ") + argv, wxEXEC_ASYNC);
  }
  else {
    queue_message(MESSAGE_ERROR, _("Executable file '" + path + "' not found!"));
  }
}

UpdaterP Updater::byName(const String& name) {
  return package_manager.open<Updater>(name + _(".mse-updater"));
}

IMPLEMENT_REFLECTION_NO_SCRIPT(Updater) {
  REFLECT_BASE(Packaged);
  REFLECT(updater_name);
}

String Updater::typeNameStatic() { return _("updater"); }
String Updater::typeName() const { return _("updater"); }
Version Updater::fileVersion() const { return app_version; }

void Updater::validate(Version file_app_version) {
  Packaged::validate(file_app_version);
}

// special behaviour of reading/writing UpdaterPs: only read/write the name

void Reader::handle(UpdaterP& updater) {
  updater = Updater::byName(getValue());
}
void Writer::handle(const UpdaterP& updater) {
  if (updater) handle(updater->name());
}
