#include "stdafx.h"
#include "GameApp.h"
//-----------------------------------------------------------------------------
#if defined(_MSC_VER)
#	pragma comment( lib, "Engine.lib" )
#endif
//-----------------------------------------------------------------------------
int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
	EngineDeviceCreateInfo createInfo;
	createInfo.window.maximized = false;
	//createInfo.window.width = 1280;
	//createInfo.window.height = 960;
	createInfo.window.vsyncEnabled = true;
	auto engineDevice = EngineDevice::Create(createInfo);
	engineDevice->RunApp(std::make_shared<GameApp>());
}
//-----------------------------------------------------------------------------