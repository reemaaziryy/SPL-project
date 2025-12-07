#include "DJControllerService.h"
#include "MP3Track.h"
#include "WAVTrack.h"
#include <iostream>
#include <memory>

DJControllerService::DJControllerService(size_t cache_size)
    : cache(cache_size) {}
/**
 * TODO: Implement loadTrackToCache method
 */
int DJControllerService::loadTrackToCache(AudioTrack& track) {
    const std::string title = track.get_title();

    // (a) לבדוק אם הטראק כבר בקאש לפי title
    if (cache.contains(title)) {
        // HIT – רק לרענן את זמן הגישה דרך get ולחזור 1
        cache.get(title);  // זה גם מעדכן את ה־LRU
        return 1;
    }

    // (c) MISS – צריך להכניס לקאש
    // ליצור clone פולימורפי
    PointerWrapper<AudioTrack> cloned = track.clone();

    // אם ה-clone נכשל (wrapper ריק)
    if (!cloned) {
        std::cerr << "[ERROR] Track: \"" << title << "\" failed to clone\n";
        return 0; // נחשב כ-MISS בלי עקירה (לא הכנסנו כלום)
    }

    // (c) סימולציה של load + beat analysis על ה-clone
    cloned->load();
    cloned->analyze_beatgrid();

    // (c) לעטוף בקאש (ה־PointerWrapper כבר קיים – פשוט מעבירים ב-move ל-put)
    bool evicted = cache.put(std::move(cloned));

    // (e) קוד החזרה:
    //  0 – MISS בלי עקירה
    // -1 – MISS עם עקירה
    return evicted ? -1 : 0;
}

void DJControllerService::set_cache_size(size_t new_size) {
    cache.set_capacity(new_size);
}
//implemented
void DJControllerService::displayCacheStatus() const {
    std::cout << "\n=== Cache Status ===\n";
    cache.displayStatus();
    std::cout << "====================\n";
}

/**
 * TODO: Implement getTrackFromCache method
 */
AudioTrack* DJControllerService::getTrackFromCache(const std::string& track_title) {
    return cache.get(track_title);
}
