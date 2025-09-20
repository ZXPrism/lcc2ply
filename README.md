# lcc2ply

a simple tool to convert 3dgs .lcc format to .ply format.

## Getting Started
1. Ensure you have a C++20 compatible compiler installed and properly configured in your system's PATH
2. Install [xmake](https://xmake.io/)
3. Build project
    ```sh
    git clone https://github.com/ZXPrism/lcc2ply.git
    cd lcc2ply
    xmake -y
    ```
4. The built executable will be available in the ./bin/ directory.

## Example Usage
- `./lcc2ply -i scenes/PentHouse -o output.ply --lod 0`
