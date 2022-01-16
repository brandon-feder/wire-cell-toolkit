// Helpers to make configuration objects for file input/output
// components.

local pg = import "pgraph.jsonnet";

{
    // If a frame is written out "dense" it the sink needs to know
    // what the bounds are in channel IDs (rows) and ticks (columns).
    frame_bounds(nchannels, nticks, first_chid=0, first_tick=0) :: {
        chbeg: first_chid, chend: first_chid+nchannels,
        tbbeg: first_tick, tbend: first_tick+nticks
    },

    /// Sink a stream of frames to file. 
    frame_file_sink(filename, tags=[],
                    digitize=false, dense=null) :: 
        pg.pnode({
            type: "FrameFileSink",
            name: filename,
            data: {
                outname: filename,
                tags: tags,
                digitize: digitize,
                dense: dense,
            },
        }, nin=1, nout=0),
        


    // Like a frame_file_sink but pass input frames both to output
    // port 0 and to a sink.
    frame_file_tap(filename, tags=[], digitize=false, dense=null) ::
        pg.fan.tap('FrameFanout', 
                   $.frame_file_sink(filename, 
                                     tags=tags,
                                     digitize=digitize,
                                     dense=dense), filename),


}
