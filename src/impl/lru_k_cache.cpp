#include "lru_k_cache.h"
#include "config.h"

LRUKCache::LRUKCache(size_t num_frames, size_t k) : cache_size_(num_frames), k_(k) {}

LRUKNode::LRUKNode(frame_id_t frame_id) : fid_(frame_id) {}

auto LRUKCache::Evict() -> std::optional<frame_id_t> {
    // 加锁
    latch_.lock();
  
    //更新距离k
    for (auto &[k, v] : node_store_) {
      if (static_cast<size_t>(v.GetHistorySize()) >= k_) {
        node_store_[k].Setk(current_timestamp_ - v.GetBackHistory());
      }
    }
  
    // 解锁
    latch_.unlock();
  
    // 加锁
    latch_.lock();
  
    //假设有+inf
    int min = 0x3f3f3f3f;
    frame_id_t flag_1 = -1;
    for (auto &[k, v] : node_store_) {
      if (v.Getk() == 0x3f3f3f3f && v.GetIsEvictable() && static_cast<int>(v.GetBackHistory()) < min) {
        min = v.GetBackHistory();
        flag_1 = k;
      }
    }
  
    if (flag_1 != -1) {
      Remove(flag_1);

      // 解锁
      latch_.unlock();

      return flag_1;
    }
  
    //如果没有+inf
    int max = 0;
    frame_id_t flag_2 = -1;
    for (auto &[k, v] : node_store_) {
      if (v.GetIsEvictable()) {
        if (static_cast<int>(v.Getk()) >= max) {
          max = v.Getk();
          flag_2 = k;
        }
      }
    }
    if (flag_2 != -1) {
      Remove(flag_2);
      // 解锁
      latch_.unlock();
      return flag_2;
    }
    // 解锁
    latch_.unlock();
    //没有可驱逐的帧
    return std::nullopt;
}

void LRUKCache::RecordAccess(frame_id_t frame_id) {
    current_timestamp_++;
    if (node_store_.find(frame_id) == node_store_.end()) {
        // 如果是新的客户
        LRUKNode n{frame_id};
        n.AddTime(current_timestamp_);
        node_store_[frame_id] = n;
    } else if (static_cast<size_t>(frame_id) > cache_size_) {
        //抛出异常
        throw "没有该client_fd";
        // 解锁
        latch_.unlock();
        return;
    } else {
        node_store_[frame_id].AddTime(current_timestamp_);
        if (node_store_[frame_id].GetHistorySize() >= static_cast<int>(k_)) {
        node_store_[frame_id].Settime(current_timestamp_);
        }
    }
}

void LRUKCache::SetEvictable(frame_id_t frame_id, bool set_evictable) {
    // 加锁
    latch_.lock();
  
    if (static_cast<size_t>(frame_id) > cache_size_) {
      //抛出异常
      throw "没有该frame_id";
      // 解锁
      latch_.unlock();
      return;
    }
  
    if (!node_store_[frame_id].GetIsEvictable() && set_evictable) {
      node_store_[frame_id].SetIsEvictable(set_evictable);
      curr_size_++;
      // std::cout << "设置: " << frame_id << " 可驱逐" << std::endl;
    } else if (node_store_[frame_id].GetIsEvictable() && !set_evictable) {
      node_store_[frame_id].SetIsEvictable(set_evictable);
      curr_size_--;
      // std::cout << "设置: " << frame_id << " 不可驱逐" << std::endl;
    }
    // 解锁
    latch_.unlock();
}

void LRUKCache::Remove(frame_id_t frame_id) {
    // latch_.lock();
    if (node_store_.find(frame_id) == node_store_.end()) {
        // std::cout << "????" << std::endl;
        return;
    }

    if (!node_store_[frame_id].GetIsEvictable()) {
        throw "不可驱逐该client_fd";
        return;
    }
}

auto LRUKCache::Size() -> size_t { return curr_size_; }