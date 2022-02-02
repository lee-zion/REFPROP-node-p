# node module for REFPROP shared library

<img src="https://user-images.githubusercontent.com/34432835/152092436-00311527-c4c0-43de-bb91-edb0529ac0ea.png" height="320">

## How do I get shared library?

By running `npm run setup`, node-refprop-napi.node file is compiled.

The output of this CMake system is the shared library file based on your OS. 
To ensure ABI compatibility, it is highly recommended to use docker.
This project is based on Ubuntu 18.04.

## Issues within REFPROP

REFPROP itself is not in public domain, so this build system itself cannot work. 

by node-addon-api

# Build with Docker (for Linux Distribution)

Once you have docker installed, you should be able to run following command:

```
docker run hello-world
docker build -t refprop-node:1.x .
docker images
docker run -it [--name ANY_NAME] refprop-node:1.x /bin/bash
```

You are now inside the container. `root@<containerid>:/app#` will show up.
Enter the following command:

```
npm run setup
```

Check the build log.
After success, node-refprop-napi.node file for linux is created at project root.
To enter the container, run following command:

```
docker cp <containerid>/app/node-refprop-napi.node /Users/econ/basic-node-addon/docker-refprop.node
exit
```

# References
- Inspired by work of [EvilDrW](https://github.com/EvilDrW/node-refprop), nodejs module for working with NIST refprop.
- [REFPROP-cmake](https://github.com/usnistgov/REFPROP-cmake) offered CMake build system for shared library.
- [REFPROP-headers](https://github.com/CoolProp/REFPROP-headers) offered REFPROP header file.
