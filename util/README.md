Wire Cell Utilities
===================

Overview
--------

This package provides some utility classes and functions.  The
functionality is not specific to Wire Cell, per se.

Dependencies
------------

This package has no dependencies on any other Wire Cell package and no
outside dependencies other than C++.


Highlights
-----------

Some classes and functionality are worth highlighting here:

* **Interface** Wire Cell makes heavy use of the Interface software
  pattern.  The entire `iface` package is mostly made of interface
  classes.  Of particular note is the `Interface` interface class
  which is used to mark major component classes.  Component classes
  can be instantiated while maintaining loose coupling between
  compilation units.

* **NamedFactory** is the mechanism to allow construction of an
  interface given its base class type and the "name" of its concrete
  implementation.  Client code which instantiates an interface does
  not compile against the concrete implementation and only indirectly
  links against the shared library in which it resides.

* **Point** is a simple but useful implementation of a 3D vector.

* **Units** is a system of units.  Any and all numeric quantities in
  Wire Cell which has units is assumed to be in this system of units

* **Testing** is some functions that facilitate writing tests such as
  `Assert`.  Two more are `TimeKeeper` and `MemUsage` which are
  bundled together in `ExecMon`.  As their names imply they help code
  measure how much time and memory it uses.

