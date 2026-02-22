#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>

// ============================================================
//  EventBus  –  pub/sub desacoplado, thread-safe para leitura
//  Subscribers se registram com um string "topic".
//  Publishers disparam eventos com um payload (string).
// ============================================================

class EventBus {
public:
    using Handler = std::function<void(const std::string& payload)>;

    static EventBus& instance() {
        static EventBus bus;
        return bus;
    }

    void subscribe(const std::string& topic, Handler handler) {
        std::lock_guard lock(m_mutex);
        m_handlers[topic].push_back(std::move(handler));
    }

    void publish(const std::string& topic, const std::string& payload = "") {
        std::lock_guard lock(m_mutex);
        auto it = m_handlers.find(topic);
        if (it != m_handlers.end()) {
            for (auto& h : it->second)
                h(payload);
        }
    }

private:
    EventBus() = default;
    std::mutex m_mutex;
    std::unordered_map<std::string, std::vector<Handler>> m_handlers;
};