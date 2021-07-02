#ifndef __clan_tool_json__
#define __clan_tool_json__

#include "../set/6mix_obj.h"

namespace cl
{
	//json 解析类, 非常简单明了
	template<CharType C, AllocMemType A>
	class _Json
	{ 
		C* org_buf_ = nullptr;//解析过程中用到的缓冲
		C* buf_ = nullptr;//随着递归解析, buf_指针会递增
		bool format_ = true;
	public:
		using Str = _String<C, A, 0>;
		using MO = _MixObj<C, A>;
		MO root_;
		s32 fraction_ = 2;//默认小数位2位
	public:
		C* _parse_str(const C* p, C* buf, s32& len)
		{
			p++;//跳过当前'"' 
			auto start = p;
			while (true)
			{
				while (*p && *p != '"') p++;
				cl_assert(*p != 0);

				if (*(p - 1) != '\\')
					break;
				p += 1;
			}

			len = s32(p - start);
			CStr::copy(buf, start, len);
			return (C*)(p + 1);
		}

		C* _parse_num(const C* str, detail::Str2Val& val)
		{
			auto ret = detail::_str2val(str, val);
			cl_assert(ret != nullptr);
			return ret;
		}

		enum class OtherVal
		{
			None,
			Null,
			True,
			False
		};

		C* _parse_other_val(const C* str, OtherVal& val)
		{
			C buf[6];
			CStr::copy(buf, 6, str, 6);
			buf[5] = 0;
			CStr::lower(buf);
			if (CStr::equ(buf, "false", 5))
			{
				val = OtherVal::False;
				return (C*)(str + 5);
			}
			if (CStr::equ(buf, "true", 4))
			{
				val = OtherVal::True;
				return (C*)(str + 4);
			}
			if (CStr::equ(buf, "null", 4))
			{
				val = OtherVal::Null;
				return (C*)(str + 4);
			}
			cl_assert(false);
			return nullptr;
		}

		C* _skip_space(const C* str)
		{
			while (*str && *str <= 32) ++str;
			return (C*)str;
		}

		C* _parse_arr(const C* str, MO& arr)
		{
			bool more_item = false;
			str += 1;//跳过 '[' 
			while (true)
			{
				str = _skip_space(str);
				cl_assert(*str != 0);
				if (*str == ']')
				{
					cl_assert(more_item == false);
					return (C*)(str + 1);
				}

				{
					MO node;
					str = _parse_sub(str, node);
					arr.push(std::move(node));

					more_item = false;
				}

				str = _skip_space(str);//跳过\r\n
				if (*str == ',')
				{
					str++;
					more_item = true;
				}
			};
		}

		C* _parse_map(const C* str, MO& map)
		{
			bool more_item = false;
			str += 1;//跳过'{' 

			while (true)
			{
				str = _skip_space(str);
				cl_assert(*str != 0);
				if (*str == '}')
				{
					cl_assert(more_item == false);
					return (C*)(str + 1);
				}

				{
					cl_assert(*str == '"');

					s32 len = 0;
					str = _parse_str(str, buf_, len);
					auto key = buf_;
					buf_ += len + 1;

					str = _skip_space(str);
					cl_assert(*str == ':');
					str++;
					str = _skip_space(str);

					MO node;
					str = _parse_sub(str, node);
					map[key] = std::move(node);
					buf_ = key;

					more_item = false;
				}

				str = _skip_space(str);//跳过\r\n
				if (*str == ',')
				{
					str++;
					more_item = true;
				}
			};
		}

		C* _parse_sub(const C* str, MO& obj)
		{
			cl_assert(*str != 0);
			//解析数组
			if (*str == '[') return _parse_arr(str, obj);
			//解析字典
			if (*str == '{') return _parse_map(str, obj);
			//解析字符串
			if (*str == '\"')
			{
				s32 len = 0;
				str = _parse_str(str, buf_, len);
				obj = buf_;
				return (C*)str;
			}
			//解析数字 
			if (*str == '-' || (*str >= 0 && *str <= '9'))
			{
				detail::Str2Val val; 
				str = _parse_num(str, val);
				if (val.is_float_)
					obj = (f64)val.val_;
				else
					obj = (s64)val.val_;
				return (C*)str;
			}
			OtherVal val;
			str = _parse_other_val(str, val);
			if (val == OtherVal::Null)
				obj = nullptr;
			if (val == OtherVal::False)
				obj = false;
			if (val == OtherVal::True)
				obj = true;
			return (C*)str;
		}

