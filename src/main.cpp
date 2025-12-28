#include <memory>

#include "hyperxApp.h"

int main(int argc, char* argv[]) {
  if (argv[1] == NULL || std::string(argv[1]) == "--systray") {
    std::unique_ptr<wxApp> pApp = std::make_unique<hyperxApp>(argv[1] != NULL);
    wxApp::SetInstance(pApp.get());
    wxEntry(argc, argv);
    pApp.release();  // wxWidgets manages cleanup, don't delete it
    wxEntryCleanup();
  } else {
    std::cout << "HyperX Alpha Help" << std::endl;
    std::cout << "  Use --systray to start gui with legacy systray support!"
              << std::endl;
  }
  return 0;
}
