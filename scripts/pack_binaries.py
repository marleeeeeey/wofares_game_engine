import subprocess
import sys
from datetime import datetime


def pack_binaries(path_to_7z, root_folder, build_folder, achive_name, web_or_desktop):
    # Define paths based on the build type
    src_path = f"{root_folder}/{build_folder}/src/*"

    exclude_paths = []

    # Define the paths to exclude
    if web_or_desktop == "desktop":
        exclude_paths += [
            f"{root_folder}/{build_folder}/src/CMakeFiles",
            f"{root_folder}/{build_folder}/src/cmake_install.cmake",
            f"{root_folder}/{build_folder}/src/logs",
            f"*.lib",
            f"*.pdb",
            f"*.tiled-session",
            f"*.aseprite",
            f"*.tiled-project",
        ]
    elif web_or_desktop == "web":
        exclude_paths += [
            f"{root_folder}/{build_folder}/src/assets",
            f"{root_folder}/{build_folder}/src/CMakeFiles",
            f"{root_folder}/{build_folder}/src/cmake_install.cmake",
            f"{root_folder}/{build_folder}/src/config.json",
        ]

    # Get current date-time as a string in the format YYYY-MM-DD_HH-MM-SS
    current_date_time = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

    # Form the 7z command for creating a zip archive
    command = [
        f"{path_to_7z}",
        "a",
        "-tzip",
        f"{root_folder}/achives/{current_date_time}_{achive_name}_{web_or_desktop}.zip",
        src_path,
    ]
    for exclude in exclude_paths:
        command.append(f"-xr!{exclude}")

    # Execute the command
    subprocess.run(command)


if __name__ == "__main__":
    if len(sys.argv) != 6:
        print("Usage: pack_binaries.py <path_to_7z> <root_folder> <build_folder> <achive_name> <web/desktop>")
        sys.exit(1)

    path_to_7z = sys.argv[1]
    root_folder = sys.argv[2]
    build_folder = sys.argv[3]  # build/Debug_web
    achive_name = sys.argv[4]
    web_or_desktop = sys.argv[5]
    try:
        pack_binaries(path_to_7z, root_folder, build_folder, achive_name, web_or_desktop)
    except ValueError as e:
        print(e)
        sys.exit(1)
