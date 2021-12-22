import sys
import os
import shutil  
import time
from win32com.shell import shell,shellcon  
#python.exe -m pip install --upgrade pip 
#python -m pip install pypiwin32 -i  https://mirrors.aliyun.com/pypi/simple/

#创建目录
def api_mk_dir(dir):
    if not os.path.exists(dir):
        os.makedirs(dir)

def api_rm_file(file):
    print("del file " + file)
    #os.remove(file)
    flags = shellcon.FOF_SILENT | shellcon.FOF_ALLOWUNDO | shellcon.FOF_NOCONFIRMATION
    shell.SHFileOperation((0, shellcon.FO_DELETE, file, None, flags, None, None)) #移动到回收站,而非直接删除
 
#移除目录
def api_rm_dir(dir):
    items = os.listdir(dir)
    for item in items:
        sub = os.path.join(dir, item)
        if os.path.isdir(item): 
            api_rm_dir(item)
        else:
            api_rm_file(item)
    print("del dir " + dir)
    os.rmdir(dir)

   
#删除目标文件夹中在源文件夹中不存在的文件
def rm_file_no_in_src(src_dir, dst_dir):
    items = os.listdir(dst_dir)
    for item in items: 
        src = os.path.join(src_dir, item)
        if os.path.exists(src):
            continue
        dst = os.path.join(dst_dir, item) 
        api_rm_file(dst)

def copy_file(src, dst):
    if not os.path.exists(dst): 
        print("copy file " + dst) 
        shutil.copy(src, dst)
        return

    src_time = os.path.getmtime(src)
    dst_time = os.path.getmtime(dst)
    if src_time > dst_time: 
        print("modify file " + dst) 
        shutil.copy(src, dst) 

def copy_header(src_dir, dst_dir): 
    if not os.path.exists(dst_dir):
        api_mk_dir(dst_dir)

    rm_file_no_in_src(src_dir, dst_dir)

    items = os.listdir(src_dir)
    for item in items:
        src = os.path.join(src_dir, item)
        if os.path.isdir(src):
            if item == "src":
                continue
            dst = os.path.join(dst_dir, item)  
            copy_header(src, dst)
        else:
            name, ext = os.path.splitext(item)
            if ext != '.h' and ext != '.hpp': 
                continue
            dst = os.path.join(dst_dir, item)  
            copy_file(src, dst)
        
print("\n\npython argv :  SolutionDir  ProjectDir  UM_Build_Dir  ProjectName  TargetName  Configuration  PlatformTarget\n" + str(sys.argv[1:]) + "\n\n")  
time.sleep(1) #等待外部程序对文件的所有权释放, 否则可能会出现拷贝失败, 没有权限的问题
#$(SolutionDir) $(ProjectDir) $(dst_Build_Dir) $(ProjectName) $(TargetName) $(Configuration)

SolutionDir = sys.argv[1]   
ProjectDir = sys.argv[2] 
UM_Build_Dir = sys.argv[3] #debug, release 各种配置的生成主目录
ProjectName = sys.argv[4]   
TargetName = sys.argv[5]  #目标名称可能为 projectD
Configuration = sys.argv[6]
PlatformTarget = sys.argv[7]

#'E:\\Projcet_CCF\\', 'E:\\Projcet_CCF\\CBase\\CLan\\', 'W:\\Build\\', 'CLan', 'CLan', 'Release'
#SolutionDir = 'E:\\Projcet_CCF\\' 
#ProjectDir = 'E:\\Projcet_CCF\\CBase\\CLan\\'
#UM_Build_Dir = 'W:\\Build\\'  
#ProjectName = 'CLan'
#TargetName = 'CLan'
#Configuration = 'Release'

    
#不拷贝测试项目
if ProjectName.find('test') != -1:
    exit(0) 

third_bin_dir = None

if Configuration == "Debug" :
    third_bin_dir = os.path.join(SolutionDir, "build_need\\") + "binD"
else:
    third_bin_dir = os.path.join(SolutionDir, "build_need\\") + "bin"
    
suffix = Configuration + "_" + PlatformTarget + "\\"
  
ex_inc_dir = UM_Build_Dir + "inc\\" + ProjectName + "\\"
ex_lib_dir = os.path.join(UM_Build_Dir, "lib\\") + suffix
ex_bin_dir = os.path.join(UM_Build_Dir, "bin\\" ) + suffix
ex_pdb_dir = os.path.join(UM_Build_Dir, "pdb\\") + suffix 

print(ex_inc_dir)
print(ex_lib_dir)
print(ex_bin_dir)
print(ex_pdb_dir)

api_mk_dir(ex_inc_dir) 
api_mk_dir(ex_lib_dir) 
api_mk_dir(ex_bin_dir) 
api_mk_dir(ex_pdb_dir)  

build_dir = os.path.join(UM_Build_Dir, suffix)  
print("Debug Dir : " + build_dir)
###########################################################
#拷贝第三方库
########################################################### 
if os.path.exists(third_bin_dir):
    items = os.listdir(third_bin_dir)
    for item in items: 
        src = os.path.join(third_bin_dir, item)

        dst = os.path.join(build_dir, item) #拷贝到当前目录
        copy_file(src, dst) 
        
        dst = os.path.join(ex_bin_dir, item) #拷贝到导出目录
        copy_file(src, dst)
    
###########################################################
#拷贝lib
########################################################### 
src = build_dir + TargetName + ".lib" 
if os.path.exists(src): 
    dst = ex_lib_dir + TargetName + ".lib"  
    copy_file(src, dst)   
   
###########################################################
#拷贝pdb文件  
########################################################### 
src = build_dir + TargetName + ".pdb"
if os.path.exists(src): 
    dst = ex_pdb_dir + TargetName + ".pdb" 
    copy_file(src, dst) 
 
###########################################################   
#拷贝dll或者exe
###########################################################    
src = build_dir + TargetName + ".dll" 
if os.path.exists(src): 
    dst = ex_bin_dir + TargetName + ".dll" 
    copy_file(src, dst) 
 
src = build_dir + TargetName + ".exe"  
if os.path.exists(src): 
    dst = ex_bin_dir + TargetName + ".exe"  
    copy_file(src, dst)

###########################################################
#拷贝头文件  
########################################################### 
src = ProjectDir
if os.path.exists(src): 
    copy_header(src, ex_inc_dir)   #inc 

exit(0)