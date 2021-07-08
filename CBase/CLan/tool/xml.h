#ifndef __clan_tool_xml__
#define __clan_tool_xml__

#include "../set/2arr.h"
#include "../set/6mix_obj.h"


namespace cl
{
	namespace detail
	{
		s32 xml_decode_str(char* dst, const char* src);
		s32 xml_decode_str(wchar* dst, const wchar* src);

		s32 xml_encode_str(char* dst, s32 size, const char* src);
		s32 xml_encode_str(wchar* dst, s32 size, const wchar* src);

#define _CL_XML_Attr "_cl_xml_attr"
		static inline char* _xml_attr_key(const char*) { return (char*)_CL_XML_Attr; }
		static inline wchar* _xml_attr_key(const wchar*) { return (wchar*)_cl_W(_CL_XML_Attr); }

#define _CL_XML_Text "_cl_xml_text"
		static inline char* _xml_text_key(const char*) { return (char*)_CL_XML_Text; }
		static inline wchar* _xml_text_key(const wchar*) { return (wchar*)_cl_W(_CL_XML_Text); }

#define _CL_XML_Com "_cl_xml_com"
		static inline char* _xml_com_key(const char*) { return (char*)_CL_XML_Com; }
		static inline wchar* _xml_com_key(const wchar*) { return (wchar*)_cl_W(_CL_XML_Com); }
	}

	// 大小写敏感
	// 有根元素, 意思就是只有一个根元素
	// 属性值必须要双引号或单引号
	template<CharType C, AllocMemType A>
	class _XML
	{
	public:
		using Str = _String<C, A, 0>;
		using MO = _MixObj<C, A>;
	private:
		C* buf_ = nullptr;//解析过程中用到的缓冲 
		bool format_ = true;//是否格式化字符串 
		MO comment_;//注释
		FixedArr<MO*, 256> stack_;
	public:
		MO root_;
		s32 fraction_ = 2;//默认小数位2位 

	private:
		C* _skip_space(const C* str)
		{
			while (*str && *str <= 32) ++str;
			return (C*)str;
		}

		//解析属性
		void _parse_attr(C* str, MO& obj)
		{
			auto& attr = obj[detail::_xml_attr_key(str)];
			while (true)
			{
				str = _skip_space(str);
				if (*str == 0) return;

				auto end = CStr::find(str, '=');
				cl_assert(end != nullptr);

				*end = 0;
				CStr::trim(str, s32(end - str));
				auto key = str;

				str = _skip_space(end + 1);

				u8 start_c = *str++;//属性开始字符可以为\"或\'
				cl_assert(start_c == '\'' || start_c == '"');
				while (true)
				{
					end = CStr::find(str, start_c);
					cl_assert(end != nullptr);
					if (*(end - 1) != '\\') break;
					str = end + 1;
				}

				*end = 0;
				CStr::trim(str, s32(end - str));

				attr[key] = str;
				str = end + 1;

				int i = 0;
			}
		}

		//解析文件信息
		void _parse_info(C* str)
		{
			//str : "<?xml...?>" 
			cl_assert(CStr::equ(str, "xml ", 4));
			str += 4;
			_parse_attr(str, root_);
		}

		//这个函数用于支持重复的项
		void _get_node(MO* parent, const C* _name, MO*& obj, Str*& name)
		{
			auto& pa = *parent;
			cl_assert(pa.is_map() || pa.is_null() || pa.is_none());

			auto node = pa.map()->get(_name);
			auto& last = node->second;

			name = &node->first;

			if (last.is_null() || last.is_none())
			{
				obj = &last;
			}
			else
			{
				MO tmp;
				if (last.is_list()) 
					last.push(std::move(tmp));  
				else
				{//不是数组  
					MO arr;
					arr.push(std::move(last));
					arr.push(std::move(tmp));
					last = std::move(arr); 
				}

				auto list = last.list();
				obj = &list->back();
			}
		}


		//1 : <? ... ?>
		//2 : 注释   
		//3 : 正常节点开始 #  1 : <...> ... </...>
		//                    2 : <.../>
		C* _parse_node(const C* str, s32 level)
		{
			do
			{
				auto p = str + 1;//跳过'<'

				auto bracket_end = CStr::find(p, '>');
				cl_assert(bracket_end != nullptr);

				if (p[0] == '?')
				{//跳过xml文件编码信息
					cl_assert(level == 0);//<?xml ...?>, 必定是在最顶层, 同root同级别

					p += 1; //跳过?
					auto pe = detail::CStrX::find(p, "?>");
					cl_assert(pe + 1 == bracket_end);

					auto len = s32(pe - p);
					CStr::copy(buf_, p, len);
					_parse_info(buf_);

					str = _skip_space(bracket_end + 1);
					continue;
				}

				if (p[0] == '!' && p[1] == '-' && p[2] == '-')
				{
					p += 3; //跳过注释!--
					auto pe = detail::CStrX::find(p, "-->");
					cl_assert(pe + 2 == bracket_end);

					CStr::copy(buf_, p, s32(pe - p));
					MO node;
					node = buf_;
					comment_.push(std::move(node));

					str = _skip_space(bracket_end + 1);
					continue;
				}

				str = bracket_end + 1;
				Str* name;
				MO* node;

				{
					CStr::copy(buf_, p, s32(bracket_end - p));
					auto node_end = CStr::find(buf_, '/');
					if (node_end) *node_end = 0;

					auto attr = CStr::find(buf_, ' ');
					if (attr) *attr = 0; 

					auto parent = stack_.back();
					_get_node(parent, buf_, node, name);
					if(comment_.is_list())
						(*node)[detail::_xml_com_key(buf_)] = std::move(comment_);

					if (attr) _parse_attr(attr + 1, *node);

					if (node_end) return (C*)_skip_space(bracket_end + 1);
				}

				p = CStr::find(str, '<');
				cl_assert(p);

				if (p[1] == '/')
				{
					CStr::copy(buf_, str, s32(p - str));
					(*node)[detail::_xml_text_key(buf_)] = buf_; 
				}
				else
				{
					stack_.push(node);
					while (true)
					{
						str = _parse_node(p, level + 1);
						p = CStr::find(str, '<');
						cl_assert(p);
						if (p[1] == '/') break; 
					} 
					stack_.pop(); 
				}

				p += 2; //跳过</
				bracket_end = CStr::find(p, '>');
				cl_assert(bracket_end != nullptr);

				CStr::copy(buf_, p, s32(bracket_end - p));
				cl_assert(*name == buf_);

				return (C*)_skip_space(bracket_end + 1);
			} while (true);
		}
	public:
		_XML() { buf_ = (C*)A().alloc(sizeof(C) * 4096); }
		~_XML() { A().free(buf_); }
		//解析字符串, 非文件
		template<typename T> requires ToChars<T, C>::value
			void parse(const T& _str)
		{
			auto str = (const C*)_str;
			str = _skip_space(str);
			cl_assert(*str == '<');

			stack_.push(&root_);
			_parse_node(str, 0);
		}
		//string dump(bool format = true);
	};
}

#endif//__clan_tool_xml__