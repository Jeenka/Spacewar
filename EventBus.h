#pragma once

#include <functional>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <algorithm>

class EventBus
{
public:
    using HandlerId = std::size_t;

    template<typename Event>
    HandlerId subscribe(std::function<void(const Event&)> handler)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto &vec = handlers_[std::type_index(typeid(Event))];
        HandlerId id = nextId_++;
        vec.emplace_back(id, [h = std::move(handler)](const void* e) {
            h(*static_cast<const Event*>(e));
        });
        return id;
    }

    template<typename Event>
    void unsubscribe(HandlerId id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = handlers_.find(std::type_index(typeid(Event)));
        if (it == handlers_.end()) return;
        auto &vec = it->second;
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [id](auto &p) { return p.first == id; }), vec.end());
    }

    template<typename Event>
    void publish(const Event& e)
    {
        std::vector<std::pair<HandlerId, std::function<void(const void*)>>> copy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = handlers_.find(std::type_index(typeid(Event)));
            if (it == handlers_.end()) return;
            copy = it->second; // copy to avoid reentrancy issues
        }
        for (auto &p : copy) p.second(&e);
    }

private:
    std::mutex mutex_;
    std::atomic<HandlerId> nextId_{1};
    std::unordered_map<std::type_index, std::vector<std::pair<HandlerId, std::function<void(const void*)>>>> handlers_;
};

// For now, simple header-only global EventBus accessor.
inline EventBus& GlobalEventBus()
{
    static EventBus bus;
    return bus;
}