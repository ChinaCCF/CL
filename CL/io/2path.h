#ifndef __cl_io_path__
#define __cl_io_path__

#include "../base/9str.h"

namespace cl
{
	//����·����ʹ�� / �ָ���, ��Windows, �ײ㺯�����Զ�תΪ \\ 
	class Path
	{
	public:
		static StringW work_dir();
		static StringW exe_dir();

		static void join(StringW& dir, const StrView& sub);
		static void join(StringW& dir, const StrViewW& sub);
		static bool split(const StrViewW& path, StringW& dir, StringW& file_name, StringW& ext);
		//�� ./test_dir/xx.exe ��Ϊ C:/Main_Dir/test_dir/xx.exe
		static StringW abs_path(const StrView& relative_path);
		static StringW abs_path(const StrViewW& relative_path);

	};
}
#endif//__cl_io_path__