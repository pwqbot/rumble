#include <concepts>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

template <template <typename, typename, typename> typename T, typename Key,
          typename Value, typename Hash = std::hash<Key>>
concept is_lru = requires(T<Key, Value, Hash> v, Key key, Value value) {
    { v.Put(key, value) } -> std::same_as<void>;
    { v.Get(key) } -> std::same_as<std::optional<Value>>;
    {v.Clear()};
    {v.Print()};
};

template <typename Key_, typename Value_, typename Hash_ = std::hash<Key_>>
struct LRU {
    std::size_t                        capacity_{5};
    std::list<std::pair<Key_, Value_>> list_;
    std::unordered_map<
        Key_, typename std::list<std::pair<Key_, Value_>>::iterator, Hash_>
        mp_;

    template <typename Key, typename Value>
    auto Put(Key &&key, Value &&value) -> void {
        auto find_it = mp_.find(key);
        if (find_it == mp_.end()) {
            std::cerr << 88888 << std::endl;
            list_.emplace_front(std::forward<Key>(key),
                                std::forward<Value>(value));
            mp_.emplace(key, list_.begin());
        } else {
            list_.erase(find_it->second);
            list_.emplace_front(std::forward<Key>(key),
                                std::forward<Value>(value));
            mp_[key] = list_.begin();
        }

        if (list_.size() > capacity_) {
            mp_.erase(list_.end()->second);
            list_.pop_back();
        }
    }

    auto Get(const Key_ &key) -> std::optional<Value_> {
        auto find_it = mp_.find(key);
        if (find_it == mp_.end()) {
            return {};
        } else {
            list_.emplace_front(std::move(*find_it->second));
            list_.erase(find_it->second);
            mp_[key] = list_.begin();
            return list_.begin()->second;
        }
    }

    auto Print() const noexcept -> void {
        std::cout << "print lru" << std::endl;
        for (auto &node : list_) {
            std::cout << node.first << " " << node.second << std::endl;
        }
    }

    auto Clear() noexcept -> void {
        mp_.clear();
        list_.clear();
    }
};

template <template <typename, typename, typename> typename LRU, typename Key_,
          typename Value_, typename Hash = std::hash<Key_>>
requires is_lru<LRU, Key_, Value_, Hash>
struct LRUPro {
    std::vector<LRU<Key_, Value_, Hash>> lrus_;
    std::size_t                          bits_;
    Hash                                 hash;
    LRUPro(size_t n) : lrus_(n) {}

    template <typename Key, typename Value>
    auto Put(Key &&key, Value &&value) -> void {
        auto v = hash(key) % lrus_.size();
        lrus_[v].Put(key, value);
    }
    auto Get(const Key_ &key) -> std::optional<Value_> {
        auto v = hash(key) % lrus_.size();
        lrus_[v].Get(key);
    }
    auto Clear() noexcept -> void {
        for (auto &lru : lrus_) {
            lru.clear();
        }
    }
    auto Print() const noexcept -> void {
        for (auto &lru : lrus_) {
            lru.Print();
        }
    }
};

int main() {

    LRU<int, int> l;
    /* l.Put(1, 1); */
    l.Print();
    /* l.Put(2, 2); */
    l.Print();
    std::cerr << l.Get(1).value() << std::endl;
    /* std::cerr << l.Get(2).value() << std::endl; */
    /* l.Put(1, 3); */
    std::cerr << l.Get(1).value() << std::endl;
    l.Print();
    for (int i = 0; i < 5; i++) {
        l.Put(i, i);
        l.Print();
    }
    LRUPro<LRU, int, int> lll{5};
}
