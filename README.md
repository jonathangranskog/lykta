# Lykta - A Work-In-Progress Physically-Based Renderer

### Introduction

Lykta is a CPU renderer that is not meant for any practical usage. It's purely a hobby renderer for myself to use when I want to implement or try new ideas. However, the goal is to keep the renderer fairly simple, e.g. support only triangle meshes, a single material model and only mesh emitters. 

Future plans include possibly creating a Houdini plug-in...

### Building

This project uses CMake version 3.12 or higher for building. Everything should work automatically on Windows and Linux (not tested), but on OS X some additional work is necessary.

The project requires Embree to be installed on the computer and discoverable by CMake. Follow the installation instructions at: https://embree.github.io/downloads.html

Clone the repository using Git
```
git clone --recursive https://github.com/jonathangranskog/lykta.git
```

To build the project, use the following commands or use cmake-gui on Windows.
```
cd lykta
mkdir build
cd build
cmake ..
make
```

#### OS X
Unfortunately, OpenMP is not fully supported by OS X at this moment. However, you can easily gain access to it by using Brew.

```
brew install libomp
```

The CMake build process assumes the default Brew installation on OS X so if you change your install paths then you might have to modify the CMakeLists.txt file. 

### Example scene file:

```
{
  "camera" : {
    "type": "PerspectiveCamera",
    "resolution" :  [800, 800],
    "center": [ 0.0, 0.5, 1.05 ],
    "lookat": [0.0, 0.5, 0.0],
    "fov": 45.0,
    "nearclip": 0.01,
    "farclip" : 100.0
  },
  "objects": [
    {

      "file": "E:/Projects/lykta/spheres.obj",
      "material": "white"
    },
    {

      "file": "E:/Projects/lykta/emitter.obj",
      "material": "emitter"
    }
  ],
  "materials": [
    {
      "name": "white",
      "diffuseColor": [1.0, 1.0, 1.0],
      "emission" : [0.0, 0.0, 0.0]
    },
    {
      "name": "emitter",
      "diffuseColor": [ 0.0, 0.0, 0.0 ],
      "emissiveColor" :  [10.0, 10.0, 10.0]
    }
  ]
}
```