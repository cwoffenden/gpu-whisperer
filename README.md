# BC Values

Tool to extract S3TC (BC/DXT) compressed values from hardware. 

It needs GLFW to build. On Mac, either install using [Homebrew](//formulae.brew.sh/formula/glfw) or build from source:
```
git clone https://github.com/glfw/glfw.git GLFW
cd GLFW
mkdir build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
sudo cmake --install build
```
On Linux, tested on Debian, do:
```
sudo apt install libglfw3-dev
```
