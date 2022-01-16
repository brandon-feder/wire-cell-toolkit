#+TITLE: Wire Cell Toolkit Configuration

* What is here

This repository holds a reference set of Wire Cell Toolkit
configuration files, largely in the form of Jsonnet.

The files at top level are general utilities are effort will be kept
to maintain their stability. 

Sub-directories can be expected to evolve (and disappear). Effort will
be made to keep what is there current with the contemporaneous C++
code.

For more information on configuration see the [[https://github.com/WireCell/wire-cell-docs/tree/master/manuals/configuration.org][WCT configuration manual]]
as well as [[https://wirecell.github.io/news/categories/config/][news posts tagged with config]].

* Contents overview

This describes what is available.  Be wary is may not always be up to
date with the reality on the ground.

** General support files

Files at top level provide general support and utilities.  Primarily
there is the file:

- [[./wirecell.jsonnet]] 

This holds a Jsonnet version of the WCT /system of units/ (which is
essentially identical to CLHEP's).  It also includes a number of
Jsonnet functions to form some common configuration data structures.

- [[./vector.jsonnet]] 

This holds some functions to assist in doing vector arithmetic in Jsonnet.

- [[./pgraph.jsonnet]]

This holds functions to support building a processing graph for use by
the ~Pgrapher~ WCT app component.  Some details are [[https://wirecell.github.io/news/posts/pgrapher-configuration-improvements/][here]].

** Pgrapher Modules

Although there are several WCT app components, currently the most
powerful and flexible is Pgrapher.  It's configuration is based on
building a graph of nodes and with the help of ~pgraph.jsonnet~ it is
possible to factor many of the possible graphs into common a set of
subgraphs.  This allows a full configuration to be constructed at
various levels of detail culminating on a simple high-level
aggregation of nodes which is typically in the form of a simple linear
pipeline.  

A factored collection of Pgrapher configuration is available under
[[./pgrapher/]].  See [[./pgrapher/README.org][it's README]] for more info.

** Obsolete

Configuration can become obsolete when the C++ changes or when new
paradigms of configuration organization are adopted.  That
configuration may be moved to [[./obsolete/]] for some time before being
dropped from the tip of the master branch.

* Tests

There may be other Jsonnet files found in the ~/test/~ sub-directories
of the various WCT packages.

Running the configuration tests is done through the files under =test/=.  They rely on a simple ad-hoc test harness. All tests can be run from the top-level =wire-cell= source directory after a build like:

#+BEGIN_EXAMPLE
  ./cfg/test/test_all.sh
#+END_EXAMPLE

A single test can be run like:

#+BEGIN_EXAMPLE
  ./cfg/test/test_one.sh <testname>
#+END_EXAMPLE

Each test has a =test_<testname>.jsonnet= file.

#+BEGIN_EXAMPLE
  ls cfg/test/test_*.jsonnet
#+END_EXAMPLE

These main JSonnet files are typically composed of some chunks reused by the various different tests. The chunks are named like =cfg_*.jsonnet=.  The body of each main =test_<testname>.jsonnet= largely consists of the data flow graph definition for the =TbbFlow= Wire Cell application object.

See also https://github.com/wirecell/wire-cell-tests.
* JQ tricks

The ~jq~ tool is like ~grep~ for JSON.  Here are some useful tricks to operate on a mongo big JSON file.

** WCT JSON Config Files

Find an element in the /configuration sequence/ by type

#+BEGIN_EXAMPLE
  $ jq '.[]| select(.type|contains("Drifter"))' wct.json 
#+END_EXAMPLE
