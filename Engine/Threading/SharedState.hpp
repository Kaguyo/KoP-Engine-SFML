#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

// ============================================================
//  SharedState  –  dados trocados entre main thread e render
//  thread de forma segura.
//
//  Regra SFML 3:
//    - pollEvent  → main thread (onde a janela foi criada)
//    - clear/draw/display → render thread (após setActive(true))
//
//  Qualquer dado lido pelo render thread e escrito pelo main
//  thread deve ser protegido por mutex ou atomic.
// ============================================================

struct SharedState {
    // Sinaliza ao render thread que deve parar
    std::atomic<bool> running{ true };

    // Log de mensagens exibido no console bar
    std::vector<std::string> consoleLines;
    std::mutex consoleMutex;

    void pushConsole(const std::string& msg) {
        std::lock_guard lock(consoleMutex);
        consoleLines.push_back(msg);
        // Limita histórico a 20 linhas
        if (consoleLines.size() > 20)
            consoleLines.erase(consoleLines.begin());
    }

    // Captura thread-safe de todas as linhas para o render
    std::vector<std::string> snapshotConsole() {
        std::lock_guard lock(consoleMutex);
        return consoleLines;
    }
};