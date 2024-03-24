#include "read_ts.h"
#include "iden_pid.h"
#include "select_pids.h"
#include "utils.h"
#include <iostream>
#include <set>
#include <vector>
#include <stdexcept>

const int TS_PACKET_SIZE = 188; // Define o tamanho de cada pacote TS.
const unsigned int SYNC_BYTE = 0x47; // Byte de sincronização para identificar o início de um pacote TS.


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << "/Downloads/Prometheus-TvDigital-Equipe-Spider-Man/pattaya-aerial-view30.ts" << std::endl;
        return 1;
    }

    try {
        // Usa o primeiro argumento da linha de comando como o caminho do arquivo TS
        std::cout << "read_ts" << std::endl;
        auto blocks = read_ts(argv[1]);
        std::cout << "O arquivo foi lido com sucesso. Numero de pacotes: " << blocks.size() << std::endl;

        std::cout << "identify_pids" << std::endl;
        // Identifica todos os PIDs no ts
        std::set<int> all_pids = identify_pids(blocks);

        std::cout << "select_pids" << std::endl;
        // Seleciona os PIDs desejados
        std::set<int> selected_pids = select_pids(all_pids);
        std::cout << "Os PIDs selecionados foram:" << std::endl;
        for (int pid : selected_pids) {
            std::cout << pid << std::endl;
        }

        int pid = *selected_pids.begin();

        std::cout << "filter_blocks_by_pid" << std::endl;
        // Filtrar os blocos com os PIDs desejados
        auto filteredBlocks = filter_blocks_by_pid(blocks, selected_pids);

        std::cout << "payload_type" << std::endl;
        // Identificar o tipo do pacote 
        std::set<PayloadType> payload_type = get_payload_type(filteredBlocks);

        auto it = payload_type.find(PayloadType::PES);
        if (it != payload_type.end()){
            std::cout << "pes_packets" << std::endl;
            std::vector<std::vector<unsigned char>> pesPackets;
            extractPES(filteredBlocks, selected_pids, pesPackets);
                std::cout << "save_pes_to_file" << std::endl;
            // Salvar o payload caso for um PES
            save_pes_to_file(pesPackets, "output_pes.bin");
            std::cout << "Pacotes de fluxo elementar (PES) salvos com sucesso." << std::endl;
        }
        
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}



        // std::cout << "process_ts_packets" << std::endl;
        // // Separar e identificar o header e o payload do Ts
        // auto result = process_ts_packets(filteredBlocks);