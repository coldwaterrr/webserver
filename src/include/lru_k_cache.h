#pragma once

#include <vector>
#include <list>
#include <optional>
#include <unordered_map>
#include <mutex>
#include "config.h"

class LRUKNode {
    private:
        /** 该页面最近访问的K个时间戳的历史记录。最早的时间戳存储在前面。 */
        // Remove maybe_unused if you start using them. Feel free to change the member variables as you want.

        std::list<size_t> history_;
        size_t k_{0x3f3f3f3f};
        frame_id_t fid_;
        size_t time_;
        bool is_evictable_{false};

    public:
        LRUKNode() = default;
        explicit LRUKNode(frame_id_t frame_id);

        auto Getk() -> size_t { return k_; }

        void Setk(size_t k) { k_ = k; }

        void Settime(size_t x) { time_ = x; }

        auto Gettime() -> size_t { return time_; }

        void AddTime(size_t time) {
        if (history_.size() >= k_) {
            history_.pop_front();
        }
        history_.push_back(time);
        }

        auto GetHistorySize() -> int { return history_.size(); }

        auto GetIsEvictable() -> bool { return is_evictable_; }

        void SetIsEvictable(bool is_evictable) { is_evictable_ = is_evictable; }

        auto GetFrontHistory() -> size_t { return history_.front(); }

        auto GetBackHistory() -> size_t { return history_.back(); }

        auto Getid() -> frame_id_t { return fid_; }
};

class LRUKCache {
    public:
    explicit LRUKCache(size_t num_frames, size_t k);
  

    ~LRUKCache() = default;
  
    auto Evict() -> std::optional<frame_id_t>;
  
    void RecordAccess(frame_id_t frame_id);
  
    void SetEvictable(frame_id_t frame_id, bool set_evictable);
  
    void Remove(frame_id_t frame_id);
  
    auto Size() -> size_t;
  
    // void Reset(frame_id_t frame_id);
  
   private:
    // TODO(student): implement me! You can replace these member variables as you like.
    // Remove maybe_unused if you start using them.
    std::unordered_map<frame_id_t, LRUKNode> node_store_;
    size_t current_timestamp_{0};
    size_t curr_size_{0};
    size_t cache_size_;
    size_t k_;
    std::mutex latch_;  //闩锁
};