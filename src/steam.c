#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

#define MAX_PATH_LENGTH 260
#define MAX_STEAM_PATHS 100

char** find_steam_folders(int* num_paths) {
    char** steam_paths = (char**) malloc(MAX_STEAM_PATHS * sizeof(char*));
    int path_count = 0;
    *num_paths = 0;
    
    // Search drives for Steam installations
    for (char drive = 'A'; drive <= 'Z'; drive++) {
        char base_path[4] = {drive, ':', '\\', '\0'};
        
        if (GetDriveTypeA(base_path) == DRIVE_NO_ROOT_DIR) continue;
        
        // Check root Steam folder
        char steam_path[MAX_PATH_LENGTH];
        snprintf(steam_path, MAX_PATH_LENGTH, "%s%s", base_path, "Steam");
        char exe_path[MAX_PATH_LENGTH];
        snprintf(exe_path, MAX_PATH_LENGTH, "%s\\steam.exe", steam_path);
        
        WIN32_FIND_DATAA find_data;
        HANDLE handle = FindFirstFileA(exe_path, &find_data);
        if (handle != INVALID_HANDLE_VALUE) {
            steam_paths[path_count] = (char*) malloc(MAX_PATH_LENGTH);
            strncpy(steam_paths[path_count], steam_path, MAX_PATH_LENGTH);
            path_count++;
            FindClose(handle);
            continue;
        }
        FindClose(handle);
        
        // Check one level deep
        char search_path[MAX_PATH_LENGTH];
        snprintf(search_path, MAX_PATH_LENGTH, "%s*", base_path);
        
        handle = FindFirstFileA(search_path, &find_data);
        if (handle == INVALID_HANDLE_VALUE) continue;
        
        do {
            if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
            if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) continue;
            
            snprintf(steam_path, MAX_PATH_LENGTH, "%s%s\\Steam", base_path, find_data.cFileName);
            snprintf(exe_path, MAX_PATH_LENGTH, "%s\\steam.exe", steam_path);
            
            HANDLE exe_handle = FindFirstFileA(exe_path, &find_data);
            if (exe_handle != INVALID_HANDLE_VALUE) {
                steam_paths[path_count] = (char*) malloc(MAX_PATH_LENGTH);
                strncpy(steam_paths[path_count], steam_path, MAX_PATH_LENGTH);
                path_count++;
                FindClose(exe_handle);
            }
            
        } while (FindNextFileA(handle, &find_data) && path_count < MAX_STEAM_PATHS);
        
        FindClose(handle);
    }
    
    // Get additional paths from libraryfolders.vdf
    for (int i = 0; i < path_count; i++) {
        char vdf_path[MAX_PATH_LENGTH];
        snprintf(vdf_path, MAX_PATH_LENGTH, "%s\\steamapps\\libraryfolders.vdf", steam_paths[i]);
        
        FILE* file = fopen(vdf_path, "r");
        if (!file) continue;
        
        char line[MAX_PATH_LENGTH];
        while (fgets(line, MAX_PATH_LENGTH, file)) {
            if (strstr(line, "\"path\"") == NULL) continue;
            
            char* first_quote = strchr(line, '\"');
            if (!first_quote) continue;
            first_quote = strchr(first_quote + 1, '\"');
            if (!first_quote) continue;
            first_quote++;
            
            char* last_quote = strchr(first_quote, '\"');
            if (!last_quote) continue;
            
            int len = last_quote - first_quote;
            char lib_path[MAX_PATH_LENGTH];
            strncpy(lib_path, first_quote, len);
            lib_path[len] = '\0';
            
            // Normalize path by replacing
            char* pos = lib_path;
            while ((pos = strstr(pos, "\\\\"))) {
                memmove(pos + 1, pos + 2, strlen(pos + 2) + 1);
            }
            
            WIN32_FIND_DATAA path_data;
            HANDLE path_handle = FindFirstFileA(lib_path, &path_data);
            if (path_handle != INVALID_HANDLE_VALUE) {
                steam_paths[path_count] = (char*) malloc(MAX_PATH_LENGTH);
                strncpy(steam_paths[path_count], lib_path, MAX_PATH_LENGTH);
                path_count++;
                FindClose(path_handle);
            }
        }
        fclose(file);
    }
    
    *num_paths = path_count;
    return steam_paths;
}

int main() {
    int num_paths;
    char** steam_folders = find_steam_folders(&num_paths);
    
    printf("Found Steam folders:\n");
    for (int i = 0; i < num_paths; i++) {
        printf("- %s\n", steam_folders[i]);
        free(steam_folders[i]);
    }
    free(steam_folders);
    
    printf("\nPress any key to exit...");
    getchar();
    return 0;
}