// TestDism.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Dism/dism.h>
#include <libCL/io/3print.h>
#include <libCL/set/3list.h>

int main()
{
	cl::StringW pre_path = L"C:\\Users\\ccf\\Desktop\\iso\\";

	//{//先用wim api 删除多余版本, 只保留专业版 
	//	auto wim_path = pre_path;
	//	wim_path << L"install.wim";
	//	//wim_path << L"1.wim";

	//	auto wim = cl::Wim::open(wim_path.data());
	//	if (wim.valid())
	//	{
	//		auto tmp = pre_path; tmp << L"tmp";
	//		wim.set_tmp_dir(tmp.data());//临时目录要提前创建
	//		wim.print_info();

	//		wim.del_sub(5);
	//		wim.del_sub(5);
	//		wim.del_sub(1);
	//		wim.del_sub(1);
	//		wim.del_sub(1);

	//		int arr[] = { 1 };
	//		auto ret_wim = pre_path; ret_wim << L"1.wim";  tmp << L"2";
	//		wim.rebuild(ret_wim.data(), tmp.data(), arr, 1);//临时目录要提前创建
	//	}
	//}

	int i = 0;
	{//再挂在wim, 删除app, 和不要feature和Capability
		//在删除app和feature和capability时, 会删除对应的package
		auto log = pre_path; log << L"log.txt";
		auto tmp = pre_path; tmp << L"tmp";
		auto tmp2 = pre_path; tmp2 << L"tmp2";
		auto wim = pre_path; wim << L"1.wim";

		cl::Dism::init(log.data(), tmp.data());//临时目录要提前创建 
		//cl::Dism::mount(wim.data(), tmp2.data(), 1);

		{
			auto dism = cl::Dism::open(tmp2.data());


			cl::List<cl::StringW> list;

			//{//移除app  
			//	list.clear();
			//	dism.enum_app([&list](cl::ItemInfo* info)->void 
			//	{
			//		list.push_back(info->name_);
			//	});

			//	for (auto& p : list)
			//	{
			//		if(p != L"Microsoft.StorePurchaseApp_11811.1001.1813.0_neutral_~_8wekyb3d8bbwe" && 
			//		   p != L"Microsoft.VCLibs.140.00_14.0.27323.0_x64__8wekyb3d8bbwe"&&
			//		   p != L"Microsoft.WindowsCalculator_2020.1906.55.0_neutral_~_8wekyb3d8bbwe" &&
			//		   p != L"Microsoft.WindowsStore_11910.1002.513.0_neutral_~_8wekyb3d8bbwe" )
			//			dism.remove_app(p.data());
			//	}
			//	 
			//	dism.commit();
			//}

			//{//Capability 
			//	list.clear();
			//	dism.enum_Capabilities([&list](cl::ItemInfo* info)->void
			//	{
			//		list.push_back(info->name_);
			//	});

			//	for (auto& p : list)
			//	{
			//		if (p != L"DirectX.Configuration.Database~~~~0.0.1.0" &&
			//			p != L"Language.Basic~~~zh-CN~0.0.1.0" &&
			//			p != L"Language.Fonts.Hans~~~und-HANS~0.0.1.0" &&
			//			p != L"Microsoft.Windows.MSPaint~~~~0.0.1.0" &&
			//			p != L"Microsoft.Windows.Notepad~~~~0.0.1.0" &&
			//			p != L"Windows.Client.ShellComponents~~~~0.0.1.0")
			//			dism.remove_Capability(p.data());
			//	}
			//	dism.commit();
			//}

			//{//Features 
			//	list.clear();
			//	dism.enum_Features([&list](cl::ItemInfo* info)->void
			//	{
			//		list.push_back(info->name_);
			//	});

			//	for (auto& p : list)
			//	{
			//		if (p != L"Printing-PrintToPDFServices-Features" && 
			//			p != L"Windows-Defender-Default-Definitions" &&
			//			p != L"Printing-Foundation-Features" &&
			//			p != L"Printing-Foundation-InternetPrinting-Client" &&
			//			p != L"MicrosoftWindowsPowerShellV2Root"&&
			//			p != L"MicrosoftWindowsPowerShellV2" )
			//			dism.remove_Feature(p.data());
			//	}
			//	dism.commit();
			//}


			{//Package 
				dism.remove_Package(L"Microsoft-OneCore-DirectX-Database-FOD-Package~31bf3856ad364e35~amd64~~10.0.19041.1");
				dism.remove_Package(L"Microsoft-Windows-UserExperience-Desktop-Package~31bf3856ad364e35~amd64~~10.0.19041.1503");
				dism.commit();
			}

			{
				dism.enum_app([](cl::ItemInfo* info)->void {});
				dism.enum_Capabilities([](const cl::ItemInfo* info)->void {});
				dism.enum_Features([](const cl::ItemInfo* info)->void {});
				dism.enum_Packages([](cl::PackageInfo* info)->void {});
			}

		}

		cl::Dism::unmount(tmp2.data());
		cl::Dism::uninit();
		int i = 0;
	}
}

