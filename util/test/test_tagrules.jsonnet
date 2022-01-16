[
    {
        frame: {
            tagXY: ["tagX%d"%n, "tagY%d"%n],
            ["is-tagged%d"%n]: "was-tagged",
        },
        // Output traces are tagged based on a map from an
        // input to one or more output tags.
        trace: {
            intagA: "outtagA%d"%n,
            intagB: ["outtagB%d"%n, "outtagBB%d"%n],
        }
    } for n in std.range(0,1)
]
