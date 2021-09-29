#!/usr/bin/env python
##
##  Nrrd.py: bridge between Nrrd and Numpy arrays
##  Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
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

import numpy, teem, ctypes

# translates from numpy type to teem type
def ndarrayGetTypes( array ):
    typeTable = {
        numpy.int8      :   (ctypes.c_byte, teem.nrrdTypeChar),
        numpy.uint8     :   (ctypes.c_ubyte, teem.nrrdTypeUChar),
        numpy.int16     :   (ctypes.c_short, teem.nrrdTypeShort),
        numpy.uint16    :   (ctypes.c_ushort, teem.nrrdTypeUShort),
        numpy.int       :   (ctypes.c_int, teem.nrrdTypeInt),
        numpy.uint      :   (ctypes.c_uint, teem.nrrdTypeUInt),
        numpy.int32     :   (ctypes.c_int32, teem.nrrdTypeInt),
        numpy.uint32    :   (ctypes.c_uint32, teem.nrrdTypeUInt),
        numpy.int64     :   (ctypes.c_int64, teem.nrrdTypeLLong),
        numpy.uint64    :   (ctypes.c_uint64, teem.nrrdTypeULLong),
        numpy.float     :   (ctypes.c_float, teem.nrrdTypeFloat),
        numpy.double    :   (ctypes.c_double, teem.nrrdTypeDouble)     }
    
    dt = array.dtype.type

    if dt not in typeTable:
        raise Exception("array type %s not supported" % dt)
    # sanity checking
    ct, tt = typeTable[dt]
    x = numpy.dtype(dt).itemsize
    y = ctypes.sizeof(ct)
    z = teem.nrrdTypeSize[tt]
    if not (x == y) or not (x == z) or not (y == z):
        raise Exception("corresponding numpy, teem, and ctypes types are not of the same size")
    return typeTable[dt]

def teemTypeToNumpyType(teem_type):
    numpy_endianess_glyph = '<' if teem.airMyEndian() == 1234 else '>'
    numpy_type_map = {
        teem.nrrdTypeChar: numpy_endianess_glyph + 'i1',
        teem.nrrdTypeUChar: numpy_endianess_glyph + 'u1',
        teem.nrrdTypeShort: numpy_endianess_glyph + 'i2',
        teem.nrrdTypeUShort: numpy_endianess_glyph + 'u2',
        teem.nrrdTypeInt: numpy_endianess_glyph + 'i4',
        teem.nrrdTypeUInt: numpy_endianess_glyph + 'u4',
        teem.nrrdTypeLLong: numpy_endianess_glyph + 'i8',
        teem.nrrdTypeULLong: numpy_endianess_glyph + 'u8',
        teem.nrrdTypeFloat: numpy_endianess_glyph + 'f4',
        teem.nrrdTypeDouble: numpy_endianess_glyph + 'f8'
    }
    return numpy_type_map[teem_type]

                        
# design based on a combination of work done by Carlos Scheidegger 
#   [ http://code.google.com/p/python-teem/source/browse/trunk/teem/capi/numpy/__init__.py ,
#     http://code.google.com/p/python-teem/source/browse/trunk/teem/nrrd.py ]
#   and ideas presented by Travis Oliphant [ http://blog.enthought.com/?p=62 ]
                    
class Nrrd:
    def __init__(self):
        self._ctypesobj = teem.nrrdNew()
        # self._init = False
        self.base_ref = None
        self.teem = teem # world's ugliest hack -- but it appears to work
    
    def __del__(self):
        if self.base_ref == None:
            self.teem.nrrdNuke(self._ctypesobj)
        else:
            self.teem.nrrdNix(self._ctypesobj)
        self.base_ref = None
        self.teem = None
                        
    def __get_array_interface(self):
        nrrd = self._ctypesobj.contents
        s = []
        for i in reversed(range(nrrd.dim)):
            s.append(nrrd.axis[i].size)
        r = {'shape': tuple(s),
            'typestr': teemTypeToNumpyType(nrrd.type),
            'data': (nrrd.data, False),
            'version': 3}
        return r
    
    __array_interface__ = property(__get_array_interface)
                        
    def fromNDArray(self, array):
        
        if self.base_ref != None:
            self.base_ref = None
            teem.nrrdNix(self._ctypesobj)
            self._ctypesobj = teem.nrrdNew()
        
        try:
            cTy, teemTy = ndarrayGetTypes(array)
        except Exception as err:
            print err
            return
        
        c_type_p = ctypes.POINTER( cTy )
        if (array.flags.f_contiguous): # memory shared with array
            array_p = array.ctypes.data_as(c_type_p)
            dims = list(array.shape)
            sizes = (ctypes.c_size_t * teem.NRRD_DIM_MAX)(*dims)
            teem.nrrdWrap_nva(self._ctypesobj, array_p, teemTy, ctypes.c_uint(array.ndim), sizes)
            self.base_ref = array
        elif (array.flags.c_contiguous): # memory shared with array
            array_p = array.ctypes.data_as(c_type_p)
            dims = list(reversed(array.shape))
            sizes = (ctypes.c_size_t * teem.NRRD_DIM_MAX)(*dims)
            teem.nrrdWrap_nva(self._ctypesobj, array_p, teemTy, ctypes.c_uint(array.ndim), sizes)
            self.base_ref = array
        else: # must make c_contiguous -- memory not shared with array, transfer full control to teem
            arr = numpy.ascontiguousarray(array, dtype=array.dtype.type)
            array_p = arr.ctypes.data_as(c_type_p)
            dims = list(reversed(arr.shape))
            nrrd_tmp = teem.nrrdNew()
            sizes = (ctypes.c_size_t * teem.NRRD_DIM_MAX)(*dims)
            teem.nrrdWrap_nva(nrrd_tmp, array_p, teemTy, ctypes.c_uint(array.ndim), sizes)
            teem.nrrdCopy(self._ctypesobj, nrrd_tmp) # this call hangs with fmob-ch4.nrrd
            teem.nrrdNix(nrrd_tmp)
            self.base_ref = None
        self._init = True

        if isinstance(array, ExtendedArray):
            if not array.base_ref is None: # means there is some Nrrd file sharing it's memory with the array
                teem.nrrdAxisInfoCopy(self._ctypesobj, array.base_ref._ctypesobj, None, teem.NRRD_AXIS_INFO_SIZE_BIT)

    def load(self, file, args=None):
        
        if self.base_ref != None:
            self.base_ref = None
            teem.nrrdNix(self._ctypesobj)
            self._ctypesobj = teem.nrrdNew()
        
        teem.nrrdLoad(self._ctypesobj, file, args)
        self.base_ref = None
        self._init = True

    def save(self, file, args=None):
        teem.nrrdSave(file, self._ctypesobj, args)




class ExtendedArray(numpy.ndarray):
    def __new__(cls, input_array, base_ref=None):
        if isinstance(input_array, Nrrd):
            if input_array.base_ref == None:
                obj = numpy.asarray(input_array).view(cls)
                obj.base_ref = input_array
            else:
                obj = numpy.asarray(input_array.base_ref).view(cls)
                obj.base_ref = None
        else:
            obj = numpy.asarray(input_array).view(cls)
            obj.base_ref = base_ref
        return obj
    
    def __array_finalize__(self, obj):
        if obj is None: return
        self.base_ref = getattr(obj, 'base_ref', None)

