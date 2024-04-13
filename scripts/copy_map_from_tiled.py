import shutil
import os


def copy_files(source_dir, destination_dir, file_list):
    try:
        for filename in file_list:
            source_path = os.path.join(source_dir, filename)
            destination_path = os.path.join(destination_dir, filename)
            shutil.copyfile(source_path, destination_path)
            print(f"File {filename} successfully copied from {source_path} to {destination_path}")
    except Exception as e:
        print(f"Error copying files: {e}")


if __name__ == "__main__":
    source_dir = R"C:\Users\styulenev\OneDrive\gamedev\assets-projects\2024-03-06 1147 Automap with quarter tiles"
    destination_dir = R"c:\dev\LD55\assets\maps"
    file_list = ["map.json", "tileset.json", "tileset_blocks.png", "tileset_grass.png"]

    copy_files(source_dir, destination_dir, file_list)
