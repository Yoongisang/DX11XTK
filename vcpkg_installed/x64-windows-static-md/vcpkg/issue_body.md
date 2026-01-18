Package: directxmath:x64-windows-static-md@2025-04-03

**Host Environment**

- Host: x64-windows
- Compiler: MSVC 19.44.35222.0
- CMake Version: 3.30.1
-    vcpkg-tool version: 2025-11-19-da1f056dc0775ac651bea7e3fbbf4066146a55f3
    vcpkg-readonly: true
    vcpkg-scripts version: 3af1d1e60af2b2abf55760538cd607829029b07a

**To Reproduce**

`vcpkg install `

**Failure logs**

```
-- Using cached Microsoft-DirectXMath-apr2025.tar.gz

```

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
  "dependencies": [
    "directxmath",
    "directxtk"
  ]
}

```
</details>
