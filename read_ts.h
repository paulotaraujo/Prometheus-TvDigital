#ifndef READTS_H
#define READTS_H

#include <vector>
#include <string>

extern const int TS_PACKET_SIZE;
extern const unsigned int SYNC_BYTE;

// Declara a função read_ts que será definida em read_ts.cpp
std::vector<std::vector<unsigned char>> read_ts(const std::string& filePath);

#endif // READTS_H

