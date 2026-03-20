#include "hyperxFrame.h"

#include <sys/stat.h>
#include <wx/stdpaths.h>

#include "dialog.h"
#include "hyperxApp.h"

hyperxFrame::hyperxFrame(const wxChar* title, const wxPoint& pos,
                         const wxSize& size, const wxChar* runDir, wxApp* app,
                         bool useTray, bool debug)
    : wxFrame(nullptr, wxID_ANY, title, pos, size),
      m_headset(std::make_unique<headset>()),
      m_runDir(runDir),
      running(true),
      app(app),
      useTray(useTray),
      debug(debug) {
  if (!m_headset->init()) {
    dialog* error =
        new dialog(_T("HyperX Cloud Alpha Unavailable"), wxDefaultPosition,
                   wxSize(440, 150), m_runDir + _T("img/poweredOff.png"));
    throw std::runtime_error("Failed to initialize headset");
  }

  t = std::thread(&hyperxFrame::read_loop, this);
  // headset polling
  timer = std::make_unique<wxTimer>();
  timer->Bind(wxEVT_TIMER, &hyperxFrame::on_timer, this);

  if (wxTaskBarIcon::IsAvailable() && useTray) {
    taskAvailable = true;
    taskBarIcon = std::make_unique<wxTaskBarIcon>();
    taskBarIcon->Bind(wxEVT_TASKBAR_RIGHT_DOWN, &hyperxFrame::showMenu, this);
    taskBarIcon->Bind(wxEVT_TASKBAR_LEFT_DOWN, &hyperxFrame::showWindow, this);
    setTaskIcon();
  }

  createFrame();

  const char* xdgConfig = getenv("XDG_CONFIG_HOME");
  wxString configDir = xdgConfig ? wxString(xdgConfig) + "/hyperx"
                                 : wxString(wxGetHomeDir() + "/.config/hyperx");
  mkdir(configDir.mb_str(), 0755);
  m_configPath = configDir + "/config";

  m_headset->send_command(commands::CONNECTION_STATE);
}

// Status update from headset
void hyperxFrame::on_timer(wxTimerEvent& event) {
  m_headset->send_command(commands::STATUS_REQUEST);
  m_headset->send_command(commands::PING);
}

// Left Click Taskbar
void hyperxFrame::showWindow(wxTaskBarIconEvent& event) {
  (IsShown()) ? this->Hide() : this->Show();
}

void hyperxFrame::showMenu(wxTaskBarIconEvent& event) {
  enum { hOPEN = 2525, hQUIT };
  taskMenu = new wxMenu();
  taskMenu->Append(hOPEN, (IsShown()) ? _T("Hide") : _T("Show"), _T(""),
                   wxITEM_NORMAL);
  taskMenu->AppendSeparator();
  taskMenu->Append(hQUIT, _T("Quit"), _T(""), wxITEM_NORMAL);
  taskMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent& event) {
    switch (event.GetId()) {
      case hQUIT:
        this->quit(event);
        break;
      case hOPEN:
        (IsShown()) ? this->Hide() : this->Show();
        break;
      default:;
        break;
    }
  });
  taskBarIcon->PopupMenu(taskMenu);
}

void hyperxFrame::micSwitch(wxCommandEvent& event) {
  (micMonitor->GetValue())
      ? m_headset->send_command(commands::MICROPHONE_MONITOR)
      : m_headset->send_command(commands::MICROPHONE_MONITOR_OFF);
  mic_monitor = micMonitor->GetValue();
  saveSettings();
}

void hyperxFrame::voiceSwitch(wxCommandEvent& event) {
  (voicePrompt->GetValue())
      ? m_headset->send_command(commands::VOICE_PROMPTS)
      : m_headset->send_command(commands::VOICE_PROMPTS_OFF);
  voice = voicePrompt->GetValue();
  saveSettings();
}

void hyperxFrame::quit(wxCommandEvent& event) {
  running = false;
  m_headset->send_command(commands::PING);
  if (timer->IsRunning()) {
    timer->Stop();
  }
  t.join();
  if (taskAvailable) {
    taskBarIcon->RemoveIcon();
  }
  this->Destroy();
}

void hyperxFrame::sleepChoice(wxCommandEvent& event) {
  switch (sleepTimer->GetSelection()) {
    case 0:
      m_headset->send_command(commands::SLEEP_TIMER_10);
      break;
    case 1:
      m_headset->send_command(commands::SLEEP_TIMER_20);
      break;
    case 2:
      m_headset->send_command(commands::SLEEP_TIMER_30);
      break;
    default:
      break;
  }
  saveSettings();
}

