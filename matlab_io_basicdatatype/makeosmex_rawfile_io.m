%% This file is generated in case I forget the right way to compile next
%% time, :-)
%%
%%by Hanchuan Peng
%% 2007-03-06
% last update: 090722: by Hanchuan Peng. This makes it easire to compile all mex files under this directory

mex loadRaw2Stack_c.cpp mg_image_lib.cpp mg_utilities.cpp -ltiff
mex saveStack2File_c.cpp mg_image_lib.cpp mg_utilities.cpp -ltiff

mex checkMachineEndian.cpp


