from ctypes import *
from ctypes.util import find_library

class FILE(Structure):
    pass

# oddly, size_t is in ctypes, but not ptrdiff_t
if sizeof(c_void_p) == 4:
    ptrdiff_t = c_int32
elif sizeof(c_void_p) == 8:
    ptrdiff_t = c_int64

#libc = CDLL(find_library("c"))
# HEY these are probably Mac-specific, please fix
#stdin = POINTER(FILE).in_dll(libc, "__stdinp")
#stderr = POINTER(FILE).in_dll(libc, "__stderrp")
#stdout = POINTER(FILE).in_dll(libc, "__stdoutp")