void hyperxFrame::setTaskIcon() {
  if (taskAvailable) {
    if (status == connection_status::CONNECTED) {
      switch (battery) {
        case 0 ... 10:
          wicon = wxIcon(wxIconLocation(m_runDir + _T("img/tray0.png")));
          break;
        case 11 ... 30:
          wicon = wxIcon(wxIconLocation(m_runDir + _T("img/tray20.png")));
          break;
        case 31 ... 50:
          wicon = wxIcon(wxIconLocation(m_runDir + _T("img/tray40.png")));
          break;
        case 51 ... 70:
          wicon = wxIcon(wxIconLocation(m_runDir + _T("img/tray60.png")));
          break;
        case 71 ... 90:
          wicon = wxIcon(wxIconLocation(m_runDir + _T("img/tray80.png")));
          break;
        case 91 ... 100:
          wicon = wxIcon(wxIconLocation(m_runDir + _T("img/tray100.png")));
          break;
      }
      taskBarIcon->SetIcon(wicon, std::to_string(battery * 3) +
                                      " Hours Remaining(" +
                                      std::to_string(battery) + "%)");
    } else {
      wicon = wxIcon(wxIconLocation(m_runDir + _T("img/traydc.png")));
      taskBarIcon->SetIcon(wicon, "Power Off");
    }
  }
}

void hyperxFrame::createFrame() {
  // main Layout

  this->Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& event) {
    if (taskAvailable) {
      this->Hide();
    } else {
      wxCommandEvent quitEvent;
      this->quit(quitEvent);
    }
  });
  this->Bind(wxEVT_ICONIZE, [this](wxIconizeEvent& event) { this->Hide(); });
  const auto margin = FromDIP(4);
  auto mainSizer = new wxBoxSizer(wxHORIZONTAL);
  wxPanel* panel = new wxPanel(this, wxID_ANY);
  auto sizer = new wxBoxSizer(wxVERTICAL);

  // logo
  wxBitmapBundle logoImg(
      wxImage(m_runDir + "img/hyperx.png", wxBITMAP_TYPE_PNG));
  auto logo = new wxStaticBitmap(panel, wxID_ANY, logoImg, wxDefaultPosition,
                                 wxSize(200, 70));

  connectedLabel = new wxStaticText(panel, wxID_ANY, _T("Connected"));

  // Feature
  auto featureBox = new wxStaticBoxSizer(wxVERTICAL, panel, _T("Features"));
  // sleepTimer
  sleepTimerLabel = new wxStaticText(panel, wxID_ANY, _T("Sleep Timer"));
  sleepTimer =
      new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);
  sleepTimer->SetSelection(0);
  sleepTimer->SetToolTip(_T("Set Sleep Timer"));
  sleepTimer->Bind(wxEVT_CHOICE, &hyperxFrame::sleepChoice, this);
  sleepTimer->Disable();
  // voicePrompt
  voicePromptLabel = new wxStaticText(panel, wxID_ANY, _T("Voice Prompt"));
  voicePrompt = new wxSwitchCtrl(panel, wxID_ANY, false);
  voicePrompt->SetToolTip(_T("Enable Voice Prompt"));
  voicePrompt->Bind(wxEVT_SWITCH, &hyperxFrame::voiceSwitch, this);
  voicePrompt->Disable();

  // micMonitor
  micMonitorLabel = new wxStaticText(panel, wxID_ANY, _T("Mic Monitor"));
  micMonitor = new wxSwitchCtrl(panel, wxID_ANY, false);
  micMonitor->SetToolTip(_T("Enable Mic Monitor"));
  micMonitor->Bind(wxEVT_SWITCH, &hyperxFrame::micSwitch, this);
  micMonitor->Disable();

  // mic mute status
  micMuteLabel = new wxStaticText(panel, wxID_ANY, _T("Mic: --"));

  // add to feature box
  featureBox->Add(sleepTimerLabel, 0, wxEXPAND | wxALL, margin);
  featureBox->Add(sleepTimer, 0, wxEXPAND | wxALL, margin);
  featureBox->Add(voicePromptLabel, 0, wxALIGN_RIGHT | wxALL, margin);
  featureBox->Add(voicePrompt, 0, wxALIGN_RIGHT | wxALL, margin);
  featureBox->Add(micMonitorLabel, 0, wxALIGN_RIGHT | wxALL, margin);
  featureBox->Add(micMonitor, 0, wxALIGN_RIGHT | wxALL, margin);
  featureBox->Add(micMuteLabel, 0, wxEXPAND | wxALL, margin);

  auto buttonBox = new wxBoxSizer(wxVERTICAL);
  quitButton = new wxButton(panel, wxID_EXIT, _T("Quit"));
  quitButton->Bind(wxEVT_BUTTON, &hyperxFrame::quit, this);

  buttonBox->Add(featureBox, 0, wxEXPAND | wxALL, margin);
  buttonBox->Add(quitButton, 0, wxEXPAND | wxALL, margin);
  if (taskAvailable) {
    hideButton = new wxButton(panel, wxID_ANY, _T("Minimize"));
    hideButton->Bind(wxEVT_BUTTON,
                     [this](wxCommandEvent& event) { this->Hide(); });
    buttonBox->Add(hideButton, 0, wxEXPAND | wxALL, margin);
  }

  sizer->Add(logo, 0, wxLEFT | wxRIGHT, margin);
  sizer->Add(connectedLabel, 0, wxLEFT | wxRIGHT, margin);
  sizer->Add(buttonBox, 0, wxEXPAND | wxALL, margin);

  panel->SetSizer(sizer);
  mainSizer->Add(panel, 1, wxALL, 8);
  this->SetSizerAndFit(mainSizer);
  if (!taskAvailable) {
    this->Show();
  }
}

