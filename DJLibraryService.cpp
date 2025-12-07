#include "DJLibraryService.h"
#include "SessionFileParser.h"
#include "MP3Track.h"
#include "WAVTrack.h"
#include <iostream>
#include <memory>
#include <filesystem>


DJLibraryService::DJLibraryService(const Playlist& playlist) 
    : playlist(playlist) {}
/**
 * @brief Load a playlist from track indices referencing the library
 * @param library_tracks Vector of track info from config
 */
DJLibraryService::~DJLibraryService() {
    // הספרייה מחזיקה בבעלות על כל ה-AudioTrack* בוקטור
    for (AudioTrack* t : library) {
        delete t;
    }
    library.clear();
}
void DJLibraryService::buildLibrary(const std::vector<SessionConfig::TrackInfo>& library_tracks) {
    //Todo: Implement buildLibrary method
    // קודם כל ננקה ספרייה קיימת (אם יש) כדי לא ליצור דליפות זיכרון
    for (AudioTrack* t : library) {
        delete t;
    }
    library.clear();

    std::cout << "[DJLibraryService] Building library with "
              << library_tracks.size() << " tracks...\n";

    // נעבור על כל TrackInfo שהגיע מה־SessionFileParser
    for (const auto& info : library_tracks) {
        AudioTrack* track = nullptr;

        if (info.type == "MP3") {
    track = new MP3Track(
        info.title,
        info.artists,
        info.duration_seconds,
        info.bpm,
        info.extra_param1,   // bitrate
        static_cast<bool>(info.extra_param2) // has_tags
    );
    }
    else if (info.type == "WAV") {
        track = new WAVTrack(
            info.title,
            info.artists,
            info.duration_seconds,
            info.bpm,
            info.extra_param1,   // sample_rate
            info.extra_param2    // bit_depth
        );
    }
    else {
        std::cerr << "[ERROR] Unknown track type: " << info.type << std::endl;
    }
        
    if (track) {
        library.push_back(track);
    }


    std::cout << "TODO: Implement DJLibraryService::buildLibrary method\n"<< library_tracks.size() << " tracks to be loaded into library.\n";
    }
}
/**
 * @brief Display the current state of the DJ library playlist
 * 
 */
void DJLibraryService::displayLibrary() const {
    std::cout << "=== DJ Library Playlist: "
              << playlist.get_name() << " ===" << std::endl;

    if (playlist.is_empty()) {
        std::cout << "[INFO] Playlist is empty.\n";
        return;
    }

    playlist.display();

    std::cout << "Total duration: " 
              << playlist.get_total_duration()
              << " seconds" << std::endl;
}

/**
 * @brief Get a reference to the current playlist
 * 
 * @return Playlist& 
 */
Playlist& DJLibraryService::getPlaylist() {
    // Your implementation here
    return playlist;
}

/**
 * TODO: Implement findTrack method
 * 
 * HINT: Leverage Playlist's find_track method
 */
AudioTrack* DJLibraryService::findTrack(const std::string& track_title) {
    // Your implementation here
    return playlist.find_track(track_title);
}

void DJLibraryService::loadPlaylistFromIndices(const std::string& playlist_name, 
                                               const std::vector<int>& track_indices) {
    // Your implementation here
    std::cout << "[INFO] Loading playlist: " << playlist_name << std::endl;

    // ליצור פלייליסט חדש בשם המבוקש
    playlist = Playlist(playlist_name);

    int added_count = 0;

    for (int index : track_indices) {
        // האינדקסים הם 1-based בקובץ הקונפיג
        if (index <= 0 || static_cast<size_t>(index) > library.size()) {
            std::cerr << "[WARNING] Invalid track index: " << index << std::endl;
            continue;
        }

        AudioTrack* source = library[index - 1];

        // clone פולימורפי
        PointerWrapper<AudioTrack> cloned = source->clone();
        if (!cloned) {
            std::cerr << "[ERROR] Track: \"" << source->get_title()
                      << "\" failed to clone. Skipping.\n";
            continue;
        }

        // סימולציית load + beat analysis על העותק
        cloned->load();
        cloned->analyze_beatgrid();

        // להעביר בעלות לפלייליסט – משחררים את המצביע מה-wrapper
        AudioTrack* raw = cloned.release();
        playlist.add_track(raw);
        ++added_count;

        std::cout << "Added '" << raw->get_title()
                  << "' to playlist '" << playlist_name << "'" << std::endl;
    }

    std::cout << "[INFO] Playlist loaded: " << playlist_name
              << " (" << added_count << " tracks)" << std::endl;
}
/**
 * TODO: Implement getTrackTitles method
 * @return Vector of track titles in the playlist
 */
std::vector<std::string> DJLibraryService::getTrackTitles() const {
    // Your implementation here
    std::vector<std::string> titles;
    std::vector<AudioTrack*> tracks = playlist.getTracks();
    titles.reserve(tracks.size());

    for (AudioTrack* t : tracks) {
        if (t) {
            titles.push_back(t->get_title());
        }
    }
    return titles;
}
