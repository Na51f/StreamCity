#include <windows.h>
#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <filesystem>
#include <iostream>

std::vector<std::string> find_steam_folders() {
    std::set<std::string> steam_paths;
    
    // Search drives for Steam installations
    for (char drive = 'A'; drive <= 'Z'; drive++) {
        std::string base_path = std::string(1, drive) + ":\\";
        // Remove the DRIVE_FIXED check to search all drive types
        if (GetDriveTypeA(base_path.c_str()) == DRIVE_NO_ROOT_DIR) continue;
        
        try {
            // Check root Steam folder
            std::string steam_path = base_path + "Steam";
            if (std::filesystem::exists(steam_path + "\\steam.exe")) {
                steam_paths.insert(steam_path);
                continue;
            }

            // Check one level deep
            for (const auto& entry : std::filesystem::directory_iterator(base_path)) {
                if (!entry.is_directory()) continue;
                steam_path = entry.path().string() + "\\Steam";
                if (std::filesystem::exists(steam_path + "\\steam.exe")) {
                    steam_paths.insert(steam_path);
                }
            }
        } catch (const std::filesystem::filesystem_error&) {
            continue;
        }
    }
    // Get additional paths from libraryfolders.vdf
    for (const auto& path : steam_paths) {
        std::string vdf_path = path + "\\steamapps\\libraryfolders.vdf";
        if (!std::filesystem::exists(vdf_path)) continue;
        
        std::ifstream file(vdf_path);
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("\"path\"") == std::string::npos) continue;
            
            size_t first = line.find('\"', line.find('\"') + 1) + 1;
            size_t last = line.find('\"', first);
            std::string lib_path = line.substr(first, last - first);
            
            // Normalize path
            size_t pos = 0;
            while ((pos = lib_path.find("\\\\", pos)) != std::string::npos) {
                lib_path.replace(pos, 2, "\\");
                pos++;
            }
            
            if (std::filesystem::exists(lib_path)) {
                steam_paths.insert(lib_path);
            }
        }
    }
    
    return std::vector<std::string>(steam_paths.begin(), steam_paths.end());
}

int main() {
    // Test the function
    std::vector<std::string> steam_folders = find_steam_folders();
    std::cout << "Found Steam folders:\n";
    for (const auto& folder : steam_folders) {
        std::cout << "- " << folder << "\n";
    }
    std::cout << "\nPress Enter to exit...";
    std::cin.get();
    return 0;
}