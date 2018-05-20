# qolibri EPWING dictionary viewer

This is a fork of the qolibri software originally developed by [bop](https://osdn.net/users/bop/),
then improved by [Fujii Hironori](https://github.com/fujii),
[cybersphinx](https://github.com/cybersphinx), and [Matthias von Faber](https://github.com/mvf).

## Install on Debian 9 stretch

See the [releases](https://github.com/ludios/qolibri/releases) for precompiled amd64 packages.

## Building

This repository includes Debian packaging that should work your Debian package building tools.

If you would like to build it without using the Debian packaging, run:

```
sudo apt-get install --no-install-recommends qtbase5-dev qt5-qmake qt5-default qttools5-dev-tools qtmultimedia5-dev libqt5webkit5-dev libeb16-dev zlib1g-dev
qmake
make
```

The resulting binary can then be put somewhere in the path as it includes all the resources needed to run.

## Help with usage

[AJATT Tips: How to Make Sentence Cards (SRS)](https://www.youtube.com/watch?v=kny7eCfx9dA)

[Making Monolingual Sentence Cards in Real Time](https://www.youtube.com/watch?v=BzuLGmkihf4)
