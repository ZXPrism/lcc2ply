# lcc2ply

A simple tool for converting 3dgs .lcc format to .ply format.

## Notes
Currently, only supports parsing foreground data with SH band 0.

Parsing background data as well as higher SH bands are easy to implement based on current codes, which will be added later (or not).

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
- `./lcc2ply -i scenes/PentHouse --analyze`
