#include <cstring>
#include <memory>

#include "hyperxApp.h"

int main(int argc, char* argv[]) {
  bool systray = false;
  bool debug = false;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--systray") == 0)
      systray = true;
    else if (strcmp(argv[i], "--debug") == 0)
      debug = true;
    else {
      std::cout << "HyperX Alpha Help" << std::endl;
      std::cout << "  --systray  Start with legacy systray support" << std::endl;
      std::cout << "  --debug    Print HID packet data to stdout" << std::endl;
      return 0;
    }
  }

  std::unique_ptr<wxApp> pApp = std::make_unique<hyperxApp>(systray, debug);
  wxApp::SetInstance(pApp.get());
  wxEntry(argc, argv);
  pApp.release();  // wxWidgets manages cleanup, don't delete it
  wxEntryCleanup();
  return 0;
}
