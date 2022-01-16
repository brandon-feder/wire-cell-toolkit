#!/usr/bin/env python
'''
Common functions for *2tvtk.py
'''
import math
import numpy
from collections import defaultdict
from tvtk.api import tvtk, write_data
from tvtk.common import configure_input



def extrude(pts, dx):
    '''
    make a 3d set of cells based on ring of pts extruded along X axis by dx

    Return points and "relative cells"


    '''
    pts2 = [ [pt[0]+dx,pt[1],pt[2]] for pt in pts] # the other face
    all_pts = pts + pts2

    n = len(pts)
    top_cell = range(n)
    bot_cell = range(n, 2*n)
    cells = [top_cell, bot_cell]

    # enumerate the sides
    for ind in range(n):
        ind2 = (ind+1)%n
        cell = [top_cell[ind], top_cell[ind2], bot_cell[ind2], bot_cell[ind]]
        cells.append(cell)

    return all_pts, cells
        


def orderpoints(pointset):
    c = [0.0,0.0,0.0]
    for p in pointset:
        for i in range(3):
            c[i] += p[i]
    n = len(pointset)
    for i in range(3):
        c[i] /= n

    byang = list()
    for p in pointset:
        ang = math.atan2(p[2]-c[2], p[1]-c[1]);
        byang.append((ang, p))
    byang.sort()
    return [p for a,p in byang]

def blobs(blobs):
    '''
    Given a data structure which is a list of blobs, each blob is a dict:

    { 
      points=[[x1,y1,z1], [x2,y2,z2],...], 
      values=dict(name1=val1, name2=val2), 
      thickness=1.0, # optional
    }

    return an unstructured grid
    '''
    datasetnames = set()
    all_points = list()
    blob_cells = []
    for blob in blobs:
        myvalnames = list(blob['values'].keys())
        datasetnames = datasetnames.union(myvalnames)
        pts = orderpoints(blob['points'])
        pts,cells = extrude(pts, blob.get('thickness', 1.0))
        all_points += pts
        blob_cells.append((len(pts),cells))

    ugrid = tvtk.UnstructuredGrid(points=all_points)

    ptype = tvtk.Polyhedron().cell_type

    offset = 0
    for npts,cells in blob_cells:
        cell_ids = [len(cells)]
        for cell in cells:
            cell_ids.append(len(cell))
            cell_ids += [offset+cid for cid in cell]
        ugrid.insert_next_cell(ptype, cell_ids)
        offset += npts

    ugrid.cell_data.scalars = list(range(len(blobs)))
    ugrid.cell_data.scalars.name = "indices"

    print (datasetnames)
    narrays = 1
    for datasetname in sorted(datasetnames):
        arr = [b["values"].get(datasetname, 0.0) for b in blobs]
        ugrid.cell_data.add_array(arr)
        ugrid.cell_data.get_array(narrays).name = datasetname
        narrays += 1

    return ugrid


def points(jdat):
    '''
    Given a data strucutre which lists point data as list of objects like:
    {
      point= [x,y,z],
      values=dict(name1=val1, name2=val2, ...)
    }
    '''
    datasetnames = set()
    for one in jdat:
        for n in one['values'].keys():
            datasetnames.add(n)

    values = defaultdict(list)
    points = list()
    indices = list()

    for count,one in enumerate(jdat):
        for name in datasetnames:
            values[name].append(one['values'].get(name, 0.0))
        pt = one['point']
        #print (count,pt)
        points.append(pt)
        indices.append(count)
    points = numpy.array(points, 'f')
    npoints = len(points)
    #print (points.shape)
    ret = tvtk.PolyData(points=points)
    verts = numpy.arange(0, npoints, 1)
    verts.shape = (npoints,1)
    ret.verts = verts
    ret.point_data.scalars = indices[:npoints]
    ret.point_data.scalars.name = 'indices'
    for count, datasetname in enumerate(sorted(datasetnames)):
        count += 1
        arr = [p["values"].get(datasetname, 0.0) for p in jdat]
        ret.point_data.add_array(numpy.array(arr, 'f'))
        ret.point_data.get_array(count).name = datasetname

    return ret