void hyperxFrame::onConnect() {
  if (status == connection_status::CONNECTED)
    return;
  status = connection_status::CONNECTED;
  connectedLabel->SetLabel(_T("Connected"));
  micMuteLabel->SetLabel(_T("Mic: Active"));
  sleepTimer->Enable();
  voicePrompt->Enable();
  micMonitor->Enable();
  m_headset->send_command(commands::STATUS_REQUEST);
  setTaskIcon();

  // Delay applying saved settings to give the headset time to initialize
  auto settingsTimer = new wxTimer();
  settingsTimer->Bind(wxEVT_TIMER, [this, settingsTimer](wxTimerEvent&) {
    loadAndApplySettings();
    settingsTimer->Stop();
    delete settingsTimer;
  });
  settingsTimer->StartOnce(2000);

  timer->Start(30000);
}

void hyperxFrame::onDisconnect() {
  if (status == connection_status::DISCONNECTED)
    return;
  status = connection_status::DISCONNECTED;
  connectedLabel->SetLabel(_T("Disconnected"));
  sleepTimer->Disable();
  voicePrompt->Disable();
  micMonitor->Disable();
  micMuteLabel->SetLabel(_T("Mic: --"));
  setTaskIcon();
  timer->Stop();
}

void hyperxFrame::read_loop() {
  unsigned char buffer[32];
  while (running) {
    m_headset->read(buffer);
    app->CallAfter([this, &buffer]() {
      if (buffer[0] == 0x21 && buffer[1] == 0xbb) {
        if (debug) {
          printf("HID: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                 buffer[0], buffer[1], buffer[2], buffer[3],
                 buffer[4], buffer[5], buffer[6], buffer[7]);
          fflush(stdout);
        }
        switch (buffer[2]) {
          case 0x03:
            if (buffer[3] == 0x01) {
              onDisconnect();
            } else if (buffer[3] == 0x02) {
              onConnect();
            }
            break;

          // Microphone state (polled, unreliable for mute status)
          case 0x05:
            break;

          // READ SLEEP STATE SETTTING
          case 0x07:
            switch (buffer[3]) {
              case 0x0a:
                sleep = sleep_time::S10;
                sleepTimer->SetSelection(0);
                break;
              case 0x14:
                sleep = sleep_time::S20;
                sleepTimer->SetSelection(1);
                break;
              case 0x1e:
                sleep = sleep_time::S30;
                sleepTimer->SetSelection(2);
                break;
            }
            break;

          // VOICE PROMPTS
          case 0x09:
            if (buffer[3] == 0x01) {
              voice = true;
              voicePrompt->SetValue(true);
            } else if (buffer[3] == 0x00) {
              voice = false;
              voicePrompt->SetValue(false);
            }
            break;

          // Mic monitor state query response
          case 0x0a:
            if (buffer[3] == 0x00) {
              mic_monitor = false;
              micMonitor->SetValue(false);
            } else if (buffer[3] == 0x01) {
              mic_monitor = true;
              micMonitor->SetValue(true);
            }
            break;

          // Battery level
          case 0x0b:
            battery = (unsigned int)buffer[3];
            connectedLabel->SetLabel(
                "Battery: " + std::to_string(battery) + "%" +
                (charging ? " (Charging)" : ""));
            setTaskIcon();
            break;

          // Sidetone level response (read-only)
          case 0x11:
            break;

          // Ping / charging status
          case 0x0c:
            if (buffer[3] == 0x01) {
              charging = true;
            } else {
              charging = false;
            }
            break;

          case 0x0d:
            identifier = (unsigned long)buffer[3] << 40 |
                         (unsigned long)buffer[4] << 32 |
                         (unsigned long)buffer[5] << 24 |
                         (unsigned long)buffer[6] << 16 |
                         (unsigned long)buffer[7] << 8 |
                         (unsigned long)buffer[8];
            break;

          // RESPONSE TO SLEEP TIMER SET
          case 0x12:
            switch (buffer[3]) {
              case 0x0a:
                sleep = S10;
                break;
              case 0x14:
                sleep = S20;
                break;
              case 0x1e:
                sleep = S30;
                break;
            }
            break;

          // VOICE PROMPT RESPONSE
          case 0x13:
            if (buffer[3] == 0x00) {
              voice = false;
            } else if (buffer[3] == 0x01) {
              voice = true;
            }
            break;

          // Mic connected/disconnected (physical)
          case 0x20:
            if (buffer[3] == 0x00) {
              micMuteLabel->SetLabel(_T("Mic: Disconnected"));
            } else if (buffer[3] == 0x01) {
              micMuteLabel->SetLabel(_T("Mic: Active"));
              m_headset->send_command(commands::MICROPHONE_STATE);
            }
            break;

          // Mic monitor response
          case 0x22:
            if (buffer[3] == 0x00) {
              mic_monitor = false;
              micMonitor->SetValue(false);
            } else if (buffer[3] == 0x01) {
              mic_monitor = true;
              micMonitor->SetValue(true);
            }
            break;

          // Mic mute status (real-time from physical button)
          case 0x23:
            if (buffer[3] == 0x00) {
              micMuted = false;
              micMuteLabel->SetLabel(_T("Mic: Active"));
            } else if (buffer[3] == 0x01) {
              micMuted = true;
              micMuteLabel->SetLabel(_T("Mic: Muted"));
            }
            break;

          // POWER OFF
          case 0x24:
            if (buffer[3] == 0x01) {
              onDisconnect();
            } else if (buffer[3] == 0x02) {
              onConnect();
            }
            break;
        }
      }
    });
  }
}

