#include "lru_k_cache.h"
#include "config.h"

LRUKCache::LRUKCache(size_t num_frames, size_t k) : cache_size_(num_frames), k_(k) {}

LRUKNode::LRUKNode(frame_id_t frame_id) : fid_(frame_id) {}

auto LRUKCache::Evict() -> std::optional<frame_id_t> {
    std::lock_guard<std::mutex> lock(latch_);
    logger.info("Attempting to evict a frame");

    if (node_store_.empty()) {
        logger.error("Cache is empty");
        return std::nullopt;
    }

    // 首先尝试驱逐访问次数少于k次的页面（按照最早访问时间）
    size_t earliest_timestamp = current_timestamp_ + 1;
    frame_id_t victim_id = -1;

    for (const auto &[id, node] : node_store_) {
        if (!node.GetIsEvictable()) continue;
        
        if (node.GetHistorySize() < static_cast<int>(k_)) {
            if (node.GetBackHistory() < earliest_timestamp) {
                earliest_timestamp = node.GetBackHistory();
                victim_id = id;
            }
        }
    }

    if (victim_id != -1) {
        logger.info("Evicting frame with insufficient history: " + std::to_string(victim_id));
        Remove(victim_id);
        return victim_id;
    }

    // 如果没有访问次数少于k次的页面，选择k距离最大的页面
    size_t max_k_distance = 0;
    victim_id = -1;

    for (const auto &[id, node] : node_store_) {
        if (!node.GetIsEvictable()) continue;
        
        if (node.GetHistorySize() >= static_cast<int>(k_)) {
            size_t k_distance = current_timestamp_ - node.GetFrontHistory();
            if (k_distance > max_k_distance) {
                max_k_distance = k_distance;
                victim_id = id;
            }
        }
    }

    if (victim_id != -1) {
        logger.info("Evicting frame with largest k-distance: " + std::to_string(victim_id));
        Remove(victim_id);
        return victim_id;
    }

    logger.error("No evictable frames found");
    return std::nullopt;
}

void LRUKCache::RecordAccess(frame_id_t frame_id) {
    std::lock_guard<std::mutex> lock(latch_);
    
    logger.info("Recording access for frame: " + std::to_string(frame_id));
    current_timestamp_++;

    // 检查frame_id是否有效
    if (static_cast<size_t>(frame_id) >= cache_size_) {
        logger.error("Invalid frame_id: " + std::to_string(frame_id));
        throw std::runtime_error("Invalid frame_id");
    }

    // 如果是新的frame
    if (node_store_.find(frame_id) == node_store_.end()) {
        LRUKNode n{frame_id};
        n.AddTime(current_timestamp_);
        node_store_[frame_id] = n;
        curr_size_++;
        logger.info("Created new node for frame: " + std::to_string(frame_id));
    } else {
        // 更新已存在的frame
        node_store_[frame_id].AddTime(current_timestamp_);
        logger.info("Updated existing node for frame: " + std::to_string(frame_id));
    }
}

void LRUKCache::SetEvictable(frame_id_t frame_id, bool set_evictable) {
    std::lock_guard<std::mutex> lock(latch_);
  
    if (static_cast<size_t>(frame_id) > cache_size_) {
      //抛出异常
      throw "没有该frame_id";
      // 解锁
      // latch_.unlock();
      return;
    }
  
    if (!node_store_[frame_id].GetIsEvictable() && set_evictable) {
      node_store_[frame_id].SetIsEvictable(set_evictable);
      curr_size_++;
    } else if (node_store_[frame_id].GetIsEvictable() && !set_evictable) {
      node_store_[frame_id].SetIsEvictable(set_evictable);
      curr_size_--;
    }
    // 解锁
    // latch_.unlock();
}

void LRUKCache::Remove(frame_id_t frame_id) {
    if (node_store_.find(frame_id) == node_store_.end()) {
        return;
    }

    if (!node_store_[frame_id].GetIsEvictable()) {
        logger.error("Attempting to remove non-evictable frame: " + std::to_string(frame_id));
        return;
    }

    node_store_.erase(frame_id);
    curr_size_--;
    logger.info("Removed frame: " + std::to_string(frame_id));
}

auto LRUKCache::Size() -> size_t { return curr_size_; }