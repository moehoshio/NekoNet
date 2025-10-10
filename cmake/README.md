# NekoNet CMake Modules

This directory contains CMake modules used by the NekoNet project.

## NekoRunTimeCopy.cmake

Automatically copies runtime libraries (DLLs, shared objects, dylibs) to the target executable directory across different platforms.

### Features

- ✅ **Windows**: Automatically copies DLLs from vcpkg (Debug and Release configurations)
- ✅ **macOS**: Copies dylibs and sets RPATH correctly
- ✅ **Linux**: Copies .so files and configures RPATH
- ✅ **Automatic Detection**: Detects vcpkg installation automatically
- ✅ **Architecture Support**: Handles x64, x86, arm64, and arm architectures
- ✅ **Configuration Aware**: Copies appropriate debug/release libraries

### Usage

```cmake
# Add module path if not already added
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include the module
include(NekoRunTimeCopy)

# After defining your executable target
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE my_libraries)

# Copy runtime libraries
NekoRunTimeCopy(my_app)
```

### Supported Libraries

The module automatically handles common dependencies:

- **libcurl**: HTTP client library
- **OpenSSL**: SSL/TLS libraries (libssl, libcrypto)
- **zlib**: Compression library
- **Brotli**: Compression library
- **nghttp2**: HTTP/2 library
- **libssh2**: SSH library
- **libidn2**: Internationalized domain names

### Environment Variables

- `VCPKG_ROOT`: Path to vcpkg installation (optional, auto-detected)

The module attempts to detect vcpkg in the following order:
1. `VCPKG_ROOT` environment variable
2. From `CMAKE_TOOLCHAIN_FILE` path
3. Common installation paths (`C:/dev/vcpkg`, `${CMAKE_SOURCE_DIR}/vcpkg`)

### Platform-Specific Details

#### Windows
- Copies DLLs from `vcpkg/installed/{arch}/bin` (Release) and `vcpkg/installed/{arch}/debug/bin` (Debug)
- Handles debug suffix for libraries (e.g., `libcurl-d.dll`)
- Supports x64-windows and x86-windows architectures

#### macOS
- Copies dylibs from `vcpkg/installed/{arch}/lib`
- Sets `@executable_path` RPATH for portable executables
- Supports x64-osx and arm64-osx architectures
- Falls back to system libraries if vcpkg is not detected

#### Linux
- Copies shared objects from `vcpkg/installed/{arch}/lib`
- Sets `$ORIGIN` RPATH for portable executables
- Supports x64-linux, arm64-linux, and arm-linux architectures
- Falls back to system libraries if vcpkg is not detected

### Error Handling

- Non-existent libraries are silently skipped (using `|| (exit 0)`)
- Warning messages are displayed if vcpkg is not detected on Windows
- Informational messages on Unix systems when using system libraries

### Example Output

```
-- NekoRunTimeCopy: Configured for Windows (x64-windows)
--   Debug DLLs from: C:/dev/vcpkg/installed/x64-windows/debug/bin
--   Release DLLs from: C:/dev/vcpkg/installed/x64-windows/bin
```

### Troubleshooting

**Windows: DLLs not copied**
- Ensure `VCPKG_ROOT` environment variable is set
- Check that vcpkg libraries are installed: `vcpkg list`
- Verify the architecture matches your build (x64 vs x86)

**macOS/Linux: Libraries not found**
- Check if vcpkg is installed and configured
- System libraries may be used automatically (check with `ldd` or `otool`)
- Ensure libraries are installed via vcpkg: `vcpkg install curl openssl`

**RPATH Issues**
- On Unix systems, verify RPATH is set: `readelf -d executable` (Linux) or `otool -l executable` (macOS)
- Build artifacts should have `$ORIGIN` or `@executable_path` in RPATH

### Advanced Usage

To extend the module with additional libraries, edit the `DLL_LIST`, `DYLIB_PATTERNS`, or `SO_PATTERNS` in the respective platform functions:

```cmake
# In NekoRunTimeCopy.cmake
set(DLL_LIST
    "libcurl;-d"
    "libssl-3-x64;"
    # Add your custom library here:
    "mylib;-d"  # mylib-d.dll (debug) / mylib.dll (release)
)
```

### License

This module is part of the NekoNet project and follows the same license (MIT OR Apache-2.0).
