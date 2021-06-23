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
        
print("python argv :  SolutionDir  ProjectDir  UM_Build_Dir  ProjectName  TargetName  Configuration\n" + str(sys.argv[1:]) + "\n")  
time.sleep(1) #等待外部程序对文件的所有权释放, 否则可能会出现拷贝失败, 没有权限的问题
#$(SolutionDir) $(ProjectDir) $(dst_Build_Dir) $(ProjectName) $(TargetName) $(Configuration)

solution_dir = sys.argv[1]   
project_dir = sys.argv[2] 
solution_build_dir = sys.argv[3] #debug, release 各种配置的生成主目录
project_name = sys.argv[4]   
target_name = sys.argv[5]  #目标名称可能为 projectD
config = sys.argv[6]

#'E:\\Projcet_CCF\\', 'E:\\Projcet_CCF\\CBase\\CLan\\', 'W:\\Build\\', 'CLan', 'CLan', 'Release'
#solution_dir = 'E:\\Projcet_CCF\\' 
#project_dir = 'E:\\Projcet_CCF\\CBase\\CLan\\'
#solution_build_dir = 'W:\\Build\\'  
#project_name = 'CLan'
#target_name = 'CLan'
#config = 'Release'

    
#不拷贝测试项目
if project_name.find('test') != -1:
    exit(0) 
 
platform = "x64"
third_bin_dir = None

if config == "Debug" :
    third_bin_dir = os.path.join(solution_dir, "build_third\\") + "binD"
else:
    third_bin_dir = os.path.join(solution_dir, "build_third\\") + "bin"
    
suffix = config + "_" + platform + "\\"
 
export_dir = os.path.join(solution_build_dir, "export_" + suffix)   
ex_inc_dir = export_dir + "inc\\" + project_name + "\\"
ex_lib_dir = export_dir + "lib\\" 
ex_bin_dir = export_dir + "bin\\" 
ex_pdb_dir = export_dir + "pdb\\"   

api_mk_dir(ex_inc_dir) 
api_mk_dir(ex_lib_dir) 
api_mk_dir(ex_bin_dir) 
api_mk_dir(ex_pdb_dir)  

build_dir = os.path.join(solution_build_dir, suffix)  
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
src = build_dir + target_name + ".lib" 
if os.path.exists(src):
    print("copy lib " + target_name)
    dst = ex_lib_dir + target_name + ".lib"  
    copy_file(src, dst)   
   
###########################################################
#拷贝pdb文件  
########################################################### 
src = build_dir + target_name + ".pdb"
if os.path.exists(src):
    print("copy pdb " + target_name)
    dst = ex_pdb_dir + target_name + ".pdb" 
    copy_file(src, dst) 
 
###########################################################   
#拷贝dll或者exe
###########################################################    
src = build_dir + target_name + ".dll" 
if os.path.exists(src):
    print("copy dll " + target_name)
    dst = ex_bin_dir + target_name + ".dll" 
    copy_file(src, dst) 
 
src = build_dir + target_name + ".exe"  
if os.path.exists(src):
    print("copy exe " + target_name)
    dst = ex_bin_dir + target_name + ".exe"  
    copy_file(src, dst)

###########################################################
#拷贝头文件  
########################################################### 
src = project_dir
if os.path.exists(src): 
    copy_header(src, ex_inc_dir)   #inc 

exit(0)