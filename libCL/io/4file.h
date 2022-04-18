#ifndef __cl_io_file__
#define __cl_io_file__
 
#include "1stream.h"
#include "../base/9str.h"

namespace cl
{
	class File : public iStream
	{
	public:
		static bool exist(const StrViewW& path);
		static void del(const StrView& path);

		bool open();
		void close();

		uv32  write(const void* buf, uv32 size);
		uv32  read(uv8* buf, uv32 size);
		  void  flush();
		/*######################################################################*/
		// iStream
		/*######################################################################*/
		virtual bool iStream_open() override { return open(); }
		virtual void iStream_close() override { close(); }

		virtual uv32 iStream_write(const void* buf, uv32 size) override { return write(buf, size); }
		virtual uv32 iStream_read(uv8* buf, uv32 size) override { return read(buf, size); }
		virtual void iStream_flush() override { flush(); }
	};

	
}

#endif//__cl_io_file__