#ifndef __cl_io_path__
#define __cl_io_path__

#include "../base/9str.h"

namespace cl
{
	//所有路径均使用 / 分隔符, 在Windows, 底层函数会自动转为 \\ 
	class Path
	{
	public:
		static StringW work_dir();
		static StringW exe_dir();

		static void join(StringW& dir, const StrView& sub);
		static void join(StringW& dir, const StrViewW& sub);
		static bool split(const StrViewW& path, StringW& dir, StringW& file_name, StringW& ext);
		//把 ./test_dir/xx.exe 变为 C:/Main_Dir/test_dir/xx.exe
		static StringW abs_path(const StrView& relative_path);
		static StringW abs_path(const StrViewW& relative_path);

	};
}
#endif//__cl_io_path__