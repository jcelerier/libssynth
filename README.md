# libssynth

The mesh generation abilities of Structure Synth (http://structuresynth.sourceforge.net, created by Mikael Hvidtfeldt Christensen) 
put in a convenient library with fewer Qt dependencies.

- Improved performance a bit and modernized code with clang-tidy, fixed a few memory leaks as well
- Ensured that it passes clean through ASAN & UBSAN.
- Quick CLI to test. First argument is the .es file, second argument is the .rendertemplate file. If not provided, 
  the model is exported to .obj instead.

License follows the original Structure Synth license.
