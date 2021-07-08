#include <stdio.h> 
#include <CLan/base/8str.h>
#include <CLan/base/10ptr.h>

#include <CLan/set/1base.h>
#include <CLan/set/2arr.h>
#include <CLan/set/3list.h>
#include <CLan/set/4heap.h>
#include <CLan/set/5map.h>
#include <CLan/set/6mix_obj.h>

#include <CLan/tool/xml.h>


#include <locale.h>
namespace test_xml
{

	class AllocMem
	{
	public:
		char* alloc(s64 size) { return (char*)::malloc(size); }
		char* realloc(void* p, s64 size) { return (char*)::realloc(p, size); }
		void free(void* p) { ::free(p); }
	};

	void fun()
	{
		setlocale(LC_ALL, "chs");

		//cl::_MixObj<char, AllocMem> obj;
		//obj.push(1);
		//obj.push(2);
		//int i = 0;
		{
			char buf[256];
			cl::detail::xml_decode_str(buf, "&lt;&gt;&amp;&apos;&quot;123");
			auto ret = cl::CStr::equ(buf, "<>&'\"123");
			int i = 0;
		}
		{
			wchar buf[256];
			cl::detail::xml_decode_str(buf, L"&lt;&gt;&amp;&apos;&quot;123");
			auto ret = cl::CStr::equ(buf, L"<>&'\"123");
			int i = 0;
		}
		{
			char buf[256];
			cl::detail::xml_encode_str(buf, 256, "<>&'\"123");
			auto ret = cl::CStr::equ(buf,"&lt;&gt;&amp;&apos;&quot;123");
			int i = 0;
		}
		{
			wchar buf[256];
			cl::detail::xml_encode_str(buf, 256, L"<>&'\"123");
			auto ret = cl::CStr::equ(buf, L"&lt;&gt;&amp;&apos;&quot;123");
			int i = 0;
		}

		{
			cl::_XML<char, AllocMem> xml;
			xml.parse("<?xml version=\"1.0\" encoding=\"UTF-8\"?><node name=\"ccf\"><!-- comment --><val>123</val><item>str</item><item/></node>");
			int i = 0;
		}
	} 
}