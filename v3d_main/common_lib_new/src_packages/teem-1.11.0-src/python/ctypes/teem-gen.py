#! /usr/bin/env python

##
##  teem-gen.py: automatically-generated ctypes python wrappers for Teem
##  Copyright (C) 2012, 2011 University of Chicago
##  created by Sam Quinan - samquinan@cs.uchicago.edu
##
##  Permission is hereby granted, free of charge, to any person obtaining
##  a copy of this software and associated documentation files (the
##  "Software"), to deal in the Software without restriction, including
##  without limitation the rights to use, copy, modify, merge, publish,
##  distribute, sublicense, and/or sell copies of the Software, and to
##  permit persons to whom the Software is furnished to do so, subject to
##  the following conditions:
##
##  The above copyright notice and this permission notice shall be
##  included in all copies or substantial portions of the Software.
##
##  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
##  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
##  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
##  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
##  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
##  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
##  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
##

import os, sys, shutil, platform, re, string

if len(sys.argv) != 3:
    sys.exit("program expexts arguments: 'ctypeslib-gccxml source dir' 'teem install dir' ")

## (TEEM_LIB_LIST)
libs_list = ["air", "hest", "biff", "nrrd", "ell", "unrrdu", "alan", "moss", "tijk", "gage", "dye", "bane", "limn", "echo", "hoover", "seek", "ten", "elf", "pull", "coil", "push", "mite", "meet"]

#
# validate os 
#
if os.name != "posix":
    sys.exit("program only supports posix compilant systems at this time") 

#
# validate gccxml install
#
tmp = os.path.join(os.getcwd(), "tmp")
os.system("gccxml --version &> tmp")
file = open(tmp, "r")
file.seek(0)
first_line = file.readline()
file.close()
if not first_line.startswith("GCC-XML version"):
    os.remove(tmp)
    sys.exit("error: gccxml of version >= 0.9.0 is required for this script")

if not ((int(first_line[16]) > 0) or ((int(first_line[18]) == 9) and (int(first_line[20]) >= 0))):
    os.remove(tmp)
    sys.exit("error: gccxml of version >= 0.9.0 is required for this script")
os.remove(tmp)

#
# validate ctypeslib-gccxml source dir path 
#
if not os.path.isdir(sys.argv[1]):
    sys.exit("%s does not point to a directory" % sys.argv[1])

CTYPES = os.path.abspath(sys.argv[1])

#
# validate teem install dir path
#
if not os.path.isdir(sys.argv[2]):
    sys.exit("%s does not point to a directory" % sys.argv[2])

TEEM = os.path.abspath(sys.argv[2])


#
# copy files from install dir 
#

TMP_DIR = "teem-gen-tmp"

teem_include = os.path.join(TEEM, "include") 

if not os.path.isdir(teem_include): 
    sys.exit("%s is not the teem install directory; %s is not a valid path" % (teem, teem-install))

if os.path.isdir(TMP_DIR):
    shutil.rmtree(TMP_DIR)

shutil.copytree(teem_include, TMP_DIR)


#
# generate all.h
#

all_h = os.path.join(TMP_DIR, "all.h")
f_open = open (all_h, "w")

defines = []

Files = os.listdir(os.path.join(TMP_DIR, "teem"))
for file in Files:
    if file.endswith(".h"):
        f_open.write("#include <teem/%s> %s" % (file, os.linesep))
        #
        # get #define statements from file
        #
        lines = open (os.path.join(TMP_DIR, "teem", file), "r").readlines()
        expr1 = re.compile("^#define")
        expr2 = re.compile("\\\\")
        expr3 = re.compile("HAS_BEEN_INCLUDED")
        expr4 = re.compile("##")
        expr5 = re.compile("^#define [^ ]*\(")
        expr6 = re.compile("NRRD_TYPE_BIGGEST")
        # this strips out NRRD_LLONG_MAX, NRRD_LLONG_MIN, and NRRD_ULLONG_MAX,
        # which depend on macros AIR_LLONG and AIR_LLONG.  For some reason
        # this cause a problem now (Thu Jun 14 11:49:14 CDT 2012) even though
        # they haven't before, though these constants and macros are not new
        expr7 = re.compile("LLONG")
        for line in lines:
            if (expr1.search(line) and not expr2.search(line) 
                and not expr3.search(line) and not expr4.search(line)
                and not expr5.search(line) and not expr6.search(line)
                and not expr7.search(line)):
                firstword, restwords = string.replace(string.replace(line[8:], "/*", "#" ), "*/", "").split(None,1)
                defines.append("%s = %s" % (firstword, restwords))
