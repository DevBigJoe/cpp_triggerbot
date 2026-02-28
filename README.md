# Trigger Bot Overview

This project implements a simple **screen-trigger bot** for Windows using C++. 
It monitors a small circular area in the center of the screen and simulates a mouse click when it detects a significant color change in that area.
Triggerkey is XBUTTON1

---

## Behavior Summary

1. Waits for the user to hold the side mouse button.
2. Captures a reference snapshot of the small central area.
3. Continuously monitors that area for changes.
4. When a significant change is detected, simulates a left mouse click.
5. Minimal sleep (`1ms`) is used for CPU efficiency.

---

## Notes

- The bot works on **Windows** only.
- The pixel difference threshold can be adjusted for sensitivity.
- Currently, it uses a very small monitored area (`DIAMETER = 3 pixels`) which may be intended for high precision.
