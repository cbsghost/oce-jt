# JT Assistant & JT Reader

An open source implementation of JT (Jupiter Tessellation) viewer and reader. Forked from [Open CASCADE repository](https://git.dev.opencascade.org/gitweb/?p=jt.git).

## Getting Started

This instruction will get you a copy of the project up and running on your local machine. It has been tested on Debian Linux 10 (Buster), 9 (Stretch) and 8 (Jessie).

### Prerequisites

Install build environment. I use Ninja to reduce project build time.

```shell
sudo apt-get -y install build-essential cmake git ninja-build
```

Then install dependencies for the project.

```shell
sudo apt-get -y install qt5-default oce-draw liboce-\*-dev libeigen3-dev libtbb-dev zlib1g-dev
```

### Building & Running

Clone the `cbsghost/oce-jt` project from Github repository.

```shell
git clone --recursive https://github.com/cbsghost/oce-jt.git
```

Create a build directory.

```shell
cd oce-jt
mkdir build
```

Configure and build the project with Ninja.

```shell
cd build
cmake -G "Ninja" ..
ninja
```

Finally, the JT Assistant application can be run with the following command:

```shell
./JTAssistant/JTAssistant
```

## License

This project is licensed under the GNU General Public License v2.0 - see the [LICENSE.txt](LICENSE.txt) file for details.
