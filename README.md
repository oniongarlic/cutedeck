# Qt C++ and QtQuick friendly wrapper around Blackmagic Design DeckLink API

This library makes it easy to use DeckLink devices from Qt and QtQuick applications.

## For example you can:
* Use MediaPlayer to play videos and audio on an DeckLink output
* Monitor an input in a VideOutput
* Grab frames to images from a input
* Show still images on a output
* Use alpha channel images for keying
* Redirect a webcam input to a DeckLink output

See the included example app for some ideas on what you can do.

## Requirements:

* Qt 6.8 or later
* Installed DeckLink desktop drivers
* DeckLink SDK 16.0 or later
* Windows or Linux (Mac untested, might need minor code adjustments)

## Tested devices

* Intensity Pro 4K
* Mini Monitor HD
* DeckLink Duo 2
* UltraStudio Monitor 3G