f_open.close()

#
# generate teem.xml 
#
# note: these calls are unix only because windows support for command line 
#	append to python path (where the scopeis only that one call) is too
#	difficult at this time -- better support may be added to later versions
#	of python, so that something to watch for / modify in the future

pypath_append = "PYTHONPATH=%s:%s" % (CTYPES, os.getcwd())
teem_xml = os.path.join(os.getcwd(), "teem.xml")
os.system("%s python %s %s -I %s -o %s" % (pypath_append, os.path.join(CTYPES, "scripts", "h2xml.py"), os.path.abspath(all_h), TMP_DIR, teem_xml))


#
# generate pre-teem.py 
#
pre_teem_py = os.path.join(os.getcwd(), "pre-teem.py")

teem_libs = '|'.join(libs_list)

system_type = platform.system()
dll_path = ""
ext = ""
substr = ""
if system_type == "Darwin":
    dll_path = "DYLD_LIBRARY_PATH=%s" % os.path.join(TEEM, "lib")
    ext = "dylib"
    substr = "_libraries['%s']" % os.path.join(TEEM, "lib", "libteem.dylib")
else:
    dll_path = "LD_LIBRARY_PATH=%s" % os.path.join(TEEM, "lib")
    ext = "so"
    substr = "_libraries['libteem.so']"

os.system("%s %s python %s %s -l libteem.%s -o %s -m stdio -r \"(%s).*\"" % (dll_path, pypath_append, os.path.join(CTYPES, "scripts", "xml2py.py"), teem_xml, ext,  pre_teem_py, teem_libs))

#
# generate teem.py 
#

libs_destuctable = list(libs_list)

contents = open(pre_teem_py, "r").readlines()[8:]
mod_contents = []
for line in contents:
    l = line.replace(substr, "libteem")
    if not (("__darwin_size_t = c_ulong" in l) or ("size_t = __darwin_size_t" in l)):
        l2 = l.replace("\'__darwin_size_t\',", "") #designed to remove defs from long list at end
        l3 = l2.replace("\'size_t\',", "") #designed to remove defs from long list at end
        l = l3
        if not re.compile("_size_t").search(l3):
            if not re.match("#", l3): # i.e. do not make changes in commented lines
                l = l3.replace("size_t", "c_size_t")
        mod_contents.append(l)
        if "Present" in l:
            for lib in libs_destuctable:
                lib_str = lib+"Present"
                if lib_str in l:
                    libs_destuctable.remove(lib)
                    break

# in experimental libs not included, cleanup and fail
if libs_destuctable: # empty sequence implicity false
    shutil.rmtree(TMP_DIR)
    os.remove(teem_xml)
    #os.remove(pre_teem_py)
    sys.exit("ERROR: experimental libs: %s not turned on - please rebuild teem with BUILD_EXPERIMENTAL_LIBS turned on, then re-run teem-gen.py" % ','.join(libs_destuctable))

