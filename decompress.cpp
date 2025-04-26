#include <iostream>
#include <fstream>
#include <vector>
#include <zlib.h>

using namespace std;

void decompressFile(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile, ios::binary);
    ofstream out(outputFile, ios::binary);

    while (in.peek() != EOF) {
        uLongf compSize;
        in.read(reinterpret_cast<char*>(&compSize), sizeof(compSize));
        vector<Bytef> compData(compSize);
        in.read(reinterpret_cast<char*>(compData.data()), compSize);

        uLongf destSize = compSize * 4;
        vector<Bytef> decompData(destSize);
        if (uncompress(decompData.data(), &destSize, compData.data(), compSize) == Z_OK) {
            out.write(reinterpret_cast<char*>(decompData.data()), destSize);
        } else {
            cerr << "Decompression failed.\n";
        }
    }

    in.close();
    out.close();
}

int main() {
    decompressFile("output.zlib", "output.txt");
    return 0;
}