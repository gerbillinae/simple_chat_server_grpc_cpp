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
This should create a `vcpkg_installed` directory.

## Build `server` and `client`
The server and client targets are defined in the root ./CMakeLists.txt. This is our code.

```shell
cd cmake-build-debug
cmake ..
cmake --build . --target server 
cmake --build . --target client
```

## Test

```shell
./cmake-build-debug/server &
./cmake-build-debug/client <your_name_here>
```

The result should look something like:

```text
$ ./cmake-build-debug/server &
[server] listening on 0.0.0.0:50051
$ ./cmake-build-debug/client World
[client] connecting to server at localhost:50051
[client] sending HelloRequest with name: World
[server] received HelloRequest with name: World
[client] received: Hello World
```

You can kill the server by typing `fg` then `Ctrl-C`

## Conclusion

Using an in-tree vcpkg is fool-proof, but isn't great for automated builds where the dependencies do not change often.

