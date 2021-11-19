<h1 align=center>
thread-pool
</h1>

[![say thanks](https://img.shields.io/badge/Say%20Thanks-👍-1EAEDB.svg)](https://github.com/DeveloperPaul123/periodic-function/stargazers)
[![Discord](https://img.shields.io/discord/652515194572111872)](https://img.shields.io/discord/652515194572111872)

[![Ubuntu](https://github.com/DeveloperPaul123/thread-pool/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/DeveloperPaul123/thread-pool/actions/workflows/ubuntu.yml)
[![Windows](https://github.com/DeveloperPaul123/thread-pool/actions/workflows/windows.yml/badge.svg)](https://github.com/DeveloperPaul123/thread-pool/actions/workflows/windows.yml)
[![Style](https://github.com/DeveloperPaul123/thread-pool/actions/workflows/style.yml/badge.svg)](https://github.com/DeveloperPaul123/thread-pool/actions/workflows/style.yml)
[![Install](https://github.com/DeveloperPaul123/thread-pool/actions/workflows/install.yml/badge.svg)](https://github.com/DeveloperPaul123/thread-pool/actions/workflows/install.yml)

A simple, functional thread pool implementation using pure C++20.

## Features

* Built entirely with C++20
* Enqueue tasks with or without tracking results

## Integration

`dp::thread-pool` is a header only library. All the files needed are in `include/thread_pool`. 

### CMake

`ThreadPool` defines two CMake targets:

* `ThreadPool::ThreadPool`
* `dp::thread-pool`

You can then use `find_package()`:

```cmake
find_package(dp::thread-pool REQUIRED)
```

Alternatively, you can use something like [CPM](https://github.com/TheLartians/CPM) which is based on CMake's `Fetch_Content` module.

```cmake
CPMAddPackage(
    NAME thread-pool
    GITHUB_REPOSITORY DeveloperPaul123/thread-pool
    GIT_TAG #0cea9c12fb30cb677696c0dce6228594ce26171a change this to latest commit or release tag
)
```

## Usage

Simple example:

```cpp
// create a thread pool with a specified number of threads.
dp::thread_pool pool(4);

// add tasks, in this case without caring about results of individual tasks
pool.enqueue_detach([](int value) { /*...your task...*/ }, 34);
pool.enqueue_detach([](int value) { /*...your task...*/ }, 37);
pool.enqueue_detach([](int value) { /*...your task...*/ }, 38);
// and so on..
```

You can see other examples in the `/examples` folder.

## Buliding

This project has been built with:

* Visual Studio 2022
* Clang `10.+` (via WSL on Windows)
* GCC `11.+` (vis WSL on Windows)
* CMake `3.21+`

To build, run:

```bash
cmake -S . -B build
cmake --build build
```

### Build Options

| Option | Description | Default |
|:-------|:------------|:--------:|
| `TP_BUILD_TESTS` | Turn on to build unit tests. Required for formatting build targets. | ON |
| `TP_BUILD_EXAMPLES` | Turn on to build examples | ON |

### Run clang-format

Use the following commands from the project's root directory to check and fix C++ and CMake source style.
This requires _clang-format_, _cmake-format_ and _pyyaml_ to be installed on the current system. To use this feature you must turn on `TP_BUILD_TESTS`.

```bash
# view changes
cmake --build build/test --target format

# apply changes
cmake --build build/test --target fix-format
```

See [Format.cmake](https://github.com/TheLartians/Format.cmake) for details.

### Build the documentation

The documentation is automatically built and [published](https://developerpaul123.github.io/thread-pool) whenever a [GitHub Release](https://help.github.com/en/github/administering-a-repository/managing-releases-in-a-repository) is created.
To manually build documentation, call the following command.

```bash
cmake -S documentation -B build/doc
cmake --build build/doc --target GenerateDocs
# view the docs
open build/doc/doxygen/html/index.html
```

To build the documentation locally, you will need Doxygen and Graphviz on your system.

## Contributing

Contributions are very welcome. Please see [contribution guidelines for more info](CONTRIBUTING.md).

## License

The project is licensed under the MIT license. See [LICENSE](LICENSE) for more details.

## Author

| [<img src="https://avatars0.githubusercontent.com/u/6591180?s=460&v=4" width="100"><br><sub>@DeveloperPaul123</sub>](https://github.com/DeveloperPaul123) |
|:----:|
