#!/usr/bin/env python3

import io, numpy, tarfile

t = tarfile.open("test_eigen_custard_pigenc.tar")
a = io.BytesIO()
a.write(open("frompy.npy","rb").read())
print("frompy:\n",a.getbuffer().tobytes())
b = io.BytesIO()
b.write(t.extractfile("test1.npy").read())
print("test1:\n",b.getbuffer().tobytes())
print (len(a.getbuffer().tobytes()), len(b.getbuffer().tobytes()))
for i,(x,y) in enumerate(zip(a.getbuffer().tobytes(), b.getbuffer().tobytes())):
    if x == y:
        continue
    print(i,x,y) 
a.seek(0)
arr = numpy.load(a)
print(arr.shape, arr.dtype)
print(arr)
