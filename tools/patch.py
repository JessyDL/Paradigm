import re
import functools
import os
import sys

def patch_file(filename):    
    def decorator(fn):
        @functools.wraps(fn)
        def wrapper(self):
            
            if self.filename.endswith(filename):
                self.read()
                fn(self)
        
        wrapper._targetable = True
        return wrapper
    return decorator

class File(object):
    def __init__(self, filename):
        filename = filename.replace('//', '/')
        self.filename = filename
        self.dirty = False
        self.content = ""
        try:
            if ".git" in self.filename:
                self.obj = None
            else:
                self.obj = open(self.filename, 'r+')
        except Exception as e: 
            self.obj = None
            print(e)
            pass

    def patch(self):
        for key, value in self.__class__.__dict__.items():
            if(getattr(value, '_targetable', False)):
                getattr(self, key)()

    def __del__(self):
        if self.dirty and self.obj:
            print("patched ", self.filename) 
            self.obj.write(self.content)
            self.obj.truncate()
            self.obj.close()

    def read(self):
        if not self.dirty and self.obj:
            self.content = self.obj.read()
            self.obj.seek(0)
        self.dirty = True

    @patch_file("include/vulkan/vulkan.h")
    def patch_vulkan(self):
        self.content = re.sub('^#include <windows.h>$', \
          r'''typedef unsigned long DWORD;\n'''
          r'''typedef const wchar_t* LPCWSTR;\n'''
          r'''typedef void* HANDLE;\n'''
          r'''typedef struct HINSTANCE__* HINSTANCE;\n'''
          r'''typedef struct HWND__* HWND;\n'''
          r'''typedef struct HMONITOR__* HMONITOR;\n'''
          r'''typedef struct _SECURITY_ATTRIBUTES SECURITY_ATTRIBUTES;''', \
          self.content, flags = re.M)
          
    @patch_file("src/spdlog.cpp")
    def patch_spdlogcpp(self):
        if not re.search(r'#include <Windows.h>', self.content):
            self.content = "#ifdef _WIN32\n#include <Windows.h>\n#endif\n" + self.content

    @patch_file("include/spdlog/sinks/wincolor_sink.h")
    def patch_wincolorh(self):
        if re.search("#include <wincon.h>", self.content):
            self.content = re.sub("#include <wincon.h>",  \
              r'''typedef unsigned short WORD;\n'''
              r'''typedef void* HANDLE;\n'''
              r'''#ifndef FOREGROUND_BLUE\n'''
              r'''#define FOREGROUND_BLUE      0x0001\n'''
              r'''#define FOREGROUND_GREEN     0x0002\n'''
              r'''#define FOREGROUND_RED       0x0004\n'''
              r'''#define FOREGROUND_INTENSITY 0x0008\n'''
              r'''#define BACKGROUND_BLUE      0x0010\n'''
              r'''#define BACKGROUND_GREEN     0x0020\n'''
              r'''#define BACKGROUND_RED       0x0040\n'''
              r'''#define BACKGROUND_INTENSITY 0x0080\n'''
              r'''#define WIN_OVERRIDE\n'''
              r'''#endif\n''', self.content)
            self.content += "\n#ifdef WIN_OVERRIDE\n" \
              "#undef FOREGROUND_BLUE\n" \
              "#undef FOREGROUND_GREEN\n" \
              "#undef FOREGROUND_RED\n" \
              "#undef FOREGROUND_INTENSITY\n" \
              "#undef BACKGROUND_BLUE\n" \
              "#undef BACKGROUND_GREEN\n" \
              "#undef BACKGROUND_RED\n" \
              "#undef BACKGROUND_INTENSITY\n" \
              "#undef WIN_OVERRIDE\n" \
              "#endif\n"

    @patch_file("core/core.vcxproj")
    def patch_msvc(self):
        self.content = re.sub(r'(<ObjectFileName.*?>)(.*?)(</ObjectFileName>)', r'\1$(IntDir)%(Directory)\3', self.content)

def patch(root):
    root = root.replace('\\', '/')
    files = []

    for r, d, f in os.walk(root):
        for file in f:
            files.append(os.path.join(r, file))

    for f in files:
        fObj = File(f.replace('\\', '/'))
        fObj.patch()
    
if __name__ == "__main__":
    if len(sys.argv) > 1:
        patch(sys.argv[1])