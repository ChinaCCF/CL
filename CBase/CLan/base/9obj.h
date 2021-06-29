#ifndef __clan_base_obj__
#define __clan_base_obj__
 
namespace clan
{
    //定义 不能拷贝构造, 不能拷贝赋值的对象 
    class NoCopyObj
    {
    public:
        NoCopyObj() {}

        NoCopyObj(const NoCopyObj&) = delete;
        NoCopyObj& operator=(const NoCopyObj&) = delete;
     };
}

#endif//__clan_base_obj__ 