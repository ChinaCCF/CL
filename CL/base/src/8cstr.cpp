#include "../8cstr.h"

namespace cl
{
	namespace cstr
	{
		namespace lib
		{
			//1�ֽ� 0xxxxxxx 
			//2�ֽ� 110xxxxx 10xxxxxx //0xC0  1
			//3�ֽ� 1110xxxx 10xxxxxx 10xxxxxx //0xE0  2
			//4�ֽ� 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx //0xF0  3
			//5�ֽ� 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx //0xF8 4
			//6�ֽ� 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx //0xFC 5
			CharCode _get_char_code(const void* _str)
			{
				auto str = (const uv8*)_str;
				while (*str && *str < 0x80) str++;//����ascii���� 
				if (*str == 0) return CharCode::ASCII;//ǰ�涼��ascii���� 

				auto c = *str++;

				//�жϵ�ԭ��: �����ַ����Ƿ����utf8�ı���������ж�
				uv32 remain_cnt = 0;
				do
				{
					if ((c & 0xE0) == 0xC0) { remain_cnt = 1; break; }
					if ((c & 0xF0) == 0xE0) { remain_cnt = 2; break; }
					if ((c & 0xF8) == 0xF0) { remain_cnt = 3; break; }
					if ((c & 0xFC) == 0xF8) { remain_cnt = 4; break; }
					if ((c & 0xFE) == 0xFC) { remain_cnt = 5; break; }
					return CharCode::GBK;
				} while (false);

				for (uv32 i = 0; i < remain_cnt; i++)
				{
					c = str[i];
					if ((c & 0xC0) != 0x80) return CharCode::GBK;
				}
				return CharCode::UTF8;
			}

			//ת��������, ����������Ϊ��
			template<CharType T, FloatType V>
			static inline FloatStrInfo<T> _do_from_val(T* buf, uv32 size, V fval, uv32 fraction_cnt)
			{
				FloatStrInfo<T> ret;
				ret.str_ = nullptr;

				auto p = buf;

				uv64 uval = (uv64)fval;
				ret.number_cnt_ = _from_val(p, size, uval);

				if (ret.number_cnt_ == 0) return ret;
				//                          '.'            �ַ�����β
				if (size < ret.number_cnt_ + 1 + fraction_cnt + 1) return ret;

				auto cnt = fraction_cnt;
				{//����С��
					fval -= uval;
					while (cnt--) fval *= 10;

					uval = (uv64)fval;
					fval -= uval;
					if (fval > 0.5) uval += 1;
				}

				p += ret.number_cnt_;
				*p++ = '.';

				cnt = _from_val(p, size - ret.number_cnt_ - 1, uval);
				auto dif = fraction_cnt - cnt;
				rshift(p, cnt, dif);

				for (uv32 i = 0; i < dif; i++)
					p[i] = '0';

				ret.fraction_cnt_ = fraction_cnt;
				ret.str_ = buf;
				return ret;
			}
			 
			FloatStrInfo<uc8> _from_val(uc8* buf, uv32 size, fv64 fval, uv32 fraction_cnt)
			{
				return _do_from_val(buf, size, fval, fraction_cnt);
			}
			FloatStrInfo<uc16> _from_val(uc16* buf, uv32 size, fv64 fval, uv32 fraction_cnt)
			{
				return _do_from_val(buf, size, fval, fraction_cnt);
			}

			//��С���ַ���תΪ������, ֧�ֿ�ѧ������
			template<CharType T>
			static inline T* _do_2_fval(const T* str, fv64& val)
			{
				val = 0;

				uv64 uv;
				auto end = _2_val(str, uv);
				if (end == nullptr) return nullptr;

				val = (fv64)uv;

				str = end;
				if (*str == '.')
				{
					++str;
					end = _2_val(str, uv);
					if (end)
					{//������ΪС�� 
						fv64 fv = (fv64)uv;
						auto cnt = uv32(end - str);
						while (cnt--) fv /= 10;
						val += fv;
						str = end;
					}
					else
						return nullptr;
				}
				if (*str == 'e' || *str == 'E')
				{
					++str;
					sv64 sv;
					end = to_val(str, sv);
					if (end)
					{
						if (val < -10 || val > 10) CL_Throw(ExceptionCode::Float_Format_Error, 0, "float type erro!");
						if (sv > 0) while (sv > 0) { val *= 10; --sv; }
						else while (sv < 0) { val /= 10; ++sv; }
						str = end;
					}
					else
					{
						//��ǿ��С�������eΪ����
						--str;
					}
				}
				return (T*)str;
			}
			 
			void* _2_fval(const uc8* str, fv64& val) { return (void*)_do_2_fval(str, val); }
			void* _2_fval(const uc16* str, fv64& val) { return (void*)_do_2_fval(str, val); }
		} 
	}
}