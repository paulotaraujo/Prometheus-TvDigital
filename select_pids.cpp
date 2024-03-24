#include "select_pids.h"
#include <iostream>
#include <set>
#include <stdexcept>

std::set<int> select_pids(const std::set<int>& all_pids) {
    std::set<int> selected_pids;
    
    // Imprime os PIDs para o usuário
    std::cout << "PIDs presentes nos pacotes:" << std::endl;
    for (int pid : all_pids) {
        std::cout << pid << std::endl;
    }

    int num_pids_to_select = 1;

    // std::cout << "Quantos PIDs você deseja selecionar? (Devido a limitações, apena um pid sera tratado)";
    // std::cin >> num_pids_to_select;

    if (num_pids_to_select <= 0 || num_pids_to_select > all_pids.size()) {
        throw std::invalid_argument("Número inválido de PIDs a serem selecionados.");
    }

    std::cout << "Digite os " << num_pids_to_select << " PIDs desejados separados por espaço:" << std::endl;
    int pid;
    for (int i = 0; i < num_pids_to_select; ++i) {
        while (true) {
            std::cin >> pid;
            // Verifica se o PID existe na lista de todos os PIDs
            if (all_pids.count(pid) > 0) {
                selected_pids.insert(pid);
                break;
            } else {
                std::cout << "O PID " << pid << " não está presente na lista de PIDs mostrados. Por favor, insira um PID válido." << std::endl;
            }
        }
    }

    return selected_pids;
}
