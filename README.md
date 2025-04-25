# Description

Smog-control is a college project, whose aim is to display current and historical data regarding air pollution in Poland in a GUI. Simple as that. 

# Build instructions:

## Build using console

#### 1. Make sure you have a C++ compiler installed
#### 2. Clone repository

```bash
git clone https://github.com/makkacprzak/smog-control.git
```

#### 3. Install all depenancies:

***Debian based distributions***

```bash
sudo apt install cmake curl libcurl3-openssl-dev qt6-base-dev libqt6charts6-dev
```

***Red hat based distributions***

```bash
sudo dnf install cmake curl libcurl-devel qt6-qtbase-devel qt6-qtcharts-devel
```


#### 2. Build using cmake

```bash
cmake -S . -B build && cmake --build Build
```

## Build using QT Creator
Some package managers may use different names for QT libraries, or include different libraries with the base, so this is a more universal solution.

#### 1. Install QT Creator following these instructions: https://doc.qt.io/qtcreator/creator-how-to-install.html

#### 2. Install curl and libcurl-dev if needed
Some package managers include libcurl with curl package (e.g. homebrew), so check for yourself and act accordingly.

#### 3. Clone repository

```bash
git clone https://github.com/makkacprzak/smog-control.git
```

#### 3. Open project using QT Creator
On `Welcome` screen select `Open Project...`, navigate to repo directory and select the `CMakeLists.txt` file. From my experience you can accept the default configuration. Then just click the play button, and it will build and run the project.

# License

* This project is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.

* This project uses [Qt](https://www.qt.io/) framework for GUI features. Qt is licensed under the GNU Lesser General Public License (LGPL) version 3.0. See the [Qt License](LICENSES/LICENSE-QT.md) for more information.

* This project uses curl library for HTTP requests. Curl is licensed under the curl license, which is a mix of the MIT and BSD licenses. See the [Curl License](LICENSES/LICENSE-CURL.md) for more information.

* This project uses [nlohmann/json](https://json.nlohmann.me/) library for JSON parsing. nlohmann/json is licensed under the MIT license. See the [nlohmann/json License](LICENSES/LICENSE-NLOHMANN.md) for more information.
