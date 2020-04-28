#include "WinTool.h"
#include <windows.h>
#include <stdio.h>

#pragma comment( lib, "user32.lib" )

namespace WinTool {

	void LockSystem()
	{
		LockWorkStation();
	}

}
