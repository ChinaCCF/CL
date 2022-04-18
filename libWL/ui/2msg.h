#ifndef __wl_ui_msg__
#define __wl_ui_msg__

#include "../base/1type.h"

namespace wl
{ 
	class Message
	{
	public: 
		static void loop();
		static const wchar* to_str(uv32 msg);
	 };
}

#endif//__wl_ui_window__