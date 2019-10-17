Build instructions for Vaa3D/Mozak with support for a gamecontroller and a spacenavigator mouse.
================================================================================================

The gamecontroller and spacenavigator mouse code in Vaa3D relies on a few Thirdparty libraries. These libraries need to be built before Vaa3D/Mozak is built.

For any third party library, binaries (and associated headers/libs) are compiled into one build folder of your choice, e.g. c:\\vs2013Build

The DSL library
===============
Checkout the master branch of the DSL library from https://github.com/TotteKarlsson/dsl

Step 1: 
Use CMake to configure and generate build files (solution files) for Visual Studio and DSL thirdparties, found in the Thirdparties folder. 

Set 2:
Use CMake to configure and generate build files (solution files) for Visual Studio and DSL library. 

Step 3:
Build and install the libraries

GameController code
===================
Step 1: 
Checkout the master branch of the game controller library from https://github.com/AllenInstitute/gamecontroller-lib
Use CMake to configure and generate build files (solution files) for Visual Studio. 

Step 2: 
Build and install the library


Space Navigator 
===============
Step 1:
Download and install drivers for the spacenavigator from: https://www.3dconnexion.com/service/drivers.html

Step 2:
Checkout the master branch of a simple wrapper around the SpaceNavigators SDK from: https://github.com/AllenInstitute/3dx-lib/tree/master

Step 3:
Use CMake to configure and generate build files (solution files) for Visual Studio and DSL library. 

Step 4:
Build and install the library





