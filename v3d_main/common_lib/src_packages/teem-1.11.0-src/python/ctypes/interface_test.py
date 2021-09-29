#!/usr/bin/env python

# test.py: 16-bit grayscale PNG to nrrd file
# Sam Quinan

import Nrrd as nrd, numpy as np 

# TESTING: fmob-ch4 crop test
print "TEST: fmob-ch4.nrrd"
a = nrd.Nrrd()
a.load("../../data/fmob-c4h.nrrd")
print "nrrd loaded..."
b = nrd.ExtendedArray(a)
print "wrapped as array..."
c = b[5:11,:,:]
print "cropped..."
#print c.shape
d = np.ascontiguousarray(c, dtype=c.dtype.type)
print "made contiguous..."
#print d.shape
e = nrd.Nrrd()
e.fromNDArray(d)
print "loaded..."
e.save("fmob_test_out.nrrd")
print "done"



# OLD TESTING -- IGNORE

#in_image = Image.open("msix.png")
#f_data = numpy.asarray(in_image)
#f_nrrd = Nrrd()
#f_nrrd.fromNDArray(f_data)
#del f_data
#f_nrrd.save("test.nrrd")
#t = f_nrrd
#a = numpy.asarray(f_nrrd)
#print a
#del f_nrrd

#a = Nrrd()
#a.load("msix.png")
#
#b = ExtendedArray(a)
#print b
#print type(b)
#print b.flags
#print b.data
#print b.base_ref
#print b.base is None
#
#print "-------------------------------"
#
#in_image = Image.open("msix.png")
#c = numpy.asarray(in_image)
#del in_image
#
#print c
#print type(c)
#print c.flags
#print c.data
#
#d = Nrrd()
#d.fromNDArray(c)
#
#e = ExtendedArray(d)
#e2 = e[:,[0,2]]
#
#print e 
#print type(e)
#print e.flags
#print e.data
#print e.base_ref
#
#
#print "-------------------------------"
#
#arr = numpy.zeros((4,))
#print arr.base is None
#print arr
#v1 = arr.view(ExtendedArray)
#print v1.base is arr
#print v1
#v2 = arr[2:]
#print v2.base is arr
#print v2

#c_data = numpy.asarray(in_image)
#print "C CONTIGUOUS"
#print c_data
#print c_data.flags
#print c_data.strides
#f_data = numpy.require(in_image, dtype=None, requirements='F_CONTIGUOUS')
#print "F CONTIGUOUS"
#print f_data
#print f_data.flags
#print f_data.strides
#c_nrrd = ndarrayToNrrd(c_data)
#f_nrrd = ndarrayToNrrd(f_data)
#teem.nrrdSave("foo.nrrd", c_nrrd, None)
#teem.nrrdSave("bar.nrrd", f_nrrd, None)