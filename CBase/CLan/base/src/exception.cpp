#include "../5exception.h"
#include "../8str.h"

namespace clan
{
    Exception::Exception(const Exception& e)
    {
        line_ = e.line_;
        code_ = e.code_;
        Str::copy(file_, 256, e.file_);
        Str::copy(msg_, 256, e.msg_);
    } 
    Exception::Exception(const wchar* file, s32 line, s32 main_code, s32 sub_code, const wchar* msg)
    {
        line_ = line;
        code_ = (main_code << 16) | sub_code;
        Str::copy(file_, 256, file);
        Str::copy(msg_, 256, msg);
    }
}