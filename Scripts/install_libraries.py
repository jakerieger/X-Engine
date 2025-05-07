"""
Script that automates the process of building and installing the XENGINE libraries for use in
external projects.
"""
import os
import sys
import subprocess
import platform


def check_admin():
    """Check if script is running with administrative privileges."""
    try:
        if platform.system() == 'Windows':
            import ctypes
            return ctypes.windll.shell32.IsUserAnAdmin() != 0
        else:
            # For Unix-based systems, check if user is root (uid 0)
            return os.geteuid() == 0
    except:
        return False


def run_command(command):
    """Execute a command and print output."""
    print(f"Executing: {command}")
    try:
        process = subprocess.Popen(
            command,
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True
        )

        # Real-time output
        while True:
            output = process.stdout.readline()
            if output == '' and process.poll() is not None:
                break
            if output:
                print(output.strip())

        return_code = process.poll()

        if return_code != 0:
            error_output = process.stderr.read()
            print(f"Error (code {return_code}):")
            print(error_output)
            return False
        return True
    except Exception as e:
        print(f"Execution error: {e}")
        return False


def main():
    # Check for admin privileges
    if not check_admin():
        print("This script requires administrative privileges.")
        print("Please run this script as administrator.")
        sys.exit(1)

    # Print welcome message
    print("=" * 50)
    print("       Build Configuration Installer")
    print("=" * 50)

    # Get configuration choice
    print("\nPlease select a configuration option:")
    print("1. Debug - Development build with debug symbols")
    print("2. Release - Optimized build for deployment")
    print("3. Dist - Distribution build with packaging")

    valid_choice = False
    choice = 1  # default config is Debug
    while not valid_choice:
        try:
            choice = int(input("\nEnter your choice (1-3): "))
            if 1 <= choice <= 3:
                valid_choice = True
            else:
                print("Invalid choice. Please enter 1, 2, or 3.")
        except ValueError:
            print("Invalid input. Please enter a number.")

    # Define build directory and configuration
    build_dir = "build"
    config_type = ""

    if choice == 1:
        config_type = "Debug"
    elif choice == 2:
        config_type = "Release"
    elif choice == 3:
        config_type = "Dist"  # Typical for distribution builds

    print(f"\nBuilding {config_type} configuration...\n")

    # Create build directory if it doesn't exist
    config_path = os.path.join(build_dir, config_type)
    if not os.path.exists(config_path):
        os.makedirs(config_path)

    os.chdir(config_path)

    # Run CMake configure
    cmake_config_cmd = f"cmake -G Ninja -DCMAKE_BUILD_TYPE={config_type} ../.."
    if not run_command(cmake_config_cmd):
        print("Configuration failed. Exiting.")
        sys.exit(1)

    # Run CMake build
    cmake_build_cmd = f"cmake --build . --config {config_type}"
    if not run_command(cmake_build_cmd):
        print("Build failed. Exiting.")
        sys.exit(1)

    print("\n" + "=" * 50)
    print(f"   {config_type} build completed successfully!")
    print("=" * 50)

    # Install CMake build
    cmake_install_cmd = "cmake --install ."
    if not run_command(cmake_install_cmd):
        print("Install failed. Exiting.")
        sys.exit(1)

    print("\n" + "=" * 50)
    print(f"   XENGINE libraries installed successfully!")
    print("=" * 50)


if __name__ == "__main__":
    main()
