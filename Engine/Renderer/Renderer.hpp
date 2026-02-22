#pragma once
#include <SFML/Graphics.hpp>
#include <thread>
#include "../Threading/SharedState.hpp"
#include "../UI/UI.hpp"

// ============================================================
//  Renderer  –  roda em thread dedicada
//
//  Regra crítica SFML 3 (da documentação oficial):
//    "deactivate the window in the main thread before launching
//     the rendering thread, then activate it inside that thread"
//
//  Fluxo:
//    1. main() cria janela
//    2. main() chama window.setActive(false)
//    3. main() lança Renderer::start()
//    4. Renderer::loop() chama window.setActive(true)
//    5. Render loop roda independente do event loop
// ============================================================

class Renderer {
public:
    Renderer(sf::RenderWindow& window, SharedState& state, const sf::Font& font)
        : m_window(window)
        , m_state(state)
        , m_sidebar(window.getSize().y, font)
        , m_console(window.getSize().x,
                    static_cast<float>(window.getSize().y),
                    font){}

    // Lança a thread de render
    void start() {
        m_thread = std::thread(&Renderer::loop, this);
    }

    // Aguarda a thread terminar (chamado após o event loop fechar)
    void join() {
        if (m_thread.joinable())
            m_thread.join();
    }

    // Expõe o sidebar para o event loop delegar input de mouse
    Sidebar& sidebar() { return m_sidebar; }

private:
    void loop() {
        // Ativa o contexto OpenGL nesta thread
        m_window.setActive(true);
        m_window.setFramerateLimit(60);

        // Área de conteúdo (direita da sidebar, acima do console)
        sf::RectangleShape contentArea;
        contentArea.setPosition({Sidebar::WIDTH, 0.f});
        contentArea.setSize({
            m_window.getSize().x - Sidebar::WIDTH,
            m_window.getSize().y - ConsoleBar::HEIGHT
        });
        contentArea.setFillColor(Theme::BG);

        // Divisor vertical entre sidebar e conteúdo
        sf::RectangleShape divider;
        divider.setPosition({Sidebar::WIDTH - 1.f, 0.f});
        divider.setSize({1.f, static_cast<float>(m_window.getSize().y)});
        divider.setFillColor(Theme::ACCENT);

        while (m_state.running) {
            m_window.clear(Theme::SIDEBAR_BG);

            // Áreas de fundo
            m_window.draw(contentArea);
            m_window.draw(divider);

            // UI – sidebar (botões)
            m_sidebar.draw(m_window);

            // UI – console com snapshot thread-safe
            auto lines = m_state.snapshotConsole();
            m_console.draw(m_window, lines);

            m_window.display();
        }

        // Devolve contexto antes de sair
        m_window.setActive(false);
    }

    sf::RenderWindow& m_window;
    SharedState&      m_state;
    Sidebar           m_sidebar;
    ConsoleBar        m_console;
    std::thread       m_thread;
};