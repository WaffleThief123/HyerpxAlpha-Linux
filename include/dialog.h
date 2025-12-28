#ifndef DIALOG_H
#define DIALOG_H

#include <wx/wx.h>

#include <memory>

class dialog : public wxDialog {
 public:
  dialog(const wxChar* title, const wxPoint& pos, const wxSize& size,
         const wxChar* staticImg);

  virtual ~dialog();

 private:
  wxPanel* dialogPanel;
  wxStaticBitmap* dialogLogo;
  wxBoxSizer* dialogSizer;
  std::unique_ptr<wxTimer> dialogTimer;
  wxBitmapBundle logo;
};

#endif
