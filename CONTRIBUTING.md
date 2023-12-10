# Contributing to Ryzom Core

:+1::tada: First off, thanks for taking the time to contribute! :tada::+1:


## Local development

For IDEs supporting dev containers the setup of required dependencies can be simplified. Here is an example using
Visual Studio Code.


### Visual Studio Code

Visual Studio Code supports [Developing inside a Container](https://code.visualstudio.com/docs/devcontainers/containers).
You can [follow the installation instructions here](https://code.visualstudio.com/docs/devcontainers/containers#_installation).
This way you have all the tools and libraries required to compile the code.
Once you [opened the project in a container](https://code.visualstudio.com/docs/devcontainers/containers#_quick-start-open-an-existing-folder-in-a-container)
add the [CMake Tools Extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) to support
that supports the tooling used for this project.

Now in order to configure and build that parts you want, there are a variety of options to set. Here is a common set to
build most parts of the project. Set these as required in your **Cmake: Configure Args**.

```
-DWITH_STATIC=ON
-DWITH_NEL_TESTS=OFF
-DWITH_NEL_SAMPLES=ON
-DWITH_LUA51=OFF
-DWITH_LUA52=ON
-DWITH_RYZOM=ON
-DWITH_RYZOM_SERVER=ON
-DWITH_RYZOM_CLIENT=ON
-DWITH_RYZOM_TOOLS=ON
-DWITH_NEL_TOOLS=ON
-DWITH_NELNS=ON
-DWITH_NELNS_LOGIN_SYSTEM=ON
-DWITH_NELNS_SERVER=ON
-DWITH_QT5=ON
-DWITH_LIBGSF=ON
-DWITH_MONGODB:BOOL=OFF
-DWITH_PCH:BOOL=OFF
```
