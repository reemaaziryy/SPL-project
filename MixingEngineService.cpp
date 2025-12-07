#include "MixingEngineService.h"
#include <iostream>
#include <memory>
#include <cmath>   


/**
 * TODO: Implement MixingEngineService constructor
 */
MixingEngineService::MixingEngineService()
    : active_deck(0), auto_sync(false), bpm_tolerance(0)
{
    decks[0] = nullptr;
    decks[1] = nullptr;
    std::cout << "[MixingEngineService] Initialized with 2 empty decks." << std::endl;
}

/**
 * TODO: Implement MixingEngineService destructor
 */
MixingEngineService::~MixingEngineService() {
    std::cout << "[MixingEngineService] Cleaning up decks..." << std::endl;
    for (size_t i = 0; i < 2; ++i) {
        if (decks[i]) {
            delete decks[i];
            decks[i] = nullptr;
        }
    }
}


/**
 * TODO: Implement loadTrackToDeck method
 * @param track: Reference to the track to be loaded
 * @return: Index of the deck where track was loaded, or -1 on failure
 */
int MixingEngineService::loadTrackToDeck(const AudioTrack& track) {
    std::cout << "\n=== Loading Track to Deck ===\n";

    // (b) clone פולימורפי עטוף ב-PointerWrapper
    PointerWrapper<AudioTrack> cloned = track.clone();
    if (!cloned) {
        std::cerr << "[ERROR] Track: \"" << track.get_title()
                  << "\" failed to clone" << std::endl;
        return -1;  // לא משנים מצב דקים
    }

    // לבדוק אם זה הטראק הראשון – שני הדקים ריקים
    bool first_track = (decks[0] == nullptr && decks[1] == nullptr);

    // (d) דק מטרה – אם זה הראשון → 0, אחרת הדק הלא-אקטיבי
    size_t target = first_track ? 0 : (1 - active_deck);
    std::cout << "[Deck Switch] Target deck: " << target << std::endl;

    // (f) אם דק המטרה תפוס – לפרוק אותו
    if (decks[target]) {
        std::cout << "[Unload] Unloading deck " << target
                  << " (" << decks[target]->get_title() << ")\n";
        delete decks[target];
        decks[target] = nullptr;
    }

    // (g) הכנת הטראק – load + analyze_beatgrid
    cloned->load();
    cloned->analyze_beatgrid();

    // (h) ניהול BPM – רק אם יש דק אקטיבי וטראק קודם
    if (!first_track && decks[active_deck] && auto_sync) {
        if (!can_mix_tracks(cloned)) {
            sync_bpm(cloned);
        }
    }

    // (i) שחרור המצביע מה-Wrapper והעלאתו לדק המטרה
    AudioTrack* raw = cloned.release();
    decks[target] = raw;
    std::cout << "[Load Complete] '" << decks[target]->get_title()
              << "' is now loaded on deck " << target << std::endl;

    // (j) Instant transition – לפרוק את הדק שהיה אקטיבי לפני זה
    if (!first_track && decks[active_deck]) {
        std::cout << "[Unload] Unloading previous deck " << active_deck
                  << " (" << decks[active_deck]->get_title() << ")\n";
        delete decks[active_deck];
        decks[active_deck] = nullptr;
    }

    // (k) לעדכן דק אקטיבי
    active_deck = target;
    std::cout << "[Active Deck] Switched to deck " << active_deck << std::endl;

    // (m) מחזירים את אינדקס הדק
    return static_cast<int>(target);
}

/**
 * @brief Display current deck status
 */
void MixingEngineService::displayDeckStatus() const {
    std::cout << "\n=== Deck Status ===\n";
    for (size_t i = 0; i < 2; ++i) {
        if (decks[i])
            std::cout << "Deck " << i << ": " << decks[i]->get_title() << "\n";
        else
            std::cout << "Deck " << i << ": [EMPTY]\n";
    }
    std::cout << "Active Deck: " << active_deck << "\n";
    std::cout << "===================\n";
}

/**
 * TODO: Implement can_mix_tracks method
 * 
 * Check if two tracks can be mixed based on BPM difference.
 * 
 * @param track: Track to check for mixing compatibility
 * @return: true if BPM difference <= tolerance, false otherwise
 */
bool MixingEngineService::can_mix_tracks(const PointerWrapper<AudioTrack>& track) const {
     // אין דק אקטיבי → אי אפשר למקסס
    if (!decks[active_deck]) {
        return false;
    }
    // אין טראק חדש → אי אפשר למקסס
    if (!track) {
        return false;
    }

    int active_bpm = decks[active_deck]->get_bpm();
    int new_bpm    = track->get_bpm();

    int diff = std::abs(active_bpm - new_bpm);
    return diff <= bpm_tolerance;
}

/**
 * TODO: Implement sync_bpm method
 * @param track: Track to synchronize with active deck
 */
void MixingEngineService::sync_bpm(const PointerWrapper<AudioTrack>& track) const {
    if (!decks[active_deck] || !track) {
        return; // אין מה לסנכרן
    }

    int active_bpm = decks[active_deck]->get_bpm();
    int new_bpm    = track->get_bpm();
    int original   = new_bpm;

    int avg = (active_bpm + new_bpm) / 2;

    std::cout << "[Sync BPM] Syncing BPM from " << original
              << " to " << avg << std::endl;

    track->set_bpm(avg);
}
