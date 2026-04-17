# KDialogue
KDialogue for Godot 4.5

## Build
- Clone the repository
- Update submodules
    ```sh
    git submodule update --init --recursive
    ```
- Make sure `SCons` is installed from `pip` and run `CompileGodotCpp.ps1` (Windows)
- Set `CacheVariables` for CMake

| Cache Variables       | Example                      | Explain                                  | Optional? |
|-----------------------|------------------------------|------------------------------------------|-----------|
| GODOT_PROJECT_BIN_DIR | C:/path/to/godot-project/bin | Deployment path for GDExtension binaries | Yes       |

- Build CMake
- Compile Source Code