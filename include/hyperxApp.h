#ifndef HYPERXAPP_H
#define HYPERXAPP_H

#include <wx/wx.h>

#include "hyperxFrame.h"

class hyperxApp : public wxApp {
 public:
  hyperxApp(bool systray, bool debug);
  ~hyperxApp();

  virtual bool OnInit();

 private:
  hyperxFrame* m_frame;
  bool systray;
  bool debug;
};

#endif
