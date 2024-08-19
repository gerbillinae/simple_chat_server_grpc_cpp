# GRPC with VCPKG

## Setup

If you haven't cloned this repo with the `--recurse-submodules` flag, then get the vcpkg submodule:
```shell
git submodule update --init --recursive 
```

Build `vcpkg`:
```shell
source ./vcpkg/bootstrap-vcpkg.sh
```

## Install dependencies with `vcpkg`
This must be done from the same directory as the `vcpkg.json` file:
```shell
./vcpkg/vcpkg install
```

## Conclusion

Using an in-tree vcpkg in foolproof, but isn't efficent for automated builds where the 
