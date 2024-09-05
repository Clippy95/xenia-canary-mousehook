# Xenia Canary Netplay

This is a fork of [Xenia Canary](https://github.com/xenia-canary/xenia-canary) which implements online multiplayer features. The REST API powering this fork can be found [here](https://github.com/AdrianCassar/Xenia-WebServices#xenia-web-services).

Current online sessions are displayed at [https://xenia-netplay-2a0298c0e3f4.herokuapp.com/](https://xenia-netplay-2a0298c0e3f4.herokuapp.com/).

---

## Netplay Wiki

The wiki contains a compatibility list and general information about the current state netplay.

To get started with netplay read [config setup](https://github.com/AdrianCassar/xenia-canary/wiki/Config-Setup).

### Wiki Pages:
* [Home](https://github.com/AdrianCassar/xenia-canary/wiki)
* [Netplay Compatibility](https://github.com/AdrianCassar/xenia-canary/wiki/Netplay-Compatibility)
* [Testing Procedure](https://github.com/AdrianCassar/xenia-canary/wiki/Testing-Procedure)
* [Config Setup](https://github.com/AdrianCassar/xenia-canary/wiki/Config-Setup)
* [FAQ](https://github.com/AdrianCassar/xenia-canary/wiki/FAQ)

### Netplay Builds:
* [Latest stable release](https://github.com/AdrianCassar/xenia-canary/releases/latest)
* [Github Actions](https://github.com/AdrianCassar/xenia-canary/actions?query=actor%3AAdrianCassar+branch%3Anetplay_canary_experimental)
    * These builds are latest work in progress and require a github account to download.
* [Mousehook](https://github.com/marinesciencedude/xenia-canary-mousehook/releases?q=Netplay)
    * A limited number of games have netplay and mousehook support.

---

# Mousehook

This is a fork of [emoose's Xenia build](https://github.com/emoose/xenia) as originally [ported to Canary by Marcelo20XX](https://www.reddit.com/r/emulation/comments/qppb6d/goldeneye_xbla_with_updated_xenia_canary_mousehook/).

## Supported Games

| Game  | Notes  |
|---|---|
| Orange Box | All Games TU0 |
| Portal Still Alive |
| CSGO | |
| CSGO Beta | |
| Left 4 Dead 2 | TU0 |
| Left 4 Dead | TU0, GOTY |
| Portal 2 |  TU0 |
| Team Fortress 2 | TU0 |
| Bloody Good Time |
| Postal III | |
| GoldenEye XBLA | Nov 16th 2007, also renamed as 'Aug 25th 2007' |
| Perfect Dark XBLA | b33, b52 (TU0) & b102 |
| Halo 3 | TU0/TU3 & 08172 'delta' |
| Halo 3: ODST | |
| Halo Reach | TU0/TU1 |
| Halo 4 | TU0/TU8 |
| Crackdown 2 | TU0/TU5 |
| Saints Row 2 | TU3 |
| Dark Messiah of Might and Magic | Singleplayer & Multiplayer |
| Gears Of Wars 1 | TU0/TU5 |
| Gears Of Wars 2 | TU0/TU6 |
| Gears Of Wars 3 | TU0/TU6 |
| Gears Of Wars Judgement | TU0/TU4|

---

<p align="center">
    <a href="https://github.com/xenia-canary/xenia-canary/tree/canary_experimental/assets/icon">
        <img height="120px" src="https://raw.githubusercontent.com/xenia-canary/xenia/master/assets/icon/128.png" />
    </a>
</p>

<h1 align="center">Xenia Canary - Xbox 360 Emulator</h1>

Xenia is an experimental emulator for the Xbox 360. For more information, see the
[Xenia wiki](https://github.com/xenia-canary/xenia-canary/wiki).

Come chat with us about **emulator-related topics** on [Discord](https://discord.gg/Q9mxZf9).
For developer chat join `#dev` but stay on topic. Lurking is not only fine, but encouraged!
Please check the [FAQ](https://github.com/xenia-project/xenia/wiki/FAQ) page before asking questions.
We've got jobs/lives/etc, so don't expect instant answers.

Discussing illegal activities will get you banned.

## Status

Buildbot | Status | Releases
-------- | ------ | --------
Windows | [![CI](https://github.com/xenia-canary/xenia-canary/actions/workflows/Windows_build.yml/badge.svg?branch=canary_experimental)](https://github.com/xenia-canary/xenia-canary/actions/workflows/Windows_build.yml) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/cd506034fd8148309a45034925648499)](https://app.codacy.com/gh/xenia-canary/xenia-canary/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade) | [Latest](https://github.com/xenia-canary/xenia-canary/releases/latest) ◦ [All](https://github.com/xenia-canary/xenia-canary/releases)
Linux | Curently unsupported
Netplay Build | | [Latest](https://github.com/AdrianCassar/xenia-canary/releases/latest)

## Quickstart

See the [Quickstart](https://github.com/xenia-project/xenia/wiki/Quickstart) page.

## FAQ

See the [frequently asked questions](https://github.com/xenia-project/xenia/wiki/FAQ) page.

## Game Compatibility

See the [Game compatibility list](https://github.com/xenia-project/game-compatibility/issues)
for currently tracked games, and feel free to contribute your own updates,
screenshots, and information there following the [existing conventions](https://github.com/xenia-project/game-compatibility/blob/master/README.md).

## Building

See [building.md](docs/building.md) for setup and information about the
`xb` script. When writing code, check the [style guide](docs/style_guide.md)
and be sure to run clang-format!

## Contributors Wanted!

Have some spare time, know advanced C++, and want to write an emulator?
Contribute! There's a ton of work that needs to be done, a lot of which
is wide open greenfield fun.

**For general rules and guidelines please see [CONTRIBUTING.md](.github/CONTRIBUTING.md).**

Fixes and optimizations are always welcome (please!), but in addition to
that there are some major work areas still untouched:

* Help work through [missing functionality/bugs in games](https://github.com/xenia-project/xenia/labels/compat)
* Reduce the size of Xenia's [huge log files](https://github.com/xenia-project/xenia/issues/1526)
* Skilled with Linux? A strong contributor is needed to [help with porting](https://github.com/xenia-project/xenia/labels/platform-linux)

See more projects [good for contributors](https://github.com/xenia-project/xenia/labels/good%20first%20issue). It's a good idea to ask on Discord and check the issues page before beginning work on
something.

## Disclaimer

The goal of this project is to experiment, research, and educate on the topic
of emulation of modern devices and operating systems. **It is not for enabling
illegal activity**. All information is obtained via reverse engineering of
legally purchased devices and games and information made public on the internet
(you'd be surprised what's indexed on Google...).
