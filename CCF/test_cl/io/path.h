#include <CL/io/2path.h>

namespace test
{ 
	void test_path()
	{
		auto exe = cl::Path::exe_dir();
		auto word = cl::Path::work_dir();

		cl::StringW dir, file, ext;
		cl::Path::split(exe, dir, file, ext);

		cl::StringW join = "C:/abc";
		cl::Path::join(join, "abc");

		cl::StringW join2 = "C:/abc/";
		cl::Path::join(join2, "abc");

		cl::StringW join3 = "C:/abc";
		cl::Path::join(join3, "/abc");

		cl::StringW join4 = "C:/abc/";
		cl::Path::join(join4, "/abc");

		//auto abs = cl::Path::abs_path("../abc");
		//auto abs2 = cl::Path::abs_path("/abc");
		//auto abs3 = cl::Path::abs_path("./abc");
		//auto abs4 = cl::Path::abs_path("./abc/../efg");
		auto abs5 = cl::Path::abs_path("../../abc");
		int i = 0;
	}
}