void hyperxFrame::saveSettings() {
  std::ofstream file(m_configPath.mb_str());
  if (file.is_open()) {
    file << "sleep_timer=" << sleepTimer->GetSelection() << "\n";
    file << "voice_prompts=" << (voice ? 1 : 0) << "\n";
    file << "mic_monitor=" << (mic_monitor ? 1 : 0) << "\n";
  }
}

void hyperxFrame::loadAndApplySettings() {
  std::ifstream file(m_configPath.mb_str());
  if (!file.is_open())
    return;

  std::string line;
  int sleepVal = -1;
  int voiceVal = -1;
  int micVal = -1;

  while (std::getline(file, line)) {
    auto pos = line.find('=');
    if (pos == std::string::npos)
      continue;
    std::string key = line.substr(0, pos);
    int val = std::stoi(line.substr(pos + 1));

    if (key == "sleep_timer")
      sleepVal = val;
    else if (key == "voice_prompts")
      voiceVal = val;
    else if (key == "mic_monitor")
      micVal = val;
  }

  if (sleepVal >= 0 && sleepVal <= 2) {
    sleepTimer->SetSelection(sleepVal);
    switch (sleepVal) {
      case 0:
        m_headset->send_command(commands::SLEEP_TIMER_10);
        break;
      case 1:
        m_headset->send_command(commands::SLEEP_TIMER_20);
        break;
      case 2:
        m_headset->send_command(commands::SLEEP_TIMER_30);
        break;
    }
  }

  if (voiceVal == 1)
    m_headset->send_command(commands::VOICE_PROMPTS);
  else if (voiceVal == 0)
    m_headset->send_command(commands::VOICE_PROMPTS_OFF);

  if (micVal == 1)
    m_headset->send_command(commands::MICROPHONE_MONITOR);
  else if (micVal == 0)
    m_headset->send_command(commands::MICROPHONE_MONITOR_OFF);
}
