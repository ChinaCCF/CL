#include <Windows.h>
#include "../2path.h"

namespace cl
{
    StringW Path::work_dir()
    {
		constexpr uv32 path_len = 1024;
		StringW ret;
	Label_Redo:
		ret.reserve(path_len);
		auto len = GetCurrentDirectoryW(path_len, (wchar*)ret.data());
		if (len == 0)
		{
			ret.clear();
			return ret;
		}
		if (len > path_len)
		{
			ret.reserve(len + 1);
			goto Label_Redo;
		}

        ret.length(len);
        ret.replace('\\', '/');
        ret << '/'; //window 最后一位不为分隔符
		return ret;
    }

    StringW Path::exe_dir()
    {
        constexpr uv32 path_len = 1024;
        StringW ret;
        ret.reserve(path_len); 
        auto len = GetModuleFileNameW(nullptr, (wchar*)ret.data(), path_len);
        if (len == 0) ret.clear(); 

        ret.length(len);
        ret.replace('\\', '/');
        return ret;
    }

    void Path::join(StringW& dir, const StrViewW& sub)
    {
        if(dir.back() != '/') dir << '/';
        auto p = sub.data();
        while (*p == '/') ++p;
        dir << p;
    }

    void Path::join(StringW& dir, const StrView& sub)
    {
        StringW tmp = sub;
        join(dir, tmp);
    }

	bool Path::split(const StrViewW& path, StringW& dir, StringW& file_name, StringW& ext)
	{ 
        dir.clear(); file_name.clear(); ext.clear();

        sv32 index = path.findr('/'); 
        // 目录路径无法分解, 例如 C://Test/
        if (index == -1 || index == path.length() - 1) return false; 
         
        auto p = path.data();
        dir.append(p, index + 1);

        p += index + 1;
        auto dot = cstr::findr(p, '.');
        if (dot)
        {
            file_name.append(p, uv32(dot - p));
            ext.append(dot + 1);
        }
        else 
            file_name.append(p);

        return true;
	}

    StringW Path::abs_path(const StrViewW& relative_path)
    {
        auto ret = work_dir();
        ret.data()[ret.length() - 1] = 0;

        auto src = relative_path.data();
        auto dst = ret.data();

        while (src[0] == '/') ++src;
        if (src[0] == '.' && src[1] == '/') src += 2;
          
        uc16* p = nullptr;
        while (src[0] == '.' && src[1] == '.' && src[2] == '/')//删减父目录对
        {
            src += 3; 
            p = cstr::findr(dst, '/');
            if (!p) return CL_TXT16("");
            *p = 0; 
        }  
        {//防止 ../abc/../efd/a.exe  这种格式
            auto tp = cstr::find(src, "../");
            if (tp) return CL_TXT16("");
        }
        if (p)
        {
            *p++ = '/'; *p = 0;
            ret.length(uv32(p - dst));
        }
        else
            ret.data()[ret.length() - 1] = '/';

        ret << src;
        return ret;
    }
    StringW Path::abs_path(const StrView& relative_path)
    {
        StringW tmp = relative_path;
        return abs_path(tmp);
    }

}