#include "read_ts.h"
#include <fstream>
#include <stdexcept>

std::vector<std::vector<unsigned char>> read_ts(const std::string& filePath) {
    std::ifstream tsFile(filePath, std::ios::binary);
    if (!tsFile) {
        throw std::runtime_error("Nao foi possivel abrir o arquivo");
    }

    std::vector<std::vector<unsigned char>> blocks;
    std::vector<unsigned char> packet(TS_PACKET_SIZE);

    try {
        while (tsFile.read(reinterpret_cast<char*>(packet.data()), TS_PACKET_SIZE)) {
            if (packet[0] != SYNC_BYTE) {
                throw std::runtime_error("Synchronization error: O pacote nao comeca com SYNC_BYTE");
            }
            blocks.push_back(packet);
        }
    } catch (...) {
        tsFile.close();
        throw;
    }
    
    tsFile.close();
    return blocks;
}

