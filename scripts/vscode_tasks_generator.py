import json
from enum import Enum, auto
import os


class Platform(Enum):
    WINDOWS = auto()
    LINUX = auto()


class BuildType(Enum):
    DEBUG = auto()
    RELEASE = auto()


class StopOnFirstError(Enum):
    YES = auto()
    NO = auto()


class ExportCompileCommands(Enum):
    YES = auto()
    NO = auto()


##################################### SETTINGS #####################################


class Settings:
    def __init__(self):
        self.current_platform = Platform.WINDOWS
        self.current_build_type = BuildType.DEBUG
        self.compiler = "clang++"
        self.make_tool = "Ninja"
        self.toolchain_file = "vcpkg/scripts/buildsystems/vcpkg.cmake"
        self.stop_on_first_error = StopOnFirstError.YES
        self.export_compile_commands = ExportCompileCommands.YES
        self.executable_name = "LD55_Hungry_Portals"

    def build_folder(self):
        return {
            BuildType.DEBUG: "build/debug",
            BuildType.RELEASE: "build/release",
        }[self.current_build_type]

    def build_type_name(self):
        return {
            BuildType.DEBUG: "Debug",
            BuildType.RELEASE: "Release",
        }[self.current_build_type]

    def path_to_python(self):
        return {
            Platform.WINDOWS: "python",
            Platform.LINUX: "python3",
        }[self.current_platform]

    def path_to_7z(self):
        return {
            Platform.WINDOWS: "C:\\Program Files\\7-Zip\\7z.exe",
            Platform.LINUX: "7z",
        }[self.current_platform]


##################################### COMMON FUNCTIONS #####################################


def common_append_vscode_statusbar_label(task, statusbar_label: str = None):
    if statusbar_label:
        task["options"] = {
            "statusbar": {
                "hide": False,
                "label": statusbar_label,
            }
        }
    return task


def common_set_type_shell(task):
    task["type"] = "shell"
    return task


##################################### SPECIFIC TASKS #####################################


def generate_000_remove_vcpkg_folders_task(s: Settings):
    command = {
        Platform.WINDOWS: "rmdir /s /q vcpkg vcpkg_installed",
        Platform.LINUX: "rm -rf vcpkg vcpkg_installed",
    }
    return {
        "label": "000. Remove vcpkg folders",
        "command": command[s.current_platform],
    }


def generate_001_remove_build_folder_task(s: Settings):
    command = {
        Platform.WINDOWS: "rmdir /s /q build",
        Platform.LINUX: "rm -rf build",
    }
    return {
        "label": "001. Remove build folder",
        "command": command[s.current_platform],
    }


def generate_002_git_submodule_update_task(s: Settings):
    return {
        "label": "002. Git submodule update",
        "command": "git submodule update --init --recursive",
    }


def generate_003_install_vcpkg_as_subfolder_task(s: Settings):
    command = {
        Platform.WINDOWS: "git clone https://github.com/microsoft/vcpkg && .\\vcpkg\\bootstrap-vcpkg.bat && .\\vcpkg\\vcpkg install --triplet=x64-Windows",
        Platform.LINUX: "git clone https://github.com/microsoft/vcpkg && ./vcpkg/bootstrap-vcpkg.sh && ./vcpkg/vcpkg install --triplet=x64-linux",
    }
    return {
        "label": "003. Install vcpkg as subfolder",
        "command": command[s.current_platform],
    }


def generate_005_clear_console_task(s: Settings):
    command = {
        Platform.WINDOWS: "cls",
        Platform.LINUX: "clear",
    }
    return {
        "label": "005. Clear console",
        "command": command[s.current_platform],
    }


def generate_010_cmake_configure_task(s: Settings):
    export_compile_commands = {
        ExportCompileCommands.YES: " -DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        ExportCompileCommands.NO: "",
    }[s.export_compile_commands]

    command = (
        f"cmake -S . -B {s.build_folder()} -DCMAKE_BUILD_TYPE={s.build_type_name()} -G{s.make_tool} -DCMAKE_CXX_COMPILER={s.compiler} -DCMAKE_TOOLCHAIN_FILE={s.toolchain_file} {export_compile_commands}",
    )

    return {
        "label": "010. Configure",
        "command": command,
    }


def generate_020_cmake_build_task(s: Settings):
    stop_on_first_error = {
        StopOnFirstError.YES: "-k 0",
        StopOnFirstError.NO: "",
    }[s.stop_on_first_error]

    return {
        "label": "020. + Build",
        "command": f"cmake --build {s.build_folder()} -- {stop_on_first_error}",
        "dependsOn": [
            "010. Configure",
        ],
    }


def generate_030_copy_config_json_task(s: Settings):
    return {
        "label": "030. + Copy config.json",
        "command": f"cmake -E copy config.json {s.build_folder()}/src/config.json",
        "dependsOn": [
            "020. + Build",
        ],
    }


def generate_040_copy_assets_task(s: Settings):
    return {
        "label": "040. + Copy assets",
        "command": f"cmake -E copy_directory assets {s.build_folder()}/src/assets",
        "dependsOn": [
            "030. + Copy config.json",
        ],
    }


def generate_050_run_task(s: Settings):
    return {
        "label": "050. + Run",
        "command": f"${{workspaceFolder}}/{s.build_folder()}/src/{s.executable_name}.exe",
        "dependsOn": [
            "040. + Copy assets",
        ],
    }


def generate_060_pack_task(s: Settings):
    return {
        "label": "060. + Pack",
        "command": f'{s.path_to_python()} scripts/pack_binaries.py "{s.path_to_7z()}" ${{workspaceFolder}} {s.build_type_name()} {s.executable_name}_{s.build_type_name()}',
        "dependsOn": [
            "040. + Copy assets",
        ],
    }


def generate_070_just_run_task(s: Settings):
    return {
        "label": "070. Run",
        "command": f"${{workspaceFolder}}/build/debug/src/{s.executable_name}.exe",
    }


##################################### MAIN FUNCTION #####################################


def generate_tasks():
    tasks = {"version": "2.0.0", "tasks": []}
    settings = Settings()

    functions_and_statusbar_name = [
        (generate_000_remove_vcpkg_folders_task, None),
        (generate_001_remove_build_folder_task, None),
        (generate_002_git_submodule_update_task, None),
        (generate_003_install_vcpkg_as_subfolder_task, None),
        (generate_005_clear_console_task, None),
        (generate_010_cmake_configure_task, None),
        (generate_020_cmake_build_task, "Build"),
        (generate_030_copy_config_json_task, None),
        (generate_040_copy_assets_task, None),
        (generate_050_run_task, "Run"),
        (generate_060_pack_task, "Pack"),
        (generate_070_just_run_task, None),
    ]

    for function, statusbar_visibility in functions_and_statusbar_name:
        task = function(settings)
        if statusbar_visibility:
            task = common_append_vscode_statusbar_label(task, statusbar_visibility)
        task = common_set_type_shell(task)
        tasks["tasks"].append(task)

    return tasks


if __name__ == "__main__":
    current_file_dir = os.path.dirname(os.path.abspath(__file__))
    tasks_config = generate_tasks()
    with open(f"{current_file_dir}/../.vscode/tasks.json", "w") as tasks_file:
        json.dump(tasks_config, tasks_file, indent=4)
