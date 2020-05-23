#include "DebugGUI/NanoGUI.h"

#if defined(_WIN32)
#  pragma warning(push)
#  pragma warning(disable: 4457 4456 4005 4312)
#endif

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#if defined(_WIN32)
#  pragma warning(pop)
#endif


#if defined(_WIN32)
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#  include <windows.h>
#endif

void NanoGUI::BeginGUIThread(std::thread& guiThread)
{
  guiThread = std::thread([this]()
    {
      InitGUIWindows();
    });
}

bool NanoGUI::InitGUIWindows()
{
  // init nanogui
  try
  {
    nanogui::init();

    // add window
    _windows.insert(std::make_pair("Test", nanogui::ref(new SimpleDebugWindow(100, 100, "Test"))));

    /* scoped variables */
    {
      for (auto& app : _windows)
      {
        app.second->drawAll();
        app.second->setVisible(true);
      }
      nanogui::mainloop();
    }
    nanogui::shutdown();
  }
  catch (const std::runtime_error& e)
  {
    std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
#if defined(_WIN32)
    MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
#else
    std::cerr << error_msg << endl;
#endif
    return false;
  }
  return true;
}