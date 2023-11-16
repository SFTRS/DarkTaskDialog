// © Softros Systems
// GPL3
// github.com/SFTRS/DarkTaskDialog

#pragma once

#include <windows.h>

#include <detours.h>
#pragma comment(lib, "Detours.lib")

#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

#pragma comment(lib, "Msimg32.lib")

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#include <Vssym32.h>
#include <Vsstyle.h>

#include <string>

#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

#include <set>

namespace SFTRS {
    namespace DarkTaskDialog
    {
        enum Theme { dark, light };

        namespace detail
        {
            /* COLORS */
            const COLORREF white = RGB(255, 255, 255);
            const HBRUSH darkBrush = CreateSolidBrush(RGB(36, 36, 36));
            const HBRUSH notSoDarkBrush = CreateSolidBrush(RGB(51, 51, 51));

            /* ORIGINAL FUNCTIONS */

            //Color of text for main headers, check boxes, expanded areas, and expander buttons.
            static HRESULT(WINAPI* trueGetThemeColor)(HTHEME, int, int, int, COLORREF*) = GetThemeColor;   

            //Color of text for radio buttons and command link button main text.
            static HRESULT(WINAPI* trueDrawThemeText)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, LPCRECT) = DrawThemeText;
            
            //Color of text for the second line of command link buttons.
            static HRESULT(WINAPI* trueDrawThemeTextEx)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, LPRECT, const DTTOPTS*) = DrawThemeTextEx; 

            //Primary panel background, secondary panel background, footnote panel background, separator color, and Windows 11-specific: expander button and checkbox.
            static HRESULT(WINAPI* trueDrawThemeBackgroundEx)(HTHEME, HDC, int, int, LPCRECT, const DTBGOPTS*) = DrawThemeBackgroundEx;
            
            //Color of command link button arrows and progress bars.
            static HRESULT(WINAPI* trueDrawThemeBackground)(HTHEME, HDC, int, int, LPCRECT, LPCRECT) = DrawThemeBackground; 
            
            //Wraps callback to enable detours for all necessary functions and enumerates children to subclass them.
            static HRESULT(WINAPI* trueTaskDialogIndirect)(const TASKDIALOGCONFIG*, int*, int*, BOOL*) = TaskDialogIndirect;
            
