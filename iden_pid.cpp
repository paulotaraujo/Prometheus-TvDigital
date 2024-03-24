#include "iden_pid.h"

std::set<int> identify_pids(const std::vector<std::vector<unsigned char>>& blocks) {
    std::set<int> pids; // Usamos um set para garantir PIDs únicos

    // Iterar sobre cada pacote para extrair os PIDs
    for (const auto& packet : blocks) {
        // Lógica de extração do PID
        if (packet.size() >= TS_PACKET_SIZE && packet[0] == SYNC_BYTE) {
            int pid = ((packet[1] & 0x1F) << 8) | packet[2];
            pids.insert(pid);
        }
    }

    return pids;
}


