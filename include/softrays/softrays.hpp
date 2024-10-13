// The main include files for this library softrays
#pragma once

namespace softrays {

#if defined(PLATFORM_WEB)
constexpr auto IsWebBuild = true;
#else
constexpr auto IsWebBuild = false;
#endif
}
