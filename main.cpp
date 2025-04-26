#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <zlib.h>

using namespace std;

const int CHUNK_SIZE = 1024 * 1024;
mutex writeMutex;

void compressChunk(const vector<char>& input, vector<uLongf>& outputSizes, vector<vector<Bytef>>& compressedData, int index) {
    uLongf compSize = compressBound(input.size());
    vector<Bytef> out(compSize);
    if (compress(out.data(), &compSize, reinterpret_cast<const Bytef*>(input.data()), input.size()) == Z_OK) {
        lock_guard<mutex> lock(writeMutex);
        compressedData[index] = vector<Bytef>(out.begin(), out.begin() + compSize);
        outputSizes[index] = compSize;
    }
}

void compressFile(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile, ios::binary);
    vector<vector<char>> chunks;
    while (!in.eof()) {
        vector<char> buffer(CHUNK_SIZE);
        in.read(buffer.data(), CHUNK_SIZE);
        buffer.resize(in.gcount());
        if (!buffer.empty()) chunks.push_back(move(buffer));
    }
    in.close();

    int numChunks = chunks.size();
    vector<vector<Bytef>> compressedData(numChunks);
    vector<uLongf> outputSizes(numChunks);
    vector<thread> threads;

    for (int i = 0; i < numChunks; ++i) {
        threads.emplace_back(compressChunk, cref(chunks[i]), ref(outputSizes), ref(compressedData), i);
    }

    for (auto& t : threads) t.join();

    ofstream out(outputFile, ios::binary);
    for (int i = 0; i < numChunks; ++i) {
        uLongf size = outputSizes[i];
        out.write(reinterpret_cast<char*>(&size), sizeof(size));
        out.write(reinterpret_cast<char*>(compressedData[i].data()), size);
    }
    out.close();
}

int main() {
    compressFile("input.txt", "output.zlib");
    return 0;
}