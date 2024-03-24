#include <iostream>
#include <fstream>
#include <vector>

const int TS_PACKET_SIZE = 188; // Define o tamanho de cada pacote TS.
const unsigned int SYNC_BYTE = 0x47; // Byte de sincronização para identificar o início de um pacote TS.
const unsigned int PID_MASK = 0x1FFF; // Máscara para extrair o PID de 13 bits dos pacotes TS.

// Função para extrair e salvar os dados do PES baseado em um PID específico.
void extractPES(const char* filename, unsigned int targetPID) {
    std::ifstream tsFile(filename, std::ifstream::binary); // Abre o arquivo TS em modo binário.
    if (!tsFile.is_open()) {
        std::cerr << "Erro ao abrir o arquivo!" << std::endl;
        return; // Se não conseguir abrir o arquivo, encerra a função.
    }

    std::vector<unsigned char> pesData; // Vetor para acumular os dados do PES.
    bool payloadStart = false; // Indica se o pacote atual marca o início de um PES.

    // Lê pacotes TS do arquivo até o fim.
    while (!tsFile.eof()) {
        unsigned char packet[TS_PACKET_SIZE]; // Buffer para um pacote TS.
        tsFile.read(reinterpret_cast<char*>(packet), TS_PACKET_SIZE); // Lê um pacote do arquivo.

        if (tsFile.gcount() < TS_PACKET_SIZE) break; // Se leu menos que um pacote completo, termina o loop.

        if (packet[0] != SYNC_BYTE) {
            std::cerr << "Sincronização perdida!" << std::endl;
            continue; // Se o byte de sincronização não estiver presente, ignora este pacote.
        }

        // Extrai o PID do pacote.
        unsigned int pid = ((packet[1] & PID_MASK) << 8) | packet[2];
        if (pid != targetPID) continue; // Se o PID não for o alvo, ignora este pacote.

        // Verifica o bit de início de payload.
        payloadStart = packet[1] & 0x40;

        // Determina o início da carga útil.
        int adaptationFieldControl = (packet[3] >> 4) & 0x3;
        int payloadOffset = 4; // O cabeçalho do TS tem 4 bytes.
        if (adaptationFieldControl == 2 || adaptationFieldControl == 3) {
            // Se houver campo de adaptação, ajusta o deslocamento da carga útil.
            payloadOffset += 1 + packet[4];
        }

        // Se houver carga útil, adiciona ao vetor do PES.
        if (adaptationFieldControl == 1 || adaptationFieldControl == 3) {
            if (payloadStart && !pesData.empty()) {
                // Se é o início de um novo PES, processa o PES anterior.
                pesData.clear(); // Limpa os dados para o novo PES.
            }
            // Adiciona a carga útil ao vetor.
            pesData.insert(pesData.end(), &packet[payloadOffset], &packet[TS_PACKET_SIZE]);
        }
    }

    tsFile.close(); // Fecha o arquivo TS.

    // Processar o último conjunto de dados do PES acumulado salvando em um arquivo:
    std::ofstream outFile("output.pes", std::ofstream::binary);
    if (outFile.is_open()) {
        outFile.write(reinterpret_cast<const char*>(pesData.data()), pesData.size());
        outFile.close(); // Fecha o arquivo de saída.
    } else {
        std::cerr << "Não foi possível abrir o arquivo de saída para escrever dados do PES." << std::endl;
    }
}