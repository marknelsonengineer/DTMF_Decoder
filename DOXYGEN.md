Doxygen Notes
=============

[Doxygen](https://www.doxygen.nl) is the de facto standard for generating
documentation from annotated C++ sources.  
Doxygen's website is:  https://www.doxygen.nl

## Notes
   - On Linux, I prefer compile my own version of Doxygen.
   - On Windows, I install it from a Setup file.
   - I've done a lot of experimentation with Doxygen and I've developed a 
     baseline configuration that I really like.
   - I need to use [DOT](https://graphviz.org ) but 
     not [DIA](http://dia-installer.de) which is very old and is not maintained
     anymore.
   - Here's a great link on Doxygen's graphics capabilities:  Dig into it:  https://alesnosek.com/blog/2015/06/28/diagrams-and-images-in-doxygen/

## Installing from Source on Linux

### Installing Graphviz from source
   DOT is a tool from [Graphviz](https://graphviz.org)

   As a mortal user:
   ````
     wget https://gitlab.com/api/v4/projects/4207231/packages/generic/graphviz-releases/7.0.5/graphviz-7.0.5.tar.gz
     gunzip graphviz-7.0.5.tar.gz
     tar -xvf graphviz-7.0.5.tar
     cd graphviz-7.0.5
     ./configure
     make
     sudo make install
   ````
   
### Installing Clang
   As root:
   ````
     pacman -S clang
   ````
   
### Compile Doxygen from source on Linux
   As root:
   ````
     pacman -S llvm    # This is 368MB, but it's only needed to compile DOxygen.  We will remove it after.
     pacman -S xapian-core
   ````
   
   As a mortal user:
   ````
     wget https://www.doxygen.nl/files/doxygen-1.9.6.src.tar.gz
     gunzip doxygen-1.9.6.src.tar.gz
     tar -xvf doxygen-1.9.6.src.tar
     cd doxygen-1.9.6

     mkdir build
     cd build

     cmake -L ..

     cmake -G "Unix Makefiles" -Dbuild_search=ON -Duse_libclang=YES -Denable_coverage=ON -Dbuild_parse=ON ..

     make -j 3   # Watch your memory and CPU utilization... This needs at least 4G of memory
     make -j 1   # Takes 10 minutes to build, but can be done in 2G of memory

     make test

     sudo make install
   ````
   
   As root:
   ````
     sudo pacman -R xapian-core
     sudo pacman -R llvm    # This is 368MB, but it's only needed to compile DOxygen.  We will remove it after.
   ````

## To integrate Doxygen with Visual Studio
   - TODO... but I wire it into Tools

## To Run Doxygen
   - `cd` to the directory and simply run `doxygen` 

## To Configure Doxygen
   - I clone the configuration from an existing app, then pick through the config
     line-by-line until I'm happy with it.  I have a lot of customizations, but
     it looks good the way I publish it.
