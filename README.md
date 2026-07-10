# GPU Whisperer

Tool to gently persuade GPUs to give up their inner workings of how compressed texture data is treated. 

It needs [GLFW](//www.glfw.org/) to build. On Mac, either install using [Homebrew](//formulae.brew.sh/formula/glfw) or build from source:
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
sudo apt install libglfw3-dev libdecor-0-dev
```
