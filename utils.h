#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <vector>
#include <iomanip>
#include "iden_pid.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <fstream>
#include <vector>

extern const int TS_PACKET_SIZE;
extern const unsigned int SYNC_BYTE;
const unsigned int PID_MASK = 0x1FFF; // Máscara para extrair o PID de 13 bits dos pacotes TS.


enum class PayloadType {
    Unknown,
    Other,
    Table,
    PES
};

struct TSHeader {
    bool tei;
    bool pusi;
    bool tp;
    int pid;
    int scramblingControl;
    int adaptationFieldControl;
    int continuityCounter;
};


void print_packet(const std::vector<unsigned char>& packet);
void save_pes_to_file(const std::vector<std::vector<unsigned char>>& pes_packets, const std::string& filename);

void extractPES(const std::vector<std::vector<unsigned char>>& tsBlocks, const std::set<int>& targetPIDs, std::vector<std::vector<unsigned char>>& pesPackets);


std::vector<std::vector<unsigned char>> filter_blocks_by_pid(const std::vector<std::vector<unsigned char>>& blocks, const std::set<int>& selected_pids);
std::pair<std::vector<std::map<std::string, std::string>>, std::vector<std::vector<unsigned char>>> process_ts_packets(const std::vector<std::vector<unsigned char>>& packets);

PayloadType determine_payload_type(const std::vector<unsigned char>& payload);
std::set<PayloadType> get_payload_type(const std::vector<std::vector<unsigned char>>& filteredBlocks);

#endif
