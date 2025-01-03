# [Readme.md](../../Readme.md) > [Internal / Readme.md](../Readme.md) > dev-setup-windows.md

## Initial development setup (do this once)

[Install Visual Studio 2022](https://visualstudio.microsoft.com/vs/)

[Setup a WSL development environment](https://learn.microsoft.com/en-us/windows/wsl/setup/environment)

Follow the [dev-setup-linux.md](./dev-setup-linux.md) guide within WSL.

## Generate project files

Generate project files each time you pull or add/remove code files:

```sh
./scripts/generate-projects-windows.bat
```

## Build all and run tests

open `./build/vs2022/xo-args.sln` and build the solution.

Set the test project as the startup project and run it. It does not require
any arguments or other inputs.

## See also:

* [dev-setup-windows.md](./dev-setup-windows.md)
