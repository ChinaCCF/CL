#include "../dism.h"
#include <Windows.h>
#include <wimgapi.h>

#include <libCL/io/3print.h>
#include <libCL/set/2arr.h>
 
namespace cl
{ 
	Wim Wim::open(const StrViewW& wim_path)
	{
		Wim ret = WIMCreateFile(wim_path.data(),
								WIM_GENERIC_READ | WIM_GENERIC_WRITE | WIM_GENERIC_MOUNT,
								WIM_OPEN_EXISTING,
								WIM_FLAG_SHARE_WRITE,
								0, //dwCompressionType
								nullptr);//pdwCreationResult 
		if (ret.valid())
			ret.wim_path_ = wim_path;
		else
			CPrint << "WIMCreateFile fail : " << GetLastError();
		return ret;
	}

	bool Wim::set_tmp_dir(const wchar* tmp)
	{
		tmp_path_ = tmp;
		if (!WIMSetTemporaryPath(wim_, tmp))
		{
			CPrint << "WIMSetTemporaryPath fail : " << GetLastError();
			return false;
		}
		return true;
	}

	bool Wim::del_sub(uv32 index)
	{
		if (WIMDeleteImage(wim_, index))
		{
			CPrint << "WIMDeleteImage : " << index << " success!";
			return true;
		}
		else
		{
			CPrint << "WIMDeleteImage fail : " << GetLastError();
			return false;
		}
	}
	bool Wim::rebuild(const StrViewW& _dst, const wchar* tmp, int* arr_index, int cnt)
	{
		Wim dst = WIMCreateFile(_dst.data(),
								WIM_GENERIC_READ | WIM_GENERIC_WRITE | WIM_GENERIC_MOUNT,
								WIM_CREATE_ALWAYS,
								WIM_FLAG_SHARE_WRITE,
								WIM_COMPRESS_LZX,
								nullptr);

		if (!dst.valid()) return false;

		if (!WIMSetTemporaryPath(dst, tmp))
		{
			CPrint << "WIMSetTemporaryPath fail : " << GetLastError();
			return false;
		}

		for (int i = 0; i < cnt; i++)
		{
			Ptr<HANDLE, Fun_WIMCloseHandle> tmp = WIMLoadImage(wim_, arr_index[i]);
			if (!tmp.valid())
			{
				CPrint << "WIMLoadImage fail : " << GetLastError();
				continue;
			}
			if (WIMExportImage(tmp, dst, WIM_EXPORT_ALLOW_DUPLICATES))
			{
				CPrint << "WIMExportImage : " << arr_index[i] << " success!";
			}
			else
			{
				CPrint << "WIMExportImage : " << arr_index[i] << " fail1 " << GetLastError();
			}
		}
		return true;
	}

	void Wim::print_info()
	{
		{
			auto cnt = WIMGetImageCount(wim_);
			cl::Print() << "WIMGetImageCount (sub image version) : " << cnt;
		}

		{
			WIM_INFO info;
			if (!WIMGetAttributes(wim_, &info, sizeof(WIM_INFO))) 
				CPrint << "WIMGetAttributes fail : " << GetLastError();

			switch (info.CompressionType)
			{
			case WIM_COMPRESS_NONE: cl::Print() << "WIM_COMPRESS_NONE"; break;
			case WIM_COMPRESS_XPRESS: cl::Print() << "WIM_COMPRESS_XPRESS"; break;
			case WIM_COMPRESS_LZX: cl::Print() << "WIM_COMPRESS_LZX"; break;
			case WIM_COMPRESS_LZMS: cl::Print() << "WIM_COMPRESS_LZMS"; break;
			default:
				break;
			} 

			if (info.WimAttributes & WIM_ATTRIBUTE_NORMAL) cl::Print() << "WIM_ATTRIBUTE_NORMAL";
			if (info.WimAttributes & WIM_ATTRIBUTE_RESOURCE_ONLY) cl::Print() << "WIM_ATTRIBUTE_RESOURCE_ONLY";
			if (info.WimAttributes & WIM_ATTRIBUTE_METADATA_ONLY) cl::Print() << "WIM_ATTRIBUTE_METADATA_ONLY";
			if (info.WimAttributes & WIM_ATTRIBUTE_VERIFY_DATA) cl::Print() << "WIM_ATTRIBUTE_VERIFY_DATA";
			if (info.WimAttributes & WIM_ATTRIBUTE_RP_FIX) cl::Print() << "WIM_ATTRIBUTE_RP_FIX";
			if (info.WimAttributes & WIM_ATTRIBUTE_SPANNED) cl::Print() << "WIM_ATTRIBUTE_SPANNED";
			if (info.WimAttributes & WIM_ATTRIBUTE_READONLY) cl::Print() << "WIM_ATTRIBUTE_READONLY";
			 
			if (info.WimFlagsAndAttr & WIM_FLAG_VERIFY) cl::Print() << "WIM_FLAG_VERIFY";
			if (info.WimFlagsAndAttr & WIM_FLAG_SHARE_WRITE) cl::Print() << "WIM_FLAG_SHARE_WRITE"; 
		}

		{
			void* p = nullptr;
			DWORD size = 8192;

			if (WIMGetImageInformation(wim_, &p, &size))
			{
				cl::Print() << "WIMGetImageInformation success : \n" << (wchar*)p;
				LocalFree(p);
			}
			else
				CPrint << "WIMGetAttributes fail : " << GetLastError();
		}

	}
}