import os
import winreg
from typing import List
from pathlib import Path

def find_steam_folders() -> List[str]:
    """
    Searches the system for Steam installation and library folders.
    Returns a list of paths where Steam folders are found.
    """
    steam_paths = set()
    
    # Common Steam installation paths to check
    default_paths = [
        "C:\\Program Files (x86)\\Steam",
        "C:\\Program Files\\Steam",
    ]
    
    # Try to get Steam path from Windows Registry
    try:
        hkey = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Valve\\Steam")
        reg_path = winreg.QueryValueEx(hkey, "InstallPath")[0]
        if os.path.exists(reg_path):
            steam_paths.add(reg_path)
    except WindowsError:
        pass

    # Check default installation paths
    for path in default_paths:
        if os.path.exists(path):
            steam_paths.add(path)
    
    # Look for additional library folders in libraryfolders.vdf
    for steam_path in steam_paths.copy():
        library_file = os.path.join(steam_path, "steamapps", "libraryfolders.vdf")
        if os.path.exists(library_file):
            with open(library_file, 'r', encoding='utf-8') as f:
                content = f.read()
                # Simple parsing of VDF format to find paths
                for line in content.splitlines():
                    if '"path"' in line.lower():
                        # Extract path between quotes
                        path = line.split('"')[3].replace("\\\\", "\\")
                        if os.path.exists(path):
                            steam_paths.add(path)
    
    return list(steam_paths)

if __name__ == "__main__":
    # Test the function
    steam_folders = find_steam_folders()
    print("Found Steam folders:")
    for folder in steam_folders:
        print(f"- {folder}")