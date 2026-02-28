#include <windows.h>
#include <thread>
#include <vector>
#include <atomic>
#include <cmath>
#include <chrono>
#include <iostream>

// Screen and circle parameters
const int SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN);
const int SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN);
const int CENTER_X = SCREEN_WIDTH / 2;
const int CENTER_Y = SCREEN_HEIGHT / 2;
const int RADIUS = 2;             // Slightly larger radius for better detection
const int DIAMETER = RADIUS * 2;

// Atomic flag to control the trigger bot thread
std::atomic<bool> running{ true };

void TriggerBotThread() {
    // Create device contexts for screen capture
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, DIAMETER, DIAMETER);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    // Bitmap info structure for pixel capture
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = DIAMETER;
    bmi.bmiHeader.biHeight = -DIAMETER; // Top-down bitmap
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    // Pixel buffers reused each loop
    std::vector<BYTE> referencePixels(DIAMETER * DIAMETER * 4);
    std::vector<BYTE> currentPixels(DIAMETER * DIAMETER * 4);

    bool monitoring = false;

    while (running) {
        // Check side mouse button
        SHORT state = GetAsyncKeyState(VK_XBUTTON1);
        bool pressed = (state & 0x8000) != 0;

        if (pressed && !monitoring) {
            monitoring = true;

            // Capture reference screenshot
            BitBlt(hdcMem, 0, 0, DIAMETER, DIAMETER,
                hdcScreen, CENTER_X - RADIUS, CENTER_Y - RADIUS, SRCCOPY);
            GetDIBits(hdcMem, hBitmap, 0, DIAMETER,
                referencePixels.data(), &bmi, DIB_RGB_COLORS);
        }
        else if (!pressed && monitoring) {
            monitoring = false;
        }

        if (monitoring) {
            // Capture current screen area
            BitBlt(hdcMem, 0, 0, DIAMETER, DIAMETER,
                hdcScreen, CENTER_X - RADIUS, CENTER_Y - RADIUS, SRCCOPY);
            GetDIBits(hdcMem, hBitmap, 0, DIAMETER,
                currentPixels.data(), &bmi, DIB_RGB_COLORS);

            // Compare pixels to reference
            bool triggered = false;
            for (size_t i = 0; i < currentPixels.size(); i += 4) {
                int diff = std::abs(referencePixels[i] - currentPixels[i])       // B
                    + std::abs(referencePixels[i + 1] - currentPixels[i + 1]) // G
                    + std::abs(referencePixels[i + 2] - currentPixels[i + 2]); // R

                if (diff > 60) { // threshold
                    triggered = true;
                    break; // exit early
                }
            }

            if (triggered) {
                // Simulate mouse click
                INPUT input[2] = {};
                input[0].type = INPUT_MOUSE;
                input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                input[1].type = INPUT_MOUSE;
                input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
                SendInput(2, input, sizeof(INPUT));

                std::cout << "[+] Shot" << std::endl;

                // Minimal delay to prevent multiple triggers
                Sleep(50);
            }
        }

        // Minimal sleep to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Clean up
    SelectObject(hdcMem, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
}

int main(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    // Start the trigger bot thread
    std::thread triggerThread(TriggerBotThread);
    triggerThread.detach();

    // Keep main thread idle
    while (true) {
        Sleep(10000);
    }

    return 0;
}