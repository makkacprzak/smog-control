# Description

Smog-control is a college project, whose aim is to display current and historical data regarding air pollution in Poland. Simple as that. 

# Easiest way to build project:

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
