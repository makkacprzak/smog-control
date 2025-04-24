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
