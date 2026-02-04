#pragma once

#include <vector>

template <typename T>
class ObjectPool
{
private:
    std::vector<T*> active;
    std::vector<T*> inactive;

public:
    ObjectPool(size_t size = 200) {
        inactive.reserve(size);
        active.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            inactive.emplace_back(new T());
        }
    }

    T* acquire() {
        if (inactive.empty()) return nullptr;
        T* obj = inactive.back();
        inactive.pop_back();
        active.push_back(obj);
        return obj;
    }

    void release(T* obj) {
        auto it = std::find(active.begin(), active.end(), obj);
        if (it != active.end()) {
            active.erase(it);
            inactive.push_back(obj);
        }
    }

    std::vector<T*>& getActiveObjects() { return active; }
};

