#ifndef __cl_io_stream__
#define __cl_io_stream__

#include "../base/2type.h"
#include "../base/3type_traits.h"
#include "../base/_exception_code.h"
#include "../base/9str.h"

namespace cl
{
	class iStream
	{
	public:
		virtual bool iStream_open() = 0;
		virtual void iStream_close() = 0;

		virtual uv32 iStream_write(const void* buf, uv32 size) = 0;
		virtual uv32 iStream_read(uv8* buf, uv32 size) = 0;
		virtual void iStream_flush() = 0; 
	};

	class RawStreamHelper
	{
		iStream* stream_ = nullptr;
	public:
		RawStreamHelper(iStream* stream) : stream_(stream) {}

		void flush() { stream_->iStream_flush(); }

		template<typename T>
			requires (IsInt_v<T> || IsBool_v<T> || IsFloat_v<T>)
		RawStreamHelper& operator<<(const T& val)
		{
			auto cnt = stream_->iStream_write(&val, sizeof(T));
			if (cnt != sizeof(T)) CL_Throw(ExceptionCode::IO_Write_Fail, 0, "");
			return *this;
		}

		template<typename T>
			requires (IsInt_v<T> || IsBool_v<T> || IsFloat_v<T>)
		RawStreamHelper& operator>>(T& val)
		{
			auto cnt = stream_->iStream_read(&val, sizeof(T));
			if (cnt != sizeof(T)) CL_Throw(ExceptionCode::IO_Read_Fail, 0, "");
			return *this;
		}

		template<CharType T> 
		RawStreamHelper& operator<<(const lib::_StrView<T>& val)
		{
			auto size = sizeof(T) * val.length();
			auto cnt = stream_->iStream_write(val.data(), size);
			if (cnt != size) CL_Throw(ExceptionCode::IO_Write_Fail, 0, "");
			return *this;
		}

		template<CharType T, class MA, uv32 N>
		RawStreamHelper& operator<<(const lib::_NString<T, MA, N>& val)
		{
			return this->operator<<(val.view());
		}

		template<CharType T, class MA, uv32 N>
		RawStreamHelper& operator>>(lib::_NString<T, MA, N>& val)
		{
			val.length(0); 
			T c; 
			do
			{
				auto cnt = stream_->iStream_read(&c, sizeof(T));
				if (cnt == 0 || c == 0) break;
				val << c;
			} while (true);
			return *this;
		}
	};
}

#endif//__cl_io_stream__