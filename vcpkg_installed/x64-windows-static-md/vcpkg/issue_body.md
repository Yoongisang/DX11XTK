Package: directxtex[core,dx11]:x64-windows-static-md@2025-10-27

**Host Environment**

- Host: x64-windows
- Compiler: MSVC 19.44.35222.0
- CMake Version: 3.31.10
-    vcpkg-tool version: 2025-12-16-44bb3ce006467fc13ba37ca099f64077b8bbf84d
    vcpkg-scripts version: 66c0373dc7 2026-01-16 (9 days ago)

**To Reproduce**

`vcpkg install `

**Failure logs**

```
-- Using cached Microsoft-DirectXTex-oct2025.tar.gz
-- Cleaning sources at C:/vcpkg/buildtrees/directxtex/src/oct2025-77bc3b9854.clean. Use --editable to skip cleaning for the packages you specify.
-- Extracting source C:/vcpkg/downloads/Microsoft-DirectXTex-oct2025.tar.gz
-- Using source at C:/vcpkg/buildtrees/directxtex/src/oct2025-77bc3b9854.clean
-- Configuring x64-windows-static-md
-- Building x64-windows-static-md-dbg
-- Building x64-windows-static-md-rel
-- Fixing pkgconfig file: C:/vcpkg/packages/directxtex_x64-windows-static-md/lib/pkgconfig/DirectXTex.pc
-- Using cached msys2-mingw-w64-x86_64-pkgconf-1~2.5.1-1-any.pkg.tar.zst
-- Using cached msys2-msys2-runtime-3.6.5-1-x86_64.pkg.tar.zst
-- Using msys root at C:/vcpkg/downloads/tools/msys2/3e71d1f8e22ab23f

```

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
  "dependencies": [
    "directxmath",
    "directxtk",
    "directxtex"
  ]
}

```
</details>
