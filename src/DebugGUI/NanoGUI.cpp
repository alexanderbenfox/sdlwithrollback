#include "DebugGUI/NanoGUI.h"
#include "DebugGUI/SimpleDebugWindow.h"

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

bool NanoGUI::Init()
{
  // init nanogui
  try
  {
    nanogui::init();

    /* scoped variables */
    {
      nanogui::ref<SimpleDebugWindow> app = new SimpleDebugWindow(100, 100, "Test");
      app->drawAll();
      app->setVisible(true);
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