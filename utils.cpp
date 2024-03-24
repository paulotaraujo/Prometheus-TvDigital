#include "utils.h"

void print_packet(const std::vector<unsigned char>& packet) {
    std::cout << "Tamanho do pacote: " << packet.size() << " bytes" << std::endl;
    std::cout << "Conteudo do pacote:" << std::endl;

    for (size_t i = 0; i < packet.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(packet[i]) << " ";
        if ((i + 1) % 16 == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

void extract_header(const std::vector<unsigned char>& packet) {
    // Primeiro byte é sempre o byte de sincronização
    if (packet[0] != SYNC_BYTE) {
        std::cerr << "Erro de sincronizacao: O pacote nao comeca com SYNC_BYTE" << std::endl;
        return;
    }

    // Extrai os bits do cabecalho do pacote TS
    unsigned char header[4];
    for (int i = 0; i < 4; ++i) {
        header[i] = packet[i];
    }

    // Imprime os bytes do cabecalho em hexadecimal
    std::cout << "Cabecalho do pacote TS:";
    for (int i = 0; i < 4; ++i) {
        std::cout << " 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(header[i]);
    }
    std::cout << std::endl;

    // Extrai informações do cabecalho
    unsigned short pid = ((header[1] & 0x1F) << 8) | header[2];
    std::cout << "PID: " << std::dec << pid << std::endl;

    bool transport_error_indicator = (header[1] & 0x80) >> 7;
    std::cout << "Transport Error Indicator: " << (transport_error_indicator ? "true" : "false") << std::endl;

}

std::vector<std::vector<unsigned char>> filter_blocks_by_pid(const std::vector<std::vector<unsigned char>>& blocks, const std::set<int>& selected_pids) {
    std::vector<std::vector<unsigned char>> filteredBlocks;

    for (const auto& block : blocks) {
        std::set<int> pids = identify_pids({block});
        for (int pid : pids) {
            if (selected_pids.find(pid) != selected_pids.end()) {
                filteredBlocks.push_back(block);
                break; // Se encontrar o PID, não precisa verificar os outros PIDs do bloco
            }
        }
    }

    return filteredBlocks;
}

std::map<std::string, std::string> decode_header(const std::vector<unsigned char>& packet) {
    std::map<std::string, std::string> header;

    // Decodificação dos campos do cabeçalho
    header["Sync Byte"] = std::to_string(packet[0]);
    header["TEI"] = std::to_string((packet[1] >> 7) & 0x01);
    header["PUSI"] = std::to_string((packet[1] >> 6) & 0x01);
    header["TP"] = std::to_string((packet[1] >> 5) & 0x01);
    header["PID"] = std::to_string(((packet[1] & 0x1F) << 8) | packet[2]);
    header["Scrambling Control"] = std::to_string((packet[3] >> 6) & 0x03);
    header["Adaptation Field Control"] = std::to_string((packet[3] >> 4) & 0x03);
    header["Continuity Counter"] = std::to_string(packet[3] & 0x0F);

    return header;
}

std::pair<std::vector<std::map<std::string, std::string>>, std::vector<std::vector<unsigned char>>> process_ts_packets(const std::vector<std::vector<unsigned char>>& packets) {
    std::vector<std::map<std::string, std::string>> headers;
    std::vector<std::vector<unsigned char>> payloads;

    for (const auto& packet : packets) {
        std::map<std::string, std::string> header = decode_header(packet);

        headers.push_back(header);

        // Extração do campo de adaptação, se presente
        if ((header["Adaptation Field Control"] == "2" || header["Adaptation Field Control"] == "3") && packet.size() > 4) {
            // O campo de adaptação começa no quarto byte após o cabeçalho
            // e seu primeiro byte indica o tamanho total do campo de adaptação
            int adaptationFieldLength = packet[4];
            if (adaptationFieldLength > 0) {
                // Se o campo de adaptação tiver um ou mais bytes, podemos extrair e armazenar
                std::vector<unsigned char> adaptationField(packet.begin() + 5, packet.begin() + 5 + adaptationFieldLength);
                // Aqui você pode processar ou armazenar o campo de adaptação conforme necessário
            }
        }

        // Extrair o payload
        if (header["Adaptation Field Control"] != "1" && packet.size() > 4) {
            std::vector<unsigned char> payload(packet.begin() + 4, packet.end());
            payloads.push_back(payload);
        }
    }

    return {headers, payloads};
}

PayloadType determine_payload_type(const std::vector<unsigned char>& payload) {
    // Verificar se o payload corresponde a uma tabela
    if (payload.size() >= 4 && payload[0] == 0x00 && payload[1] == 0xB0) {
        return PayloadType::Table; // PAT (Program Association Table)
    }
    if (payload.size() >= 4 && payload[0] == 0x02 && payload[1] == 0xB0) {
        return PayloadType::Table; // PMT (Program Map Table)
    }

    // Verificar se o payload corresponde a um pacote de fluxo elementar (PES)
    if (payload.size() >= 6 && payload[0] == 0x00 && payload[1] == 0x00 && payload[2] == 0x01) {
        return PayloadType::PES;
    }

    return PayloadType::Unknown;

}

void save_pes_to_file(const std::vector<std::vector<unsigned char>>& pes_packets, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo para escrita." << std::endl;
        return;
    }

    // Escreve o conteúdo de cada pacote PES no arquivo
    for (const auto& pes_payload : pes_packets) {
        file.write(reinterpret_cast<const char*>(pes_payload.data()), pes_payload.size());
    }

    file.close();
}

// Função para extrair e salvar os dados do PES baseado em um PID específico.
void extractPES(const std::vector<std::vector<unsigned char>>& tsBlocks, const std::set<int>& targetPIDs, std::vector<std::vector<unsigned char>>& pesPackets) {
    std::vector<unsigned char> pesData; // Vetor para acumular os dados do PES.
    bool payloadStart = false; // Indica se o pacote atual marca o início de um PES.

    // Itera sobre os blocos de TS.
    for (const auto& block : tsBlocks) {
        if (block.size() != TS_PACKET_SIZE) {
            std::cerr << "Tamanho inválido de bloco TS!" << std::endl;
            continue; // Ignora blocos com tamanho incorreto.
        }

        // Extrai o PID do pacote.
        unsigned int pid = ((block[1] & PID_MASK) << 8) | block[2];
        if (targetPIDs.find(pid) == targetPIDs.end()) continue; // Se o PID não está na lista de alvos, ignora este pacote.

        // Verifica o bit de início de payload.
        payloadStart = block[1] & 0x40;

        // Determina o início da carga útil.
        int adaptationFieldControl = (block[3] >> 4) & 0x3;
        int payloadOffset = 4; // O cabeçalho do TS tem 4 bytes.
        if (adaptationFieldControl == 2 || adaptationFieldControl == 3) {
            // Se houver campo de adaptação, ajusta o deslocamento da carga útil.
            payloadOffset += 1 + block[4];
        }

        // Se houver carga útil, adiciona ao vetor do PES.
        if (adaptationFieldControl == 1 || adaptationFieldControl == 3) {
            if (payloadStart && !pesData.empty()) {
                // Se é o início de um novo PES, processa o PES anterior.
                pesPackets.push_back(pesData); // Adiciona o pacote PES ao vetor de pacotes PES.
                pesData.clear(); // Limpa os dados para o novo PES.
            }
            // Adiciona a carga útil ao vetor.
            pesData.insert(pesData.end(), block.begin() + payloadOffset, block.end());
        }
    }

    // Processar o último conjunto de dados do PES acumulado, se houver:
    if (!pesData.empty()) {
        pesPackets.push_back(pesData); // Adiciona o último pacote PES ao vetor de pacotes PES.
    }
}

std::set<PayloadType> get_payload_type(const std::vector<std::vector<unsigned char>>& filteredBlocks) {
    std::set<PayloadType> payloadTypes;

    for (const auto& block : filteredBlocks) {
        if (block.size() >= TS_PACKET_SIZE && block[0] == SYNC_BYTE) {
            int adaptationFieldControl = (block[3] >> 4) & 0x3;
            if (adaptationFieldControl == 1 || adaptationFieldControl == 3) {
                // Payload presente
                int payloadStart = block[1] & 0x40;
                if (payloadStart) {
                    // Pacote PES
                    payloadTypes.insert(PayloadType::PES);
                } else {
                    // Outro tipo de payload (Tabela, etc.)
                    payloadTypes.insert(PayloadType::Other);
                }
            }
        }
    }

    return payloadTypes;
}