header = [
"##",
"##  teem.py: automatically-generated ctypes python wrappers for Teem",
"##  Copyright (C) 2012, 2011, 2010, 2009  University of Chicago",
"##",
"##  Permission is hereby granted, free of charge, to any person obtaining",
"##  a copy of this software and associated documentation files (the",
"##  \"Software\"), to deal in the Software without restriction, including",
"##  without limitation the rights to use, copy, modify, merge, publish,",
"##  distribute, sublicense, and/or sell copies of the Software, and to",
"##  permit persons to whom the Software is furnished to do so, subject to",
"##  the following conditions:",
"##",
"##  The above copyright notice and this permission notice shall be",
"##  included in all copies or substantial portions of the Software.",
"##",
"##  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,",
"##  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF",
"##  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND",
"##  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE",
"##  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION",
"##  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION",
"##  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.",
"##",
"##############################################################",
"##############################################################",
"#### NOTE: This teem.py file is automatically produced by",
"#### teem/python/ctypes/teem-gen.py.  Necessary changes to ",
"#### teem.py should be made in teem-gen.py, not here.",
"##############################################################",
"##############################################################",
"",
"from ctypes import *",
"import ctypes.util",
"import sys, os",
"",
"def load_library(libname, loader_path=\"\"):",
"    ext = os.path.splitext(libname)[1]",
"    if not ext:",
"        # Try to load library with platform-specific name",
"        if sys.platform == 'win32':",
"            libname_ext = '%s.dll' % libname",
"        elif sys.platform == 'darwin':",
"            libname_ext = '%s.dylib' % libname",
"        elif sys.platform == 'linux2':",
"            libname_ext = '%s.so' % libname",
"    else:"
"        libname_ext = libname",
"",
"    if (loader_path != \"\"):",
"        loader_path = os.path.abspath(loader_path)",
"        if not os.path.isdir(loader_path):",
"            libdir = os.path.dirname(loader_path)",
"        else:",
"            libdir = loader_path",
"    else:",
"        libdir = loader_path",
"",          
"    try:",
"        libpath = os.path.join(libdir, libname_ext)",
"        return CDLL(libpath)",
"    except OSError, e:",
"        raise e"
"",          
"try:",
"    libteem = load_library('libteem')",
"except OSError:",
"    print \"**\"",
"    print \"**  teem.py couldn't find and load the \\\"libteem\\\" shared library.\"",
"    print \"**\"",
"    print \"**  try setting optional loader_path argument in the load_library() call above to '<teem-install-dir>/lib/'\"",
"    print \"**\"",
"    raise ImportError",
"",          
"# =============================================================",
"# Utility types and classes to help teem.py be platform-independent.",
"",
"STRING = c_char_p",
"",
"class FILE(Structure):",
"    pass",
"",
"# oddly, size_t is in ctypes, but not ptrdiff_t",
"# which is probably a bug",
"if sizeof(c_void_p) == 4:",
"    ptrdiff_t = c_int32",
"elif sizeof(c_void_p) == 8:",
"    ptrdiff_t = c_int64",
"",
"# =============================================================",
"# What follows are all the functions, struct definitions, globals, ",
"# enum values, and typedefs in Teem. This is generated by ctypeslib:",
"#   http://svn.python.org/projects/ctypes/branches/ctypeslib-gccxml-0.9",
"# followed by further post-processing and filtering.",
"# See end of this file for definitions of stderr, stdin, stdout",
]

footer = [
"# =============================================================",
"# Make sure this shared library will work on this machine.",
"if not nrrdSanity():",
"    errstr = biffGetDone(NRRD)",
"    print \"**\"",
"    print \"** Sorry, there is a problem (described below) with the \"",
"    print \"** Teem shared library that prevents its use. This will \"",
"    print \"** have to be fixed by recompiling the Teem library for \"",
"    print \"** this platform. \"",
"    print \"**\"",
"    print \"** %s\" % errstr",
"    raise ImportError",
"",
"# =============================================================",
"# Its nice to have these FILE*s around for utility use, but they ",
"# aren't available in a platform-independent way in ctypes. These ",
"# air functions were created for this purpose.",
"stderr = airStderr()",
"stdout = airStdout()",
"stdin = airStdin()",
]

teem_py = os.path.join(os.getcwd(), "teem.py")

if os.path.exists(teem_py):
    os.remove(teem_py)

out = open(teem_py, "w")

for line in header:
    out.write(line)
    out.write(os.linesep)

out.writelines(mod_contents)
out.write(os.linesep)

out.write("# =============================================================\n")
out.write("# What follows are the all #define's in Teem, excluding macros,\n")
out.write("# and #defines that depend on compile-time tests done by the\n")
out.write("# C pre-processor.\n")
out.write("# This is created by something akin to grep'ing through the\n")
out.write("# public header files, with some extra filters.\n")
out.write(os.linesep)
out.writelines(defines)
out.write(os.linesep)

for line in footer:
    out.write(line)
    out.write(os.linesep)

out.close()


#
# cleanup
#

shutil.rmtree(TMP_DIR)
os.remove(teem_xml)
os.remove(pre_teem_py)
