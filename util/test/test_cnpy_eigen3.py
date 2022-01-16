#!/usr/bin/python

import numpy

dtype = 'int32'

arr = numpy.array(numpy.arange(12), dtype=dtype);
arr.resize(3,4)

c_order = numpy.array(arr, order='C', dtype=dtype)
f_order = numpy.array(arr, order='F', dtype=dtype)

print(arr)
print(f'native : {arr.shape} {arr.dtype}\n{arr.flags}')
print(f'nativeT: {arr.T.shape} {arr.T.dtype}\n{arr.T.flags}')
print(f'c_order: {c_order.shape} {c_order.dtype}\n{c_order.flags}')
print(f'f_order: {f_order.shape} {f_order.dtype}\n{f_order.flags}')

print('saving test_cnpy_eigen3.npz')
numpy.savez('test_cnpy_eigen3.npz',
            c_order = c_order,
            f_order = f_order,
            n_order = arr)
