[![Build osu-tracker](https://github.com/nyaruku/osu-tracker/actions/workflows/cmake-multi-platform.yml/badge.svg?branch=master&style=flat)](https://github.com/nyaruku/osu-tracker/actions/workflows/cmake-multi-platform.yml)
[![CodeQL Advanced](https://github.com/nyaruku/osu-tracker/actions/workflows/codeql.yml/badge.svg?branch=master)](https://github.com/nyaruku/osu-tracker/actions/workflows/codeql.yml)
# osu-tracker
A web server based osu! stats/session tracker

## How to use
1. run executable
2. visit http://127.0.0.1:10727

## Bug Reporting / Questions / Suggestions
- Open Issue or submit PR
- Join my Discord Server and just ask me:
- https://discord.gg/amj5vBjTQU

## Credits
- osu! API by osu!
- Score Inspector API by Amayakase
- Score Rank API by Respektive


## How to build from source:
- Windows
  - Visual Studio (Recommended)
- Linux
  - Make sure u have these installed:
    - cmake
    - libcurl4-openssl-dev (provides lib files for linking) | (curl is enough, if u are using arch)
    - ninja (generator)
  - Use bash scripts
    - "release.sh" , "debug.sh"
    - Usage
      
      Configure
      ```sh
      ./release.sh configure
      ```
      
      Build
      ```sh
      ./release.sh build
      ```

## TODO:
- Save Sessions
- Compare overall stats between sessions
- Custom Config Settings
- Local Database of Clears/Maps/Stats ?????