#ifndef IDEN_PID_H
#define IDEN_PID_H

#include <vector>
#include <set>
#include <iostream>
#include <stdexcept>

extern const int TS_PACKET_SIZE;
extern const unsigned int SYNC_BYTE;

// Função para extrair os PIDs dos pacotes TS
std::set<int> identify_pids(const std::vector<std::vector<unsigned char>>& blocks);

#endif // IDEN_PID_H