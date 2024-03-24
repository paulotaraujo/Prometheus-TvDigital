#ifndef SELECT_PIDS_H
#define SELECT_PIDS_H

#include <set>

extern const int TS_PACKET_SIZE;
extern const unsigned int SYNC_BYTE;

// Função para selecionar os PIDs desejados a partir de um conjunto de todos os PIDs encontrados
std::set<int> select_pids(const std::set<int>& all_pids);

#endif // SELECT_PIDS_H