		/*############################################################################*/
		void _push_str(Str& str, const char* con)
		{
			while (*con) str.push(*con++);
		}

		void _pad_prefix(Str& str, s32 level)
		{
			for (s32 i = 0; i < level; i++)
				_push_str(str, "  ");
		}

		void _dump_obj(Str& str, const MO& obj, s32 level)
		{
			if (obj.is_list())
				_dump_arr(str, obj, level + 1);
			else
			{
				if (obj.is_map())
					_dump_map(str, obj, level + 1);
				else
				{
					if (obj.is_bool())
						str << (bool)obj;
					else
					{
						if (obj.is_float())
							str << (f64)obj;
						else
						{
							if (obj.is_int())
								str << (s64)obj;
							else
							{
								if (obj.is_str())
								{
									str << '\"' << (const C*)obj << '\"';
								}
								else
									str << nullptr;
							}
						}
					}
				}
			}
		}

		void _dump_arr(Str& str, const MO& obj, s32 level)
		{
			auto list = obj.list();
			if (list->size())
			{
				{
					if (format_)
					{
						_push_str(str, "\r\n");
						_pad_prefix(str, level);
					}

					str.push('[');

					if (format_) _push_str(str, "\r\n");
				}

				for (auto& item : *list)
				{
					if (format_) _pad_prefix(str, level + 1);
					_dump_obj(str, item, level + 1);

					if (format_) _push_str(str, ",\r\n");
					else str.push(',');
				}

				{
					if (format_)
					{
						str.pop(3);//,\r\n 
						_push_str(str, "\r\n");
						_pad_prefix(str, level);
						str.push(']');
					}
					else
					{
						str.pop(1);
						str.push(']');
					}
				}
			}
			else
				_push_str(str, "[]");
		}

		void _dump_map(Str& str, const MO& obj, s32 level)
		{
			auto map = obj.map();
			if (map->size())
			{
				{
					if (format_)
					{
						_push_str(str, "\r\n");
						_pad_prefix(str, level);
					}

					str.push('{');

					if (format_) _push_str(str, "\r\n");
				}

				for (auto& item : *map)
				{
					if (format_) _pad_prefix(str, level + 1);

					str << '\"' << item.first << '\"' << ':';
					_dump_obj(str, item.second, level + 1);

					if (format_) _push_str(str, ",\r\n");
					else str.push(',');
				}
				{
					if (format_)
					{
						str.pop(3);//,\r\n 
						_push_str(str, "\r\n");
						_pad_prefix(str, level);
						str.push('}');
					}
					else
					{
						str.pop(1);
						str.push('}');
					}
				}
			}
			else
				_push_str(str, "{}");
		}
	public:
		_Json() { org_buf_ = buf_ = (C*)A().alloc(sizeof(C) * 4096); }
		~_Json() { A().free(org_buf_); }

		template<typename T> requires ToChars<T, C>::value
		void parse(const T& _str)
		{
			buf_ = org_buf_;

			auto str = (const C*)_str;
			str = _skip_space(str);
			cl_assert(*str == '{' || *str == '[');

			if (*str == '{')
				str = _parse_map(str, root_);
			else
				str = _parse_arr(str, root_);

			str = _skip_space(str);
			cl_assert(*str == 0);
		}
		 
		Str dump(bool format = true)
		{
			format_ = format;

			Str ret;
			ret << Fraction(fraction_);
			_dump_obj(ret, root_, -1);
			return ret;
		}

		void operator<<(const Fraction& fra) { fraction_ = fra.len_; }
	};
}

#endif//__clan_tool_json__