#include "hyperxApp.h"

#include <cstdlib>
#include <sys/stat.h>

#include "hyperxFrame.h"

// App
hyperxApp::hyperxApp(bool systray, bool debug) : systray(systray), debug(debug) {}
hyperxApp::~hyperxApp() {}

bool hyperxApp::OnInit() {
  wxImage::AddHandler(new wxPNGHandler);

  // Find asset directory: check next to binary first, then /usr/share/hyperx/
  char* resolved_path = realpath(argv[0], nullptr);
  wxString runDir(resolved_path);
  free(resolved_path);
  runDir.erase(runDir.end() - 6, runDir.end());

  struct stat st;
  wxString shareDir = "/usr/share/hyperx/";
  if (stat((runDir + "img/hyperx.png").mb_str(), &st) != 0 &&
      stat((shareDir + "img/hyperx.png").mb_str(), &st) == 0) {
    runDir = shareDir;
  }

  try {
    m_frame = new hyperxFrame(_T("HyperX Alpha"), wxDefaultPosition,
                              wxSize(200, 400), runDir, this, systray, debug);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  SetTopWindow(m_frame);
  return true;
}

// start the application from here
// wxIMPLEMENT_APP(hyperxApp);
