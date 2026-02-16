//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make card games                |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/installer.hpp>
#include <util/io/package_manager.hpp>
#include <gui/packages_window.hpp>
#include <wx/wfstream.h>
#include <wx/webrequest.h>
#include <wx/dialup.h>

class DownloadableInstallerList;

// ----------------------------------------------------------------------------- : DownloadableInstallers

/// The global installer downloader
extern DownloadableInstallerList downloadable_installers;

/// Handle downloading of installers
class DownloadableInstallerList {
public:
  inline DownloadableInstallerList() : download_status(NOT_DOWNLOADED), check_status(NOT_CHECKED), shown_dialog(false) {}

  /// Check for updates if the settings say so
  inline void check_updates() {
    settings.check_updates_counter++;
    wxDialUpManager* manager = wxDialUpManager::Create();
    bool connected = manager->IsOk() && manager->IsOnline();
    delete manager;
    if (!connected) return;
    if (
         (settings.check_updates_when == CHECK_ALWAYS)
      || (settings.check_updates_when == CHECK_5  && settings.check_updates_counter > 4)
      || (settings.check_updates_when == CHECK_10 && settings.check_updates_counter > 9)
      ) {
      settings.check_updates_counter = 0;
      check_updates_now();
    }
  }
  /// Check for updates
  /// If async==true then checking is done in another thread
  inline void check_updates_now(bool async = true) {
    if (async) {
      CheckUpdateThread* thread = new CheckUpdateThread;
      thread->Create();
      thread->Run();
    } else {
      CheckUpdateThread::Work();
    }
  }

  /// Start downloading the list of updates, return true if we are done
  inline bool download() {
    if (download_status == DONE) return true;
    if (download_status == NOT_DOWNLOADED) {
      download_status = DOWNLOADING;
      DownloadThread* thread = new DownloadThread();
      thread->Create();
      thread->Run();
    }
    return false;
  }

  /// Show a dialog to inform the user that updates are available (if there are any)
  /// Call check_updates first. Call this function from an onIdle loop
  inline void show_update_dialog(Window* parent) {
    if (shown_dialog || check_status != FOUND) return; // we already have the latest version, or this has already been displayed.
    shown_dialog = true;
    wxMessageDialog dial = wxMessageDialog(parent, _LABEL_("updates found"), _TITLE_("updates available"), wxYES_NO);
    if (dial.ShowModal() == wxID_YES) {
      (new PackagesWindow(parent))->Show();
    }
  }

  // Have we shown the update dialog?
  bool shown_dialog;

  vector<DownloadableInstallerP> installers;

  enum DownloadStatus { NOT_DOWNLOADED, DOWNLOADING, DONE } download_status;
  enum CheckStatus    { NOT_CHECKED, CHECKING, FAILED, FOUND, NOT_FOUND } check_status;
private:
  wxMutex lock;

  struct DownloadThread : public wxThread {
    inline ExitCode Entry() override {
      // fetch list
      wxWebRequestSync request = wxWebSessionSync::GetDefault().CreateRequest(settings.installer_list_url);
      auto const result = request.Execute();
      if (!result) {
        wxMutexLocker l(downloadable_installers.lock);
        downloadable_installers.download_status = DONE;
        downloadable_installers.check_status = FAILED;
        return 0;
      }
      wxInputStream* is = request.GetResponse().GetStream();
      // Read installer list
      Reader reader(*is, nullptr, _("installers"), true);
      vector<DownloadableInstallerP> installers;
      reader.handle(_("installers"),installers);
      // done
      wxMutexLocker l(downloadable_installers.lock);
      swap(installers, downloadable_installers.installers);
      downloadable_installers.download_status = DONE;
      return 0;
    }
  };

  struct CheckUpdateThread : public wxThread {
  public:
    inline void* Entry() override {
#ifndef __APPLE__
      Work();
#endif
      return 0;
    }

    inline static void Work() {
      if (downloadable_installers.check_status > NOT_CHECKED) return; // don't check multiple times simultaneously
      downloadable_installers.check_status = CHECKING;
      try {
        while (!downloadable_installers.download()) {
          wxMilliSleep(30);
        }
        if (downloadable_installers.check_status == FAILED) return;
        InstallablePackages installable_packages;
        FOR_EACH(inst, downloadable_installers.installers) {
          merge(installable_packages, inst);
        }
        FOR_EACH(p, installable_packages) {
          if (p->description->name == mse_package && app_version < p->description->version) {
            downloadable_installers.check_status = FOUND;
            return;
          }
          Version v;
          if (package_manager.installedVersion(p->description->name, v) && v < p->description->version) {
            if (
                 (settings.check_updates_what == CHECK_EVERYTHING)
              || (settings.check_updates_what == CHECK_GAMES && (  p->description->name.EndsWith("mse-updater")
                                                                || p->description->name.EndsWith("mse-locale")
                                                                || p->description->name.EndsWith("mse-game")
                                                                || p->description->name.EndsWith("mse-include")
                                                                || p->description->name.EndsWith("mse-style")
                                                                || p->description->name.EndsWith("mse-symbol-font")))
              || (settings.check_updates_what == CHECK_APP   && (  p->description->name.EndsWith("mse-updater")
                                                                || p->description->name.EndsWith("mse-locale")))
            )
            downloadable_installers.check_status = FOUND;
            return;
          }
        }
        downloadable_installers.check_status = NOT_FOUND;
      } catch (...) {
        // ignore all errors, we don't want problems if update checking fails
        downloadable_installers.check_status = FAILED;
      }
    }
  };
};

