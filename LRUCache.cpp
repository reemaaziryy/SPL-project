#include "LRUCache.h"
#include <iostream>
#include <limits>
#include "CacheSlot.h"

LRUCache::LRUCache(size_t capacity)
    : slots(capacity), max_size(capacity), access_counter(0) {}

bool LRUCache::contains(const std::string& track_id) const {
    return findSlot(track_id) != max_size;
}

AudioTrack* LRUCache::get(const std::string& track_id) {
    size_t idx = findSlot(track_id);
    if (idx == max_size) return nullptr;
    return slots[idx].access(++access_counter);
}

/**
 * TODO: Implement the put() method for LRUCache
 */
bool LRUCache::put(PointerWrapper<AudioTrack> track) {
    // (a) אם קיבלנו wrapper ריק – לא עושים כלום
    if (!track) {
        return false; // אין החדרה, אין עקירה
    }

    // מזהה של הטראק הוא הכותרת (title)
    const std::string track_id = track->get_title();

    // (b) לבדוק אם כבר יש בקאש טראק עם אותו title
    size_t existing = findSlot(track_id);
    if (existing != max_size) {
        // טראק כבר קיים – רק לעדכן זמן גישה כדי שיהיה MRU
        slots[existing].access(++access_counter);
        return false; // לא הייתה עקירה
    }

    bool eviction_occurred = false;

    // (c) אם הקאש מלא – לעקור את ה־LRU קודם
    if (isFull()) {
        eviction_occurred = evictLRU();
    }

    // (d) למצוא סלוט ריק
    size_t empty = findEmptySlot();
    if (empty == max_size) {
        // תאורטית לא אמור לקרות, אבל אם כן – נוותר
        std::cerr << "[LRUCache] ERROR: No empty slot available for insertion.\n";
        return eviction_occurred;
    }

    // (e) לשמור את הטראק החדש בסלוט הריק בתור MRU
    ++access_counter;

    // נניח של-CacheSlot יש בנאי שמקבל PointerWrapper ו־access_time
    // אם הבנאי שונה אצלך, תשנה כאן בהתאם.
    slots[empty].store(std::move(track), access_counter);

    // (f) להחזיר אם הייתה עקירה
    return eviction_occurred;
}

bool LRUCache::evictLRU() {
    size_t lru = findLRUSlot();
    if (lru == max_size || !slots[lru].isOccupied()) return false;
    slots[lru].clear();
    return true;
}

size_t LRUCache::size() const {
    size_t count = 0;
    for (const auto& slot : slots) if (slot.isOccupied()) ++count;
    return count;
}

void LRUCache::clear() {
    for (auto& slot : slots) {
        slot.clear();
    }
}

void LRUCache::displayStatus() const {
    std::cout << "[LRUCache] Status: " << size() << "/" << max_size << " slots used\n";
    for (size_t i = 0; i < max_size; ++i) {
        if(slots[i].isOccupied()){
            std::cout << "  Slot " << i << ": " << slots[i].getTrack()->get_title()
                      << " (last access: " << slots[i].getLastAccessTime() << ")\n";
        } else {
            std::cout << "  Slot " << i << ": [EMPTY]\n";
        }
    }
}

size_t LRUCache::findSlot(const std::string& track_id) const {
    for (size_t i = 0; i < max_size; ++i) {
        if (slots[i].isOccupied() && slots[i].getTrack()->get_title() == track_id) return i;
    }
    return max_size;

}

/**
 * TODO: Implement the findLRUSlot() method for LRUCache
 */
size_t LRUCache::findLRUSlot() const {
    size_t lru_index = max_size;  // "לא נמצא"
    uint64_t min_access = std::numeric_limits<uint64_t>::max();
    bool found_any = false;

    for (size_t i = 0; i < max_size; ++i) {
        if (slots[i].isOccupied()) {
            uint64_t acc = slots[i].getLastAccessTime();
            if (!found_any || acc < min_access) {
                found_any = true;
                min_access = acc;
                lru_index = i;
            }
        }
    }

    // אם הקאש ריק – מחזירים max_size לפי ההנחיות
    return found_any ? lru_index : max_size;
}

size_t LRUCache::findEmptySlot() const {
    for (size_t i = 0; i < max_size; ++i) {
        if (!slots[i].isOccupied()) return i;
    }
    return max_size;
}

void LRUCache::set_capacity(size_t capacity){
    if (max_size == capacity)
        return;
    //udpate max size
    max_size = capacity;
    //update the slots vector
    slots.resize(capacity);
}
