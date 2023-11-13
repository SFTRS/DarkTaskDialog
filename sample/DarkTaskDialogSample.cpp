//(c) Softros Systems


#include <Windows.h>
#include <Commctrl.h>
#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")

#include "..\src\DarkTaskDialog.hpp"

HRESULT CALLBACK taskdialogcallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData)
{
    if (msg == TDN_NAVIGATED)
    {
        /* page 2
        SendMessage(hwnd, TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE, 1, 1);
        SendMessage(hwnd, TDM_ENABLE_BUTTON, 1, 0);
        SendMessage(hwnd, TDM_ENABLE_RADIO_BUTTON, 1, 0);
        SendMessage(hwnd, TDM_SET_PROGRESS_BAR_MARQUEE, TRUE, 0);
        */
    }
    if (msg == TDN_CREATED)
    {
        SendMessage(hwnd, TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE, 2, 1);
        SendMessage(hwnd, TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE, 3, 1);
        SendMessage(hwnd, TDM_ENABLE_BUTTON, 1, 0);
        SendMessage(hwnd, TDM_ENABLE_BUTTON, 3, 0);
        SendMessage(hwnd, TDM_ENABLE_RADIO_BUTTON, 1, 0);
        SendMessage(hwnd, TDM_SET_PROGRESS_BAR_MARQUEE, TRUE, 0);
    }
    if (msg == TDN_HYPERLINK_CLICKED)
    {
        LPCWSTR link = (LPCWSTR)lParam;

        if (std::wstring(L"off") == link)
        {
            //DarkProgressBar::disable();
            SFTRS::DarkTaskDialog::disable();
        }

        if (std::wstring(L"on") == link)
        {
            //DarkProgressBar::enable();
            DarkTaskDialog::enable();
        }

        if (std::wstring(L"page2") == link)
        {
            TASKDIALOGCONFIG page2;
            page2.cbSize = sizeof(TASKDIALOGCONFIG);
            page2.dwCommonButtons = TDCBF_CLOSE_BUTTON;
            page2.pszWindowTitle = L"page #2";
            page2.pszMainIcon = MAKEINTRESOURCE(-8);
            page2.pszMainInstruction = L"TaskDialog Page #2";
            page2.pszContent = L"This is mostly empty page 2 of sample TaskDialog";
            page2.pfCallback = taskdialogcallback;
            SendMessage(hwnd, TDM_NAVIGATE_PAGE, NULL, (LONG_PTR)&page2);
        }
    }

    return S_OK;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    //DarkProgressBar::enable();
    DarkTaskDialog::enable();

    const TASKDIALOG_BUTTON radio[] = { {0, L"Radio button\nwith two lines"},
                                    {1, L"Second button is disabled"},
                                    {2, L"Just a normal dario button"} };

    const TASKDIALOG_BUTTON commands[] = { {0, L"Command button\nwith two lines"},
                                {1, L"Disabled command button"},
                                {2, L"Command button with shield"},
                                {3, L"Disabled command button with shield"},
                                {4, L"Normal command button"} };

    TASKDIALOGCONFIG tdc = {};
    tdc.cbSize = sizeof(TASKDIALOGCONFIG);

    tdc.dwFlags = TDF_ENABLE_HYPERLINKS
        | TDF_USE_COMMAND_LINKS | TDF_EXPAND_FOOTER_AREA | TDF_SHOW_MARQUEE_PROGRESS_BAR;
    tdc.dwCommonButtons = TDCBF_OK_BUTTON | TDCBF_CLOSE_BUTTON;
    tdc.pszWindowTitle = L"Window Title";
    tdc.pszMainIcon = TD_INFORMATION_ICON;
    tdc.pszMainInstruction = L"TaskDialog example";
    tdc.pszContent = L"This task dailog sample shows most of the controls task dialog can contian\n\
You can <A HREF=\"off\">switch dark mode off</A> for progress bar only.\n\
<A HREF=\"on\">Switchng on</A> is also possible; but only if it was already on earlier\n\
TaskDailog is usually a short living window; so changing theme on the fly is not possible. At least now.\n\
Where is also <A HREF=\"page2\">another page</A> added for testing purposes";
    tdc.cButtons = 4;
    tdc.pButtons = commands;
    tdc.nDefaultButton = TDCBF_CLOSE_BUTTON;
    tdc.cRadioButtons = 3;
    tdc.pRadioButtons = radio;
    tdc.pszVerificationText = L"TaskDialog verification text";
    tdc.pszExpandedInformation = L"TaskDialog extended information";
    tdc.pszExpandedControlText = L"TaskDialog expanded control text";
    tdc.pszCollapsedControlText = L"TaskDialog collapsed control text";
    tdc.pszFooterIcon = TD_SHIELD_ICON;
    tdc.pszFooter = L"TaskDialog footer area <A HREF=\"page2\">with a link to page #2</A>.";
    tdc.pfCallback = taskdialogcallback;

    BOOL checked = FALSE;
    HRESULT res = TaskDialogIndirect(&tdc, NULL, NULL, &checked);

    Sleep(1);

    //DarkProgressBar::disable();
    DarkTaskDialog::disable();

}