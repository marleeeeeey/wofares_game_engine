import glob
import os
import sys


def pack_binaries(root_folder, build_folder):
    # Define paths based on the build type
    src_path = f"{root_folder}/{build_folder}/src/"

    # Search *.html file in the src_path and rename it to index.html.
    html_files = glob.glob(f"{src_path}/*.html")
    if len(html_files) == 1:
        for html_file in html_files:
            os.rename(html_file, f"{src_path}/index.html")
    else:
        raise ValueError(
            f"There should be exactly one html file in the web build. Found: {len(html_files)}: {html_files}",
        )


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: pack_binaries.py <root_folder> <build_folder>")
        sys.exit(1)

    root_folder = sys.argv[1]
    build_folder = sys.argv[2]  # build/Debug_web
    try:
        pack_binaries(root_folder, build_folder)
    except ValueError as e:
        print(e)
        sys.exit(1)
