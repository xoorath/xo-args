# [Readme.md](../../Readme.md) > [Internal / Readme.md](../Readme.md) > dev-setup-linux.md

## Initial development setup (do this once)

Install build-essential, gcc and g++ multilib:

```sh
sudo apt install build-essential gcc-multilib g++-multilib
```

Allow execution of Premake5 and the project generation script:

```sh
sudo chmod +x ./tools/linux/premake5
sudo chmod +x ./scripts/generate-projects-linux.sh
```

## Generate project files

Generate project files each time you pull or add/remove code files:

```sh
./scripts/generate-projects-linux.sh
```

## Build all and run tests

```sh
cd ./build/gmake2/
make
```

```sh
./xo-args-tests/bin/Debug_x86/xo-args-tests
```

## See also:

* [dev-setup-linux.md](./dev-setup-linux.md)