            //Early subclassing of the SysLink control to eliminate blinking during page switches.
            static HWND(WINAPI* trueCreateWindowEx)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID) = CreateWindowExW;


            /* DATA STORAGE */
            static std::set<HWND> taskDialogs;
            static Theme theme;
            struct TaskDalogCallbackWrap
            {
                PFTASKDIALOGCALLBACK pfCallback;
                LONG_PTR lpCallbackData;
            };
            static int painting = 0;


            /* FUNCTIONS HELPERS */

            std::wstring wndClass(HWND hWnd)
            {
                std::wstring className(255, 0);
                className.resize(GetClassName(hWnd, &className[0], 255));
                return className;
            }

            COLORREF increaseBrightness(COLORREF color, int increase = 90)
            {
                BYTE r, g, b;
                r = min(255, GetRValue(color) + increase);
                g = min(255, GetGValue(color) + increase);
                b = min(255, GetBValue(color) + increase);
                return RGB(r, g, b);
            }

            /* GRAPHICS HELPERS */

            const BLENDFUNCTION blend_f = { AC_SRC_OVER ,0, 255, AC_SRC_ALPHA };

            struct MemoryBitmapAndDC
            {
                MemoryBitmapAndDC() = delete;
                MemoryBitmapAndDC(const MemoryBitmapAndDC&) = delete;

                HBITMAP hBitmap;
                HDC DC;
                RECT rect;
                int width;
                int height;
                MemoryBitmapAndDC(LPCRECT pRect)
                {
                    width = pRect->right - pRect->left;
                    height = pRect->bottom - pRect->top;
                    rect = { 0,0,width,height };
                    DC = CreateCompatibleDC(NULL);
                    HDC dcScreen = GetDC(NULL);
                    hBitmap = CreateCompatibleBitmap(dcScreen, width, height);
                    ReleaseDC(NULL, dcScreen);
                    hBmpOld = (HBITMAP)SelectObject(DC, hBitmap);
                }
                ~MemoryBitmapAndDC()
                {
                    SelectObject(DC, hBmpOld);
                    DeleteObject(hBitmap);
                    DeleteDC(DC);
                }

            private:
                HBITMAP hBmpOld;

            };

            Gdiplus::Bitmap* GdiBitmapFromHBITMAPwithAlpha(HBITMAP source, int width, int height)
            {
                Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);

                Gdiplus::BitmapData target_info;
                Gdiplus::Rect rect(0, 0, width, height);

                Gdiplus::Status result = bitmap->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &target_info);
                if (result != Gdiplus::Ok || target_info.Stride != width * 4)
                    return NULL;

                GetBitmapBits(source, width * 4 * height, target_info.Scan0);

                bitmap->UnlockBits(&target_info);

                return bitmap;
            }

            void AdjustBrightness(HDC hdc, LPCRECT rect, HBITMAP bitmap = NULL, float f = -0.78)
            {
                Gdiplus::Bitmap* bmp;
                if (!bitmap)
                {
                    MemoryBitmapAndDC canvas(rect);
                    AlphaBlend(canvas.DC, 0, 0, canvas.width, canvas.height, hdc, rect->left, rect->top, canvas.width, canvas.height, blend_f);
                    bmp = GdiBitmapFromHBITMAPwithAlpha(canvas.hBitmap, canvas.width, canvas.height);
                }
                else
                    bmp = GdiBitmapFromHBITMAPwithAlpha(bitmap, rect->right, rect->bottom);

                Gdiplus::Graphics graphics(hdc);
                Gdiplus::Rect gdiRect(rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top);

                Gdiplus::SolidBrush clearBrush(Gdiplus::Color::Transparent);
                graphics.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
                graphics.FillRectangle(&clearBrush, gdiRect);
                graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);

                Gdiplus::ColorMatrix colorMatrix =
                {
                    1, 0, 0, 0, 0,
                    0, 1, 0, 0, 0,
                    0, 0, 1, 0, 0,
                    0, 0, 0, 1, 0,
                    f, f, f, 0, 1
                };
                Gdiplus::ImageAttributes imageAttr;
                imageAttr.SetColorMatrix(&colorMatrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
                graphics.DrawImage(bmp, gdiRect, 0, 0, gdiRect.Width, gdiRect.Height, Gdiplus::UnitPixel, &imageAttr);

                delete bmp;
            }


            /* DETOURED FUNCTIONS */

            HRESULT WINAPI myGetThemeColor(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF* pColor)
            {
                HRESULT retVal = trueGetThemeColor(hTheme, iPartId, iStateId, iPropId, pColor);

                if (!painting)
                    return retVal;

                if (iPropId == TMT_TEXTCOLOR && iPartId == TDLG_MAININSTRUCTIONPANE)
                {
                    *pColor = increaseBrightness(*pColor, 150); // Main header.
                }
                else if (iPropId == TMT_TEXTCOLOR)
                {
                    *pColor = white; // Text color for check boxes, expanded text, and expander button text.
                }
                return retVal;
            }


            HRESULT WINAPI myDrawThemeText(HTHEME  hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect)
            {
                HRESULT retVal;
                if (painting && (iPartId == BP_RADIOBUTTON || iPartId == BP_COMMANDLINK) && iStateId != PBS_DISABLED)
                {
                    COLORREF color;
                    if (iPartId == BP_RADIOBUTTON)
                    {
                        color = white;
                    }
                    else
                    {
                        trueGetThemeColor(hTheme, iPartId, iStateId, TMT_TEXTCOLOR, &color);
                        color = increaseBrightness(color);
                    }
                    DTTOPTS options = { sizeof(DTTOPTS), DTT_TEXTCOLOR, color };
                    retVal = trueDrawThemeTextEx(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, (LPRECT)pRect, &options);
                }
                else
                {
                    retVal = trueDrawThemeText(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, dwTextFlags2, pRect);
                }
                return retVal;
            }

            HRESULT WINAPI myDrawThemeTextEx(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect, const DTTOPTS* pOptions)
            {
                if (painting && iPartId == BP_COMMANDLINK)
                {
                    DTTOPTS options = pOptions ? *pOptions : DTTOPTS{ sizeof(DTTOPTS) };
                    options.dwFlags |= DTT_TEXTCOLOR;
                    trueGetThemeColor(hTheme, iPartId, iStateId, TMT_TEXTCOLOR, &options.crText);
                    options.crText = increaseBrightness(options.crText);
                    return trueDrawThemeTextEx(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, pRect, &options);
                }
                else
                    return trueDrawThemeTextEx(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, pRect, pOptions);
            }

            HRESULT WINAPI myDrawThemeBackgroundEx(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pRect, const DTBGOPTS* pOptions)
            {
                if (!painting)
                    return trueDrawThemeBackgroundEx(hTheme, hdc, iPartId, iStateId, pRect, pOptions);

                HRESULT retVal = S_OK;

                switch (iPartId)
                {
                    case TDLG_PRIMARYPANEL:
                        FillRect(hdc, pRect, notSoDarkBrush);
                        break;
                    case TDLG_FOOTNOTEPANE:
                    case TDLG_SECONDARYPANEL:
                        FillRect(hdc, &pOptions->rcClip, darkBrush);
                        break;
                    case TDLG_FOOTNOTESEPARATOR:
                        retVal = trueDrawThemeBackgroundEx(hTheme, hdc, iPartId, iStateId, pRect, pOptions);
                        AdjustBrightness(hdc, pRect);
                        break;
                    case TDLG_EXPANDOBUTTON:
                    {
                        // In Windows 11, buttons lack background, making them indistinguishable on dark backgrounds.
                        // To address this, we invert the button. This technique isn't applicable to Windows 10 as it causes the button's border to appear chipped.
                        static enum { yes, no, unknown } mustInvertButton = unknown;
                        if (mustInvertButton == unknown)
                        {
                            trueDrawThemeBackgroundEx(hTheme, hdc, iPartId, iStateId, pRect, pOptions);
                            int buttonCenterX = pOptions->rcClip.left + (pOptions->rcClip.right - pOptions->rcClip.left) / 2;
                            int buttonCenterY = pOptions->rcClip.top + (pOptions->rcClip.bottom - pOptions->rcClip.top) / 2;
                            COLORREF centerPixel = GetPixel(hdc, buttonCenterX, buttonCenterY);
                            mustInvertButton = centerPixel == white ? no : yes;
                        }
                        FillRect(hdc, pRect, darkBrush);
                        if (mustInvertButton == yes) InvertRect(hdc, pRect);
                        retVal = trueDrawThemeBackgroundEx(hTheme, hdc, iPartId, iStateId, pRect, pOptions);
                        if (mustInvertButton == yes) InvertRect(hdc, pRect);
                        break;
                    }
                    case BP_CHECKBOX:
                    {
                        HTHEME darkButtonTheme = OpenThemeData(NULL, L"DarkMode_Explorer::Button");
                        retVal = trueDrawThemeBackgroundEx(darkButtonTheme ? darkButtonTheme : hTheme, hdc, iPartId, iStateId, pRect, pOptions);
                        if (darkButtonTheme)
                            CloseThemeData(darkButtonTheme);
                    }
                }
                return retVal;
            }

            HRESULT WINAPI myDrawThemeBackground(HTHEME  hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pRect, LPCRECT pClipRect)
            {
                HRESULT result;
                if ((iPartId == BP_COMMANDLINKGLYPH || iPartId == BP_COMMANDLINK) && painting && iStateId != PBS_DISABLED)
                {
                    MemoryBitmapAndDC canvas(pRect);
                    result = trueDrawThemeBackground(hTheme, canvas.DC, iPartId, iStateId, &canvas.rect, NULL);
                    AdjustBrightness(canvas.DC, &canvas.rect, canvas.hBitmap, iPartId == BP_COMMANDLINK ? 1.0f : 0.35f);
                    AlphaBlend(hdc, pRect->left, pRect->top, canvas.width, canvas.height, canvas.DC, 0, 0, canvas.width, canvas.height, blend_f);
                    return result;
                }

                result = trueDrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);

                if (painting && iPartId == PP_FILL) // Progress bar color fill.
                {
                    AdjustBrightness(hdc, pRect, NULL, -0.2f);
                }
                if (painting && iPartId == PP_TRANSPARENTBAR) // Progress bar background.
                {
                    AdjustBrightness(hdc, pRect);
                }

                return result;
            }

            LRESULT CALLBACK Subclassproc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);

            HWND WINAPI myCreateWindowEx(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
            {
                HWND hwnd = trueCreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
                if (HIWORD(lpClassName) != 0 && std::wstring(lpClassName) == WC_LINK)
                {
                    SetWindowSubclass(hwnd, Subclassproc, (UINT_PTR)hwnd, 0);
                }
                return hwnd;
            }

            /* ALL CHILDREN SUBCLASSING AND CALLBACKS */

            HRESULT CALLBACK taskdialogcallback(HWND, UINT, WPARAM, LPARAM, LONG_PTR);

            LRESULT CALLBACK Subclassproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
            {
                if (uMsg == TDM_NAVIGATE_PAGE)
                {
                    TASKDIALOGCONFIG* trueCongif = (TASKDIALOGCONFIG*)lParam;
                    TaskDalogCallbackWrap* trueCallBackData = (TaskDalogCallbackWrap*)dwRefData;
                    trueCallBackData->pfCallback = trueCongif ? trueCongif->pfCallback : NULL;
                    trueCallBackData->lpCallbackData = trueCongif ? trueCongif->lpCallbackData : NULL;

                    TASKDIALOGCONFIG myConfig = trueCongif ? *trueCongif : TASKDIALOGCONFIG{ sizeof(TASKDIALOGCONFIG) };
                    myConfig.pfCallback = taskdialogcallback;
                    myConfig.lpCallbackData = (LONG_PTR)trueCallBackData;

                    if (theme == dark)
                    {
                        DetourTransactionBegin();
                        DetourUpdateThread(GetCurrentThread());
                        DetourAttach(&(PVOID&)trueCreateWindowEx, myCreateWindowEx);
                        DetourTransactionCommit();
                    }
                    LRESULT retVal = DefSubclassProc(hWnd, uMsg, wParam, LPARAM(&myConfig));
                    if (theme == dark)
                    {
                        DetourTransactionBegin();
                        DetourUpdateThread(GetCurrentThread());
                        DetourDetach(&(PVOID&)trueCreateWindowEx, myCreateWindowEx);
                        DetourTransactionCommit();
                    }
                    return retVal;
                }

                if(theme == light)
                    return DefSubclassProc(hWnd, uMsg, wParam, lParam);

                switch (uMsg)
                {
                case WM_ERASEBKGND:
                {
                    SetTextColor((HDC)wParam, white); // Color for SysLink, which must be set in its parent.

                    if (!painting)
                    {
                        RECT rect;
                        GetClientRect(hWnd, &rect);
                        FillRect((HDC)wParam, &rect, notSoDarkBrush);
                        return 1;
                    }

                    if (wndClass(hWnd) == WC_LINK)
                    {
                        return 1; // Avoid erasing the background for links, as they will blink white on the extender and during page switches.
                    }

                    break;
                }

                case WM_CTLCOLORDLG:
                    return (LRESULT)darkBrush; // Window background color when the extender resizes upward (Windows 10 only).
                }

                painting++;
                LPARAM retVal = DefSubclassProc(hWnd, uMsg, wParam, lParam);
                painting--;
                return retVal;
            }

            enum Reason {ThemeSwitch, NewPage};
            BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
            {
                Reason reason = (Reason)lParam;
                EnumChildWindows(hwnd, EnumChildProc, lParam);

                std::wstring className = wndClass(hwnd);

                if (className == WC_LINK)
                {
                    LITEM linkChanges = { LIF_ITEMINDEX | LIF_STATE, 0, theme==dark?LIS_DEFAULTCOLORS:0, LIS_DEFAULTCOLORS };
                    while (SendMessage(hwnd, LM_SETITEM, 0, (LPARAM)&linkChanges))
                    {
                        linkChanges.iLink++;
                    }
                }

                if(theme == light)
                    RemoveWindowSubclass(hwnd, Subclassproc, (UINT_PTR)hwnd);
                else
                {
                    // Ensure SetWindowTheme is not called for DirectUIHWND a second time,
                    // as it may cause sizing glitches on the second page.
                    if (GetWindowSubclass(hwnd, Subclassproc, (UINT_PTR)hwnd, NULL))
                        return TRUE;

                    SetWindowSubclass(hwnd, Subclassproc, (UINT_PTR)hwnd, 0); // Subclass all children to prevent white flashing on the first WM_ERASEBKGND.
                }
                
                if (className == WC_BUTTON || className == WC_SCROLLBAR)
                {
                    SetWindowTheme(hwnd, theme == dark ? L"DarkMode_Explorer" : NULL, NULL);
                }

                if (className == L"DirectUIHWND")
                {
                    if (theme == light && reason == NewPage)
                    {
                        return TRUE;
                    }

                    WINDOWPLACEMENT pos = {};
                    GetWindowPlacement(GetParent(hwnd), &pos);
                    SetWindowTheme(hwnd, theme == dark ? L"DarkMode_Explorer" : NULL, NULL);
                    SetWindowPlacement(GetParent(hwnd), &pos);
                }

                return TRUE;
            }

            void update(HWND hwnd, Reason reason)
            {
                BOOL value = theme==dark ? TRUE : FALSE;
                DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
                EnumChildWindows(hwnd, EnumChildProc, (LPARAM)reason);
            }

            void ensureDetoursSet(bool onCreate = false)
            {
                static bool isSetNow = false;
                bool mustBeSet = theme == dark && (onCreate || taskDialogs.size());
                if (isSetNow == mustBeSet)
                    return;

                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                if (mustBeSet)
                {
                    DetourAttach(&(PVOID&)trueGetThemeColor, myGetThemeColor);
                    DetourAttach(&(PVOID&)trueDrawThemeBackgroundEx, myDrawThemeBackgroundEx);
                    DetourAttach(&(PVOID&)trueDrawThemeText, myDrawThemeText);
                    DetourAttach(&(PVOID&)trueDrawThemeTextEx, myDrawThemeTextEx);
                    DetourAttach(&(PVOID&)trueDrawThemeBackground, myDrawThemeBackground);
                }
                else
                {
                    DetourDetach(&(PVOID&)trueGetThemeColor, myGetThemeColor);
                    DetourDetach(&(PVOID&)trueDrawThemeBackgroundEx, myDrawThemeBackgroundEx);
                    DetourDetach(&(PVOID&)trueDrawThemeText, myDrawThemeText);
                    DetourDetach(&(PVOID&)trueDrawThemeTextEx, myDrawThemeTextEx);
                    DetourDetach(&(PVOID&)trueDrawThemeBackground, myDrawThemeBackground);
                }
                DetourTransactionCommit();

                static ULONG_PTR gdiplusToken;
                static Gdiplus::GdiplusStartupInput gdiplusStartupInput;
                if (mustBeSet)
                    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
                else
                    Gdiplus::GdiplusShutdown(gdiplusToken);

                isSetNow = mustBeSet;
            }

            HRESULT CALLBACK taskdialogcallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData)
            {
                if (msg == TDN_DIALOG_CONSTRUCTED) // Called on each new page, including the first one.
                {
                    update(hwnd, NewPage);
                }
                if (msg == TDN_CREATED)
                {
                    taskDialogs.insert(hwnd);
                    SetWindowSubclass(hwnd, Subclassproc, (UINT_PTR)hwnd, lpRefData);
                }
                if (msg == TDN_DESTROYED)
                {
                    taskDialogs.erase(hwnd);
                }

                HRESULT resilt = S_OK;
                TaskDalogCallbackWrap* trueCallBackData = (TaskDalogCallbackWrap*)lpRefData;
                if (trueCallBackData->pfCallback)
                    resilt = trueCallBackData->pfCallback(hwnd, msg, wParam, lParam, trueCallBackData->lpCallbackData);

                return resilt;
            }


            HRESULT WINAPI myTaskDialogIndirect(const TASKDIALOGCONFIG* pTaskConfig, int* pnButton, int* pnRadioButton, BOOL* pfVerificationFlagChecked)
            {
                ensureDetoursSet(true);
                TaskDalogCallbackWrap callBackData = { pTaskConfig->pfCallback, pTaskConfig->lpCallbackData };
                TASKDIALOGCONFIG myConfig = *pTaskConfig;
                myConfig.lpCallbackData = (LONG_PTR)&callBackData;
                myConfig.pfCallback = taskdialogcallback;
                HRESULT result = trueTaskDialogIndirect(&myConfig, pnButton, pnRadioButton, pfVerificationFlagChecked);
                ensureDetoursSet();
                return result;
            }
        }

        void setTheme(Theme theme)
        {
            static bool initialized = false;
            if (!initialized)
            {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourAttach(&(PVOID&)detail::trueTaskDialogIndirect, detail::myTaskDialogIndirect);
                DetourTransactionCommit();
                initialized = true;
            }

            if (detail::theme == theme)
                return;

            detail::theme = theme;

            detail::ensureDetoursSet();

            for (HWND hwnd : detail::taskDialogs)
            {
                detail::update(hwnd, detail::ThemeSwitch);
            }
        }
    }
}