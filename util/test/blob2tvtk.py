#!/usr/bin/env python
'''
A tvtk based converter for JSON file dumped from tests

Convert a bunch of individual blobs JSON files into one:

jq -s .  test-pdsp-*.json|jq '{blobs:[.[]|.blobs|.[0]]}' > test-pdsp.json

'''
import os
import json
import numpy
from tvtk.api import tvtk, write_data
import rendertvtk



def main(outname=None, *infiles):
    if outname is None or outname == "-":
        outname = os.path.splitext(infile)[0]

    jblobs = list()
    jpoints = list()
    for infile in infiles:
        jdat = json.loads(open(infile).read())
        jblobs += jdat["blobs"]
        jpoints += jdat.get("points", [])

    blobdata = rendertvtk.blobs(jblobs)
    #w = tvtk.XMLUnstructuredGridWriter(file_name=outfile)
    #w = tvtk.UnstructuredGridWriter(file_name=outfile)
    #configure_input(w, ugrid)
    #w.write()
    ofile = outname + '-blobs.vtu'
    print (ofile)
    write_data(blobdata, ofile)

    ### see depo2tvtk.py for rendering depos from NumpyDepoSaver
    if not jpoints:
        print ("no points to convert")
        return
    pointdata = rendertvtk.points(jpoints)
    ofile = outname + "-points.vtp"
    print(ofile)
    write_data(pointdata, ofile)
    #visualize_blobs(blobdata)

def visualize_blobs(blobdata):
    '''
    Make a mayavi pipeline to produce an immediate display.
    '''
    print ("visualize")
    from mayavi import mlab
    #s = mlab.points3d(x,y,z)
    mlab.pipeline.surface(blobdata)
    mlab.show()
    

def test(filename = "blob2tvtk_test.vtk"):
    points = numpy.asarray([
        [0.0,0.0,0.0],
        [0.0,0.0,1.0],
        [0.0,1.0,1.0],
        [0.0,1.0,0.0],

        [1.0,0.0,0.0],
        [1.0,0.0,1.0],
        [1.0,1.0,1.0],
        [1.0,1.0,0.0],

    ])

    ugrid = tvtk.UnstructuredGrid(points=points)

    # #faces,  #pts, pt1, pt2, ...  #pts, pt1, pt2, ..., ...
    pt_ids = [6]
    pt_ids.append(4)
    pt_ids += list(range(4))
    pt_ids.append(4)
    pt_ids += list(range(4,8))
    for ind in range(4):
        ind2 = (ind+1)%4
        pt_ids.append(4)
        pt_ids += [ind, ind2, 4+ind2, 4+ind]
    print (pt_ids)

    ptype = tvtk.Polyhedron().cell_type
    ugrid.insert_next_cell(ptype, pt_ids)

    ugrid.cell_data.scalars = [1.0]
    ugrid.cell_data.scalars.name = "unity"
    # exercise multiple values
    ugrid.cell_data.add_array([2.0])
    ugrid.cell_data.get_array(1).name = "duality"
    ugrid.cell_data.add_array([3.0])
    ugrid.cell_data.get_array(2).name = "trinity"
    #w = tvtk.XMLUnstructuredGridWriter(file_name=filename)
    w = tvtk.UnstructuredGridWriter(file_name=filename)
    configure_input(w, ugrid)
    w.write()
    
if '__main__' == __name__:
    import sys
    main(*sys.argv[1:])

    
