#pragma once
#include <SFML/Graphics.hpp>
#include "../Application/Application.hpp"
#include "../Renderer/Renderer.hpp"
#include "../Events/EventBus.hpp"

// ============================================================
//  Application  –  orquestra tudo
//
//  Main thread faz APENAS:
//    - Criar a janela
//    - Chamar pollEvent
//    - Traduzir eventos SFML → EventBus
//    - Delegar input de mouse para UI (hit-test)
//
//  Render thread (dentro de Renderer) faz:
//    - clear / draw / display
// ============================================================

class Application {
public:
    Application()
        : m_window(sf::VideoMode({1280, 720}), "ECS SFML Engine")
        , m_renderer(m_window, m_state, m_font)
    {
        // Carrega fonte embutida do sistema (fallback simples)
        // Em produção, use um .ttf no diretório do projeto
        try {
            m_font = sf::Font("C:/Windows/Fonts/consola.ttf");
        } catch (...) {
            try {
                m_font = sf::Font("C:/Windows/Fonts/arial.ttf");
            } catch (...) {
                // Sem fonte disponível — textos não aparecem
            }
        }

        // ── Conecta EventBus ──────────────────────────────────
        // Quando qualquer botão for clicado, escreve no console
        EventBus::instance().subscribe("button_clicked",
            [this](const std::string& label) {
                m_state.pushConsole("[click] " + label);
            });
    }

    void run() {
        // ── CRÍTICO: desativa contexto OpenGL antes de lançar
        //    o render thread (regra SFML 3 para multithreading)
        m_window.setActive(false);
        m_renderer.start();

        // ── Event loop (main thread) ──────────────────────────
        while (m_window.isOpen()) {
            while (auto event = m_window.pollEvent()) {

                if (event->is<sf::Event::Closed>()) {
                    m_state.running = false;
                    m_window.close();
                }

                // Captura redimensionamento
                if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                    sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
                    m_window.setView(sf::View(visibleArea));

                    std::string msg = "Nova largura: " + std::to_string(resized->size.x)
                    + " | Nova altura: " + std::to_string(resized->size.y);
                    m_state.pushConsole(msg);
                }

                // Mouse move → hover state dos botões
                if (const auto* mm = event->getIf<sf::Event::MouseMoved>()) {
                    sf::Vector2f pos{
                        static_cast<float>(mm->position.x),
                        static_cast<float>(mm->position.y)
                    };
                    m_renderer.sidebar().handleMouseMove(pos);
                }

                // Mouse press → detecta clique nos botões
                if (const auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mp->button == sf::Mouse::Button::Left) {
                        sf::Vector2f pos{
                            static_cast<float>(mp->position.x),
                            static_cast<float>(mp->position.y)
                        };
                        m_renderer.sidebar().handleMousePress(pos);
                    }
                }

                // Mouse release → restaura visual dos botões
                if (const auto* mr = event->getIf<sf::Event::MouseButtonReleased>()) {
                    if (mr->button == sf::Mouse::Button::Left) {
                        sf::Vector2f pos{
                            static_cast<float>(mr->position.x),
                            static_cast<float>(mr->position.y)
                        };
                        m_renderer.sidebar().handleMouseRelease(pos);
                    }
                }

                // Teclas de atalho (1-6) mapeadas para os botões
                if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                    handleKeyShortcut(kp->code);
                }
            }
        }

        // Aguarda render thread terminar limpo
        m_renderer.join();
    }

private:
    void handleKeyShortcut(sf::Keyboard::Key key) {
        static const std::vector<std::string> names = {
            "Spawn Entity", "Clear Scene", "Toggle Debug",
            "Run System",   "Load Asset",  "Save State"
        };

        int idx = -1;
        if (key == sf::Keyboard::Key::Num1 || key == sf::Keyboard::Key::Numpad1) idx = 0;
        if (key == sf::Keyboard::Key::Num2 || key == sf::Keyboard::Key::Numpad2) idx = 1;
        if (key == sf::Keyboard::Key::Num3 || key == sf::Keyboard::Key::Numpad3) idx = 2;
        if (key == sf::Keyboard::Key::Num4 || key == sf::Keyboard::Key::Numpad4) idx = 3;
        if (key == sf::Keyboard::Key::Num5 || key == sf::Keyboard::Key::Numpad5) idx = 4;
        if (key == sf::Keyboard::Key::Num6 || key == sf::Keyboard::Key::Numpad6) idx = 5;

        if (idx >= 0) {
            EventBus::instance().publish("button_clicked",
                std::to_string(idx + 1) + ". " + names[idx]);
        }
    }

    sf::Font         m_font;
    sf::RenderWindow m_window;
    SharedState      m_state;
    Renderer         m_renderer;
};