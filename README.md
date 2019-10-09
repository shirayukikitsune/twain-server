# Twain Server

This project exposes the TWAIN DSM as a web API.

This is intended to enable users to access scanners directly from their web browsers, which is not possible with the
current JavaScript API. This agent starts a background service, which exposes the TWAIN DSM to the browser as a HTTP
server (bound to _localhost_ address).

## Compiling

It is required a C++17 compiler. Tested compilers:

- Microsoft Visual C++ 2017 (toolset 14.1)
- Microsoft Visual C++ 2019 (toolset 14.2)
- GNU C++ Compiler (g++) 8.3.0
- clang 8.0.0

### Pre-requisites

In order to compile, CMake is also required. The minimum version required is 3.7.

### Dependencies

* [Boost 1.71](https://boost.org)
* [Loguru](https://github.com/emilk/loguru) (included as submodule)
* [Nlohmann JSON](https://github.com/nlohmann/json) (included as submodule)
* [Kitsune IOC](https://github.com/shirayukikitsune/ioc) (included as submodule)

To download the included dependencies:

* **During cloning**: Use the command `git clone <repo> --recurse-submodules`
* **If the repository is alredy cloned**: Use the command `git submodule update --init --recursive` from the project root

### Before compilation

The recommended way to run CMake is outside the project root.
If you are using Linux or Mac OSX, follow the steps below.
For Visual Studio, please check the section below. 

First, create a directory for our build (for example, **build**):

```
mkdir build
```

Then let CMake generate its cache:

```
cd build
cmake -DCMAKE_BUILD_TYPE=Release . -B build
```

#### Visual Studio

Visual Studio 2017 or later has internal support for CMake.
All you need is to open the root project folder in your IDE.

If you need to generate VS solutions, then you can use CMake-GUI or the command line equivalent.

### Compiling

In order to compile, just instruct CMake to do so with the following command:

```
cmake --build build --target twain-server
```  
