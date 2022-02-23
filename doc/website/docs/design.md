
# Design principles

Graphite and Geogram are designed with the following principles in mind (some of them
borrowed from the "Futurist Programmer" manifesto and programming notes, 
see [1](http://www.graficaobscura.com/future/index.html) and 
[2](http://www.graficaobscura.com/future/futnotes.html)):

- Make it as simple as possible (but not simpler)
- Make it as easy to use as possible
- Make it as easy to compile as possible
- Maximize speed
- Minimize memory consumption
- Minimize number of lines of code
- Minimize number of C++ classes
- Systematically document all classes, all functions and all parameters
- Systematically document the implementation of all algorithms, with
 relevant bibliographic references when applicable
- Assertion checks everywhere
- Zero warnings with all compilers / platforms / maximum level of
 warnings activated
- Perform systematic non-regression testing and memory checking, using
 a continuous integration platform (Jenkins)

The algorithms implemented in Geogram are specialized and tuned for most common
uses. Users interested in  generic / extensible / configurable 
implementations of these algorithms may use [CGAL](http://www.cgal.org) instead.

