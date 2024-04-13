import subprocess
import sys
from datetime import datetime


def pack_binaries(path_to_7z, root_folder, build_type, achive_name):
    # Define paths based on the build type
    src_path = f"{root_folder}/build/{build_type}/src/*"

    # Define the paths to exclude
    exclude_paths = [
        f"{root_folder}/build/{build_type}/src/CMakeFiles",
        f"{root_folder}/build/{build_type}/src/cmake_install.cmake",
        f"{root_folder}/build/{build_type}/src/logs",
        f"*.lib",
        f"*.pdb",
        f"*.aseprite",
        f"*.tiled-project",
        f"*.tiled-session",
        # Add other paths to exclude here
    ]

    # Get current date-time as a string in the format YYYY-MM-DD_HH-MM-SS
    current_date_time = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")

    # Form the 7z command for creating a zip archive
    command = [f"{path_to_7z}", "a", "-tzip", f"{root_folder}/achives/{current_date_time}_{achive_name}.zip", src_path]
    for exclude in exclude_paths:
        command.append(f"-xr!{exclude}")

    # Execute the command
    subprocess.run(command)


if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("Usage: pack_binaries.py <path_to_7z> <root_folder> <debug/release> <achive_name>")
        sys.exit(1)

    path_to_7z = sys.argv[1]
    root_folder = sys.argv[2]
    build_type = sys.argv[3]
    achive_name = sys.argv[4]
    try:
        pack_binaries(path_to_7z, root_folder, build_type, achive_name)
    except ValueError as e:
        print(e)
        sys.exit(1)
