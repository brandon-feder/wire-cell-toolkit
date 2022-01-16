Wire Cell Gen
==============

This is the Wire Cell Gen package.  It provides reference
implementation of Wire Cell interfaces in ways that can be generated
with no outside dependencies.

Functionality includes:

- wire and cell geometry,
- drifting charge deposition in the volume of the detector,
- diffusion of these charges as they drift,
- digitization of charge on the wire planes.

These layers can be used with other Wire Cell implementations.  For
example:

 - depositions from detailed simulation can be provided and then Gen's
drifting, diffusing and digitization can be used.

 - your own wire geometry can be used with Gen's cell construction or
Gen's wire generated wire geometry can be used as input to your own
cell construction.

