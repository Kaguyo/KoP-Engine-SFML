#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>
#include "../Events/EventBus.hpp"

// ============================================================
//  Componentes de UI
//
//  SidebarButton  – botão clicável que publica no EventBus
//  Sidebar        – painel lateral com lista de botões
//  ConsoleBar     – painel inferior que exibe linhas de log
// ============================================================

// ── Paleta de cores ─────────────────────────────────────────
namespace Theme {
    inline const sf::Color BG          { 18,  18,  28};
    inline const sf::Color SIDEBAR_BG  { 28,  28,  42};
    inline const sf::Color CONSOLE_BG  { 12,  12,  20};
    inline const sf::Color BTN_NORMAL  { 45,  45,  70};
    inline const sf::Color BTN_HOVER   { 70,  70, 110};
    inline const sf::Color BTN_PRESS   {100, 100, 160};
    inline const sf::Color TEXT_PRIMARY{220, 220, 255};
    inline const sf::Color TEXT_DIM    {120, 120, 160};
    inline const sf::Color ACCENT      { 90, 140, 255};
}

// ── SidebarButton ────────────────────────────────────────────
class SidebarButton {
public:
    SidebarButton(const std::string& label,
                  sf::Vector2f pos,
                  sf::Vector2f size,
                  const sf::Font& font): m_label(label), m_text(font) // SFML 3: fonte obrigatória no construtor
    {
        m_rect.setPosition(pos);
        m_rect.setSize(size);
        m_rect.setFillColor(Theme::BTN_NORMAL);
        m_rect.setOutlineThickness(1.f);
        m_rect.setOutlineColor(Theme::ACCENT);

        m_text.setString(label);
        m_text.setCharacterSize(14);
        m_text.setFillColor(Theme::TEXT_PRIMARY);

        // Centraliza texto verticalmente no botão
        auto tb = m_text.getLocalBounds();
        m_text.setOrigin({tb.position.x + tb.size.x,
                          tb.position.y + tb.size.y / 2.f});
        m_text.setPosition({pos.x + size.x / 2.f,
                            pos.y + size.y / 2.f});
    }

    bool handleMouseMove(sf::Vector2f mouse) {
        bool over = m_rect.getGlobalBounds().contains(mouse);
        m_rect.setFillColor(over ? Theme::BTN_HOVER : Theme::BTN_NORMAL);
        return over;
    }

    // Retorna true se o clique ocorreu dentro do botão
    bool handleMousePress(sf::Vector2f mouse) {
        if (m_rect.getGlobalBounds().contains(mouse)) {
            m_rect.setFillColor(Theme::BTN_PRESS);
            // Publica evento com o nome do botão como payload
            EventBus::instance().publish("button_clicked", m_label);
            return true;
        }
        return false;
    }

    void handleMouseRelease(sf::Vector2f mouse) {
        bool over = m_rect.getGlobalBounds().contains(mouse);
        m_rect.setFillColor(over ? Theme::BTN_HOVER : Theme::BTN_NORMAL);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(m_rect);
        window.draw(m_text);
    }

private:
    std::string        m_label;
    sf::RectangleShape m_rect;
    sf::Text           m_text;   // inicializado na member init list
};

// ── Sidebar ──────────────────────────────────────────────────
class Sidebar {
public:
    static constexpr float WIDTH = 180.f;

    // Constructor
    Sidebar(float windowHeight, const sf::Font& font): m_title(font) // SFML 3: fonte obrigatória no construtor
    {
        m_bg.setSize({WIDTH, windowHeight});
        m_bg.setFillColor(Theme::SIDEBAR_BG);

        // Título
        m_title.setString("ACTIONS");
        m_title.setCharacterSize(12);
        m_title.setFillColor(Theme::ACCENT);
        m_title.setStyle(sf::Text::Bold);
        m_title.setPosition({16.f, 16.f});

        // Cria botões enumerados
        const std::vector<std::string> labels = {
            "1. Spawn Entity",
            "2. Clear Scene",
            "3. Toggle Debug",
            "4. Run System",
            "5. Load Asset",
            "6. Save State",
        };

        float yStart = 50.f;
        float btnH   = 38.f;
        float gap    = 10.f;
        float margin = 12.f;

        for (const auto& lbl : labels) {
            m_buttons.emplace_back(
                lbl,
                sf::Vector2f{margin, yStart},
                sf::Vector2f{WIDTH - margin * 2.f, btnH},
                font
            );
            
            yStart += btnH + gap;
        }
    }

    void handleMouseMove(sf::Vector2f mouse) {
        for (auto& btn : m_buttons) btn.handleMouseMove(mouse);
    }

    void handleMousePress(sf::Vector2f mouse) {
        for (auto& btn : m_buttons) btn.handleMousePress(mouse);
    }

    void handleMouseRelease(sf::Vector2f mouse) {
        for (auto& btn : m_buttons) btn.handleMouseRelease(mouse);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(m_bg);
        window.draw(m_title);
        for (const auto& btn : m_buttons) btn.draw(window);
    }

private:
    sf::RectangleShape         m_bg;
    sf::Text                   m_title;
    std::vector<SidebarButton> m_buttons;
};

// ── ConsoleBar ───────────────────────────────────────────────
class ConsoleBar {
public:
    static constexpr float HEIGHT = 150.f;

    ConsoleBar(float windowWidth, float windowHeight, const sf::Font& font)
        : m_font(font)
        , m_title(font) // SFML 3: fonte obrigatória no construtor
    {
        float y = windowHeight - HEIGHT;

        m_bg.setPosition({0.f, y});
        m_bg.setSize({windowWidth, HEIGHT});
        m_bg.setFillColor(Theme::CONSOLE_BG);

        m_border.setPosition({0.f, y});
        m_border.setSize({windowWidth, 1.f});
        m_border.setFillColor(Theme::ACCENT);

        m_title.setString("CONSOLE");
        m_title.setCharacterSize(11);
        m_title.setFillColor(Theme::ACCENT);
        m_title.setStyle(sf::Text::Bold);
        m_title.setPosition({Sidebar::WIDTH + 12.f, y + 8.f});

        m_yOrigin    = y;
        m_lineStart  = y + 28.f;
        m_xStart     = Sidebar::WIDTH + 12.f;
        m_windowW    = windowWidth;
    }

    // Recebe snapshot das linhas do SharedState e as renderiza
    void draw(sf::RenderWindow& window, const std::vector<std::string>& lines) const {
        window.draw(m_bg);
        window.draw(m_border);
        window.draw(m_title);

        float lineH = 18.f;
        // Mostra apenas as últimas N linhas que cabem no painel
        int maxLines = static_cast<int>((HEIGHT - 30.f) / lineH);
        int start    = std::max(0, static_cast<int>(lines.size()) - maxLines);

        sf::Text line(m_font);  // SFML 3: fonte obrigatória no construtor
        line.setCharacterSize(13);

        for (int i = start; i < static_cast<int>(lines.size()); ++i) {
            float y = m_lineStart + (i - start) * lineH;
            // Alterna cor para legibilidade
            line.setFillColor((i % 2 == 0) ? Theme::TEXT_PRIMARY : Theme::TEXT_DIM);
            line.setString(lines[i]);
            line.setPosition({m_xStart, y});
            window.draw(line);
        }
    }

private:
    sf::RectangleShape m_bg;
    sf::RectangleShape m_border;
    sf::Text           m_title;
    const sf::Font&    m_font;
    float m_yOrigin, m_lineStart, m_xStart, m_windowW;
};