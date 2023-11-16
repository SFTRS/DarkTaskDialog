// © Softros Systems
// GPL3
// github.com/SFTRS/DarkTaskDialog


#include <Windows.h>
#include <Commctrl.h>
#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")

#include "..\src\DarkTaskDialog.hpp"

static TASKDIALOGCONFIG page1 = {};
static TASKDIALOGCONFIG page2 = {};

HRESULT CALLBACK taskdialogcallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData)
{
    if (msg == TDN_CREATED || msg == TDN_NAVIGATED && lpRefData==1)
    {
        SendMessage(hwnd, TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE, 12, 1);
        SendMessage(hwnd, TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE, 13, 1);
        SendMessage(hwnd, TDM_ENABLE_BUTTON, 11, 0);
        SendMessage(hwnd, TDM_ENABLE_BUTTON, 13, 0);
        SendMessage(hwnd, TDM_ENABLE_RADIO_BUTTON, 11, 0);
        SendMessage(hwnd, TDM_SET_PROGRESS_BAR_MARQUEE, TRUE, 0);
    }
    if (msg == TDN_HYPERLINK_CLICKED)
    {
        LPCWSTR link = (LPCWSTR)lParam;
        if (std::wstring(L"off") == link)
        {
            SFTRS::DarkTaskDialog::setTheme(SFTRS::DarkTaskDialog::light);
        }
        if (std::wstring(L"on") == link)
        {
            SFTRS::DarkTaskDialog::setTheme(SFTRS::DarkTaskDialog::dark);
        }
        if (std::wstring(L"page2") == link)
        {
            SendMessage(hwnd, TDM_NAVIGATE_PAGE, NULL, (LONG_PTR)&page2);
        }
    }
    if (msg == TDN_BUTTON_CLICKED)
    {
        if (wParam == 10)
            SendMessage(hwnd, TDM_NAVIGATE_PAGE, NULL, (LONG_PTR)&page1);
        return (wParam == IDCLOSE) ? S_OK : S_FALSE;
    }

    return S_OK;
}

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    SFTRS::DarkTaskDialog::setTheme(SFTRS::DarkTaskDialog::dark);


    // PAGE 1
    const TASKDIALOG_BUTTON radio[] = { {10, L"Radio button\nwith two lines"},
                                        {11, L"The second radio button is disabled"},
                                        {12, L"Just a normal radio button"} };

    const TASKDIALOG_BUTTON commands[] = { {10, L"Command button\nwith two lines"},
                                            {11, L"Disabled command button"},
                                            {12, L"Command button with shield"},
                                            {13, L"Disabled command button with shield"},
                                            {14, L"Normal command button"} };
    page1.cbSize = sizeof(TASKDIALOGCONFIG);
    page1.dwFlags = TDF_ENABLE_HYPERLINKS | TDF_USE_COMMAND_LINKS | TDF_EXPAND_FOOTER_AREA | TDF_SHOW_MARQUEE_PROGRESS_BAR;
    page1.dwCommonButtons = TDCBF_CLOSE_BUTTON;
    page1.pszWindowTitle = L"Dark Task Dialog Sample";
    page1.pszMainIcon = TD_INFORMATION_ICON;
    page1.pszMainInstruction = L"SFTRS::DarkTaskDialog";
    page1.pszContent = L"This task dialog sample showcases most of the controls a task dialog can contain.\n\
You can dynamically control the theme here using the following links : \n\n\
<A HREF=\"off\">Switch to light theme</A>\n\<A HREF=\"on\">Switch to dark theme</A>\n\n\
Additionally, there is <A HREF=\"page2\">another page</A> added for testing purposes.\n\n\
Other controls are available below.";
    page1.cButtons = 5;
    page1.pButtons = commands;
    page1.nDefaultButton = TDCBF_CLOSE_BUTTON;
    page1.cRadioButtons = 3;
    page1.pRadioButtons = radio;
    page1.pszVerificationText = L"Verification text";
    page1.pszExpandedInformation = L"Expanded information.\n\
Can be multiline and contain links. For example:\n\
<A HREF=\"off\">Switch to light theme</A>\n\
<A HREF=\"on\">Switch to dark theme</A>";
    page1.pszExpandedControlText = L"Hide expanded information";
    page1.pszCollapsedControlText =  L"Show expanded information";
    page1.pszFooterIcon = TD_SHIELD_ICON;
    page1.pszFooter = L"This is the footer area.\n\
It can also be multiline and contain links. For example, here is the link to <A HREF=\"page2\">page #2</A>.";
    page1.pfCallback = taskdialogcallback;
    page1.lpCallbackData = 1;


    // PAGE 2
    const TASKDIALOG_BUTTON commandsP2[] = { {10, L"Return to first page"} };
    page2.cbSize = sizeof(TASKDIALOGCONFIG);
    page2.pszWindowTitle = L"page #2";
    page2.pszMainIcon = MAKEINTRESOURCE(-8);
    page2.pszMainInstruction = L"TaskDialog Page #2";
    page2.pszContent = L"This is mostly empty page 2 of sample TaskDialog";
    page2.cButtons = 1;
    page2.pButtons = commandsP2;
    page2.pfCallback = taskdialogcallback;


    BOOL checked = FALSE;
    TaskDialogIndirect(&page1, NULL, NULL, &checked);

}