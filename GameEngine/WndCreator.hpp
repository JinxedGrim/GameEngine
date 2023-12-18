#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <commctrl.h>

#if __cplusplus < 201103L
    #ifndef _MSC_VER
        #error "C++11 or a later version is required for std::shared_ptr"
    #endif
#endif

#ifdef _MSC_VER
__pragma(warning(disable:6387))
#pragma comment(lib, "gdi32.lib")
#endif

#define SafeReleaseDC(Wnd, DC)  if((DC != NULL && DC != INVALID_HANDLE_VALUE) && Wnd != NULL) { ReleaseDC(Wnd, DC); }
#define SafeDeleteDC(DC)  if(DC != NULL && DC != INVALID_HANDLE_VALUE) { DeleteDC(DC); }
#define SafeDeleteObject(Obj)  if(Obj != NULL && Obj != INVALID_HANDLE_VALUE) { DeleteObject(Obj); }
#define SafeDestroyWindow(Wnd)  if(Wnd != NULL) { DestroyWindow(Wnd); }
#define SafeDeleteIcon(Ico)  if(Ico != NULL && Ico != INVALID_HANDLE_VALUE) { DestroyIcon(Ico); }
#define SafeDeleteFont(Font)  if(Font != NULL && Font != INVALID_HANDLE_VALUE) {  }


#ifdef _MSC_VER
__pragma(warning(default:6387))
#endif

#ifdef UNICODE
#define WndCreator WndCreatorW
#else
#define WndCreator WndCreatorA
#endif // UNICODE

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define CurrHinst ((HINSTANCE)&__ImageBase)

void(__fastcall * WndCtrlEvntProcessor)() = nullptr;

enum class WndExModes : DWORD
{
    FullScreenEx = WS_EX_TOPMOST,
    TOPMOST = WS_EX_TOPMOST,
    BorderLessEx = 0,
    WindowedEx = 0,
    ChildEx = 0,
    Layered = WS_EX_LAYERED,
    NoTaskBarAndSmallBorder = WS_EX_TOOLWINDOW,

};

inline WndExModes operator | (WndExModes lhs, DWORD rhs)
{
    return (WndExModes)((DWORD)lhs | rhs);
}


inline WndExModes operator | (WndExModes lhs, WndExModes rhs)
{
    return (WndExModes)((DWORD)lhs | (DWORD)rhs);
}


enum class WndModes : DWORD
{
    FullScreen = WS_POPUP | WS_VISIBLE,
    BorderLess = WS_POPUP | WS_VISIBLE,
    Windowed = WS_OVERLAPPED | WS_THICKFRAME | WS_VISIBLE | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MAXIMIZE,
    Child =  WS_CHILD | WS_VISIBLE,
    ClipChildren = WS_CLIPCHILDREN
};


inline WndModes operator | (WndModes lhs, DWORD rhs)
{
    return (WndModes)((DWORD)lhs | rhs);
}


inline WndModes operator | (WndModes lhs, WndModes rhs)
{
    return (WndModes)((DWORD)lhs | (DWORD)rhs);
}


enum class ButtonTypes : DWORD
{

    AutoRadio = BS_AUTORADIOBUTTON,
    PushButton = BS_PUSHBUTTON,
    Checkbox = BS_CHECKBOX,
    AutoCheck = BS_AUTOCHECKBOX,
    ThreeState = BS_AUTO3STATE,
};


enum class ButtonStyles : DWORD
{
    Flat = BS_FLAT,
    Default = BS_DEFPUSHBUTTON,
    OwnerDraw = BS_OWNERDRAW,
    LeftText = BS_LEFTTEXT,
    Text = BS_TEXT,
};


inline ButtonStyles operator | (ButtonStyles lhs, DWORD rhs)
{
    return (ButtonStyles)((DWORD)lhs | rhs);
}


inline ButtonStyles operator | (ButtonStyles lhs, ButtonStyles rhs)
{
    return (ButtonStyles)((DWORD)lhs | (DWORD)rhs);
}

enum class RadioButtonStyles : DWORD
{
    AutoRadio = BS_AUTORADIOBUTTON,
    Flat = BS_FLAT,
    OwnerDraw = BS_OWNERDRAW,
    LeftText = BS_LEFTTEXT,
    Text = BS_TEXT,
};


inline RadioButtonStyles operator | (RadioButtonStyles lhs, DWORD rhs)
{
    return (RadioButtonStyles)((DWORD)lhs | rhs);
}


inline RadioButtonStyles operator | (RadioButtonStyles lhs, RadioButtonStyles rhs)
{
    return (RadioButtonStyles)((DWORD)lhs | (DWORD)rhs);
}

enum class CheckboxStyles : DWORD
{
    AutoCheck = BS_AUTOCHECKBOX,
    Flat = BS_FLAT,
    OwnerDraw = BS_OWNERDRAW,
    LeftText = BS_LEFTTEXT,
    Text = BS_TEXT,
};


inline CheckboxStyles operator | (CheckboxStyles lhs, DWORD rhs)
{
    return (CheckboxStyles)((DWORD)lhs | rhs);
}


inline CheckboxStyles operator | (CheckboxStyles lhs, CheckboxStyles rhs)
{
    return (CheckboxStyles)((DWORD)lhs | (DWORD)rhs);
}


enum class EditControlStyles : DWORD
{
    AutoHScroll = ES_AUTOHSCROLL,
    AutoVScroll = ES_AUTOVSCROLL,
    Left = ES_LEFT,
    Multiline = ES_MULTILINE,
    NoHideSelection = ES_NOHIDESEL,
    Number = ES_NUMBER,
    Password = ES_PASSWORD,
    Readonly = ES_READONLY,
    Right = ES_RIGHT,
    UpperCase = ES_UPPERCASE,
};


inline EditControlStyles operator | (EditControlStyles lhs, DWORD rhs)
{
    return (EditControlStyles)((DWORD)lhs | rhs);
}


inline EditControlStyles operator | (EditControlStyles lhs, EditControlStyles rhs)
{
    return (EditControlStyles)((DWORD)lhs | (DWORD)rhs);
}


enum class SliderStyles : DWORD
{
    AutoTicks = TBS_AUTOTICKS,
    Vertical = TBS_VERT,
    Horizontal = TBS_HORZ,
    EnableSelRange = TBS_ENABLESELRANGE,
    FixedLength = TBS_FIXEDLENGTH,
    NoThumb = TBS_NOTHUMB,
    NoTicks = TBS_NOTICKS,
    ToolTip = TBS_TOOLTIPS,
    Top = TBS_TOP,
    Bottom = TBS_BOTTOM,
    Left = TBS_LEFT,
    Right = TBS_RIGHT,
    Reversed = TBS_REVERSED,
};


inline SliderStyles operator | (SliderStyles lhs, DWORD rhs)
{
    return (SliderStyles)((DWORD)lhs | rhs);
}


inline SliderStyles operator | (SliderStyles lhs, SliderStyles rhs)
{
    return (SliderStyles)((DWORD)lhs | (DWORD)rhs);
}


enum class LabelStyles : DWORD
{
    Left = SS_LEFT,
    Center = SS_CENTER,
    Right = SS_RIGHT,
    Icon = SS_ICON,
    Bitmap = SS_BITMAP,
    OwnerDraw = SS_OWNERDRAW,
    EditControl = SS_EDITCONTROL,
    Notify = SS_NOTIFY,
    NoPrefix = SS_NOPREFIX,
    CenterImage = SS_CENTERIMAGE,
    RealSizeImage = SS_REALSIZEIMAGE,
    Sunken = SS_SUNKEN,
    EtchedHorizontal = SS_ETCHEDHORZ,
    EtchedVertical = SS_ETCHEDVERT,
    EtchedFrame = SS_ETCHEDFRAME,
    GrayRect = SS_GRAYRECT,
    BlackRect = SS_BLACKRECT,
    WhiteRect = SS_WHITERECT,
    GrayFrame = SS_GRAYFRAME,
    BlackFrame = SS_BLACKFRAME,
    UserItem = SS_USERITEM
};

inline LabelStyles operator|(LabelStyles lhs, DWORD rhs)
{
    return static_cast<LabelStyles>(static_cast<DWORD>(lhs) | rhs);
}

inline LabelStyles operator|(LabelStyles lhs, LabelStyles rhs)
{
    return static_cast<LabelStyles>(static_cast<DWORD>(lhs) | static_cast<DWORD>(rhs));
}


struct ScreenDimensions
{
    LONG Width, Height;
};

class WndIconW
{
private:
    std::shared_ptr<HICON> Ico = nullptr;
public:
    HINSTANCE Hinst = 0;
    LPCWSTR IconName = nullptr;

    WndIconW(const LPCWSTR IconName, const HINSTANCE Inst = NULL)
    {
        this->IconName = IconName;
        this->Hinst = Inst;
        this->Ico = std::make_shared<HICON>(LoadIconW(Inst, IconName));

        if (!this->Ico)
        {
            // TODO Log error
            throw 1;
        }
    }

    WndIconW(const int ResourceID, const HINSTANCE Inst = NULL)
    {
        this->IconName = MAKEINTRESOURCEW(ResourceID);
        this->Hinst = Inst;
        this->Ico = std::make_shared<HICON>(LoadIconW(Inst, MAKEINTRESOURCEW(ResourceID)));

        if (!this->Ico || !*this->Ico)
        {
            // TODO Log error
            throw 1;
        }
    }

    ~WndIconW()
    {
        SafeDeleteIcon(*this->Ico);
    }

    HICON GetHICON()
    {
        return *this->Ico;
    }

    std::shared_ptr<HICON> GetSharedPointer()
    {
        return this->Ico;
    }
};

class WndIconA
{
    private:
    std::shared_ptr<HICON> Ico = nullptr;
    public:
    HINSTANCE Hinst = 0;
    LPCSTR IconName = nullptr;

    WndIconA(const LPCSTR IconName, const HINSTANCE Inst = NULL)
    {
        this->IconName = IconName;
        this->Hinst = Inst;
        this->Ico = std::make_shared<HICON>(LoadIconA(Inst, IconName));

        if (!this->Ico)
        {
            // TODO Log error
            throw 1;
        }
    }

    WndIconA(const int ResourceID, const HINSTANCE Inst = NULL)
    {
        this->IconName = MAKEINTRESOURCEA(ResourceID);
        this->Hinst = Inst;
        this->Ico = std::make_shared<HICON>(LoadIconA(Inst, MAKEINTRESOURCEA(ResourceID)));

        if (!this->Ico || !*this->Ico)
        {
            // TODO Log error
            throw 1;
        }
    }

    ~WndIconA()
    {
        SafeDeleteIcon(*this->Ico);
    }

    HICON GetHICON()
    {
        return *this->Ico;
    }

    std::shared_ptr<HICON> GetSharedPointer()
    {
        return this->Ico;
    }
};

class WndCreatorA
{
    public:
    WndCreatorA()
    {
        if (!DidCreate)
        {
            wc = {};
            DidCreate = false;
            Wnd = NULL;
            ErrorHandler = LogError;
        }
        else
        {
            this->~WndCreatorA();
        }
    }

    std::function<void(int, int)> CommandCb = nullptr;

    static LRESULT CALLBACK WindowProcA(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        WndCreatorA* Instance = (WndCreatorA*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

        switch (uMsg)
        {
            case WM_COMMAND:
            {
                if (Instance != nullptr && Instance->CommandCb != nullptr)
                {
                    Instance->CommandCb(lParam, wParam);
                }
                else
                {
                    DefWindowProcA(hwnd, uMsg, wParam, lParam);
                }

                break;
            }
            case WM_DESTROY: // called when DestroyWindow is called
            {
                PostQuitMessage(0);
                break; // calls DefWindowProcW: which will call WM_QUIT
            }
            case WM_CLOSE: // called when user clicks x button or alt f4
            {
                PostQuitMessage(0);
                break; // calls DefWindowProcW: which will call Destroy Window 
            }
            case WM_QUIT: // closes window
            {
                PostQuitMessage(0);
                break;
            }
        }
        return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }

    WndCreatorA(const UINT ClassStyle, const std::string_view ClassName, const std::string_view WindowName, const HCURSOR Curs, const HICON Ico, const HBRUSH BackGround, const DWORD ExFlags, const DWORD WStyle, const int x, const int y, const int Width, const int Height, HWND HwndParent = 0, const HINSTANCE hInstance = GetModuleHandleW(NULL))
    {
        SecureZeroMemory(&wc, sizeof(WNDCLASSEXA));

        wc = { sizeof(WNDCLASSEXA), ClassStyle, this->WindowProcA, 0L, 0L, hInstance, Ico, Curs, BackGround, NULL, ClassName.data(), NULL };

        if (!RegisterClassExA(&wc))
        {
            if (GetLastError() != 1410)
            {
                ErrorHandler("[WndCreator] Failed To Register Window Class: " + std::to_string(GetLastError()));
            }
            return;
        }

        this->Wnd = CreateWindowExA(ExFlags, ClassName.data(), WindowName.data(), WStyle, x, y, Width, Height, HwndParent, 0, hInstance, 0);

        this->SetWndLong(GWLP_USERDATA, (long)this);

        if (!this->Wnd)
        {
            SafeDeleteObject(wc.hbrBackground);
            UnregisterClassA(wc.lpszClassName, wc.hInstance);
            ErrorHandler("[WndCreator] Failed To Create Window: " + std::to_string(GetLastError()));
            return;
        }

        this->DidCreate = true;
    }

    WndCreatorA(const HWND Wnd)
    {
        this->Wnd = Wnd;

        this->SetWndLong(GWLP_USERDATA, (long)this);

        this->DidCreate = false;
    }

    ~WndCreatorA()
    {
        if (this->DidCreate)
        {
            SafeDestroyWindow(Wnd);
            SafeDeleteObject(wc.hbrBackground);
            UnregisterClassA(wc.lpszClassName, wc.hInstance);
        }
    }

    static void LogError(std::string ErrorMsg)
    {
        MessageBoxA(NULL, ErrorMsg.c_str(), "", MB_OK);
    }

    // Copying is not allowed
    WndCreatorA(const WndCreatorA&) = default;
    WndCreatorA& operator=(const WndCreatorA&) = default;

    // Moving is allowed
    WndCreatorA(WndCreatorA&& Rhs) noexcept
    {
        this->wc = Rhs.wc;
        this->DidCreate = Rhs.DidCreate;
        this->Wnd = Rhs.Wnd;
        this->ErrorHandler = Rhs.ErrorHandler;
        this->Children = Rhs.Children;

        Rhs.Wnd = NULL;
    }

    WndCreatorA& operator=(WndCreatorA&& Rhs) noexcept
    {
        this->wc = Rhs.wc;
        this->DidCreate = Rhs.DidCreate;
        this->Wnd = Rhs.Wnd;
        this->ErrorHandler = Rhs.ErrorHandler;
        this->Children = Rhs.Children;

        Rhs.Wnd = NULL;

        return *this;
    }

    private:
    WNDCLASSEXA wc = {};
    bool DidCreate = false;
    public:
    HWND Wnd = NULL;
    void(*ErrorHandler)(std::string) = LogError;
    std::vector<HWND> Children = {};


    const bool Hide()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_HIDE);
    }


    const bool Show()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_SHOW);
    }


    const bool Maximize()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_MAXIMIZE);
    }


    const bool Minimize()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_MINIMIZE);
    }


    const bool AddStyleFlags(const LONG_PTR StyleToAdd)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrA(this->Wnd, GWL_STYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Hide();
        Style |= StyleToAdd;
        if (!SetWindowLongPtrA(this->Wnd, GWL_STYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }
        Show();

        return true;
    }


    const bool AddStyleFlagsEx(const LONG_PTR ExStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrA(this->Wnd, GWL_EXSTYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Hide();

        Style |= ExStyle;

        if (!SetWindowLongPtrA(this->Wnd, GWL_EXSTYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Show();

        return true;
    }


    const bool SubStyleFlags(const LONG_PTR StyleToSub)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrA(this->Wnd, GWL_STYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Style &= ~(StyleToSub);

        Hide();

        if (!SetWindowLongPtrA(this->Wnd, GWL_STYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Show();

        return true;
    }


    const bool SubStyleFlagsEx(const LONG_PTR ExStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrA(this->Wnd, GWL_EXSTYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Style &= ~(ExStyle);

        this->Hide();
        if (!SetWindowLongPtrA(this->Wnd, GWL_EXSTYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }
        this->Show();

        return true;
    }


    const bool ResetStyle(const LONG_PTR NewStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        this->Hide();
        SetLastError(0);
        if (!SetWindowLongPtrA(this->Wnd, GWL_STYLE, NewStyle) && GetLastError() != 0)
        {
            std::string err = std::to_string(GetLastError());
            ErrorHandler(std::string("[WndCreator] Error Setting Window Ex Style: (" + err + ")"));
            return false;
        }
        this->Show();

        return true;
    }


    const bool ResetStyleEx(const LONG_PTR NewExStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        this->Hide();
        SetLastError(0);
        if (!SetWindowLongPtrA(this->Wnd, GWL_EXSTYLE, NewExStyle) && GetLastError() != 0)
        {
            std::string err = std::to_string(GetLastError());
            ErrorHandler(std::string("[WndCreator] Error Setting Window Ex Style: (" + err + ")"));
            return false;
        }
        this->Show();

        return true;
    }


    const bool SetWndSz(const HWND WndZPos, const int X, const int Y, const int Width, const int Height, const UINT SwFlags)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        if (!SetWindowPos(this->Wnd, WndZPos, X, Y, Width, Height, SwFlags))
        {
            ErrorHandler("[WndCreator] Error with SetWindowPos: " + std::to_string(GetLastError()));
            return false;
        }

        return true;
    }


    const bool SetWndTitle(const std::string& Str)
    {
        return SetWindowTextA(this->Wnd, Str.c_str());
    }


    const ScreenDimensions GetClientArea()
    {
        RECT rect;
        if (GetClientRect(this->Wnd, &rect))
        {
            return{ rect.right, rect.bottom };
        }
        else
        {
            return { 0, 0 };
        }
    }


    const std::string GetWndTitle()
    {
        char Buffer[1024];
        GetWindowTextA(this->Wnd, Buffer, 1024);

        std::string Str = Buffer;
        return Str;
    }


    const bool HasFocus()
    {
        if (GetFocus() == this->Wnd)
        {
            return true;
        }

        return false;
    }


    const bool SetLayeredAttributes(const COLORREF CrKey, const BYTE Alpha, const DWORD DwFlags = LWA_COLORKEY)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        if (!SetLayeredWindowAttributes(Wnd, CrKey, Alpha, DwFlags))
        {
            ErrorHandler("[WndCreator] Failed to set layered attributes: " + std::to_string(GetLastError()));
            return false;
        }

        return true;
    }


    const void SetWndLong(int Index, LONG_PTR Value)
    {
        SetWindowLongPtrA(this->Wnd, Index, Value);
    }


    const LRESULT SendWndMessage(const UINT MSG, WPARAM WParam, LPARAM LParam)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return -1;
        }

        return SendMessageA(this->Wnd, MSG, WParam, LParam);
    }


    HWND CreateChildWindow(const DWORD ExFlags, const DWORD WStyle, const std::string_view ClassName, const std::string_view WndName, const int x, const int y, const int Width, const int Height, const int Hmenu)
    {
        HWND chwnd = CreateWindowExA(ExFlags, ClassName.data(), WndName.data(), WStyle, x, y, Width, Height, this->Wnd, (HMENU)Hmenu, (HINSTANCE)GetWindowLongPtrA(this->Wnd, GWLP_HINSTANCE), NULL);

        this->Children.push_back(chwnd);

        return chwnd;
    }


    void UpdateChildren()
    {
        for (HWND W : this->Children)
        {
            UpdateWindow(W);
        }
    }


    const void Destroy()
    {
        SafeDestroyWindow(this->Wnd);
    }
};

class WndCreatorW
{
    private:
    WNDCLASSEXW wc = {};
    bool DidCreate = false;
    

    public:
    HWND Wnd = NULL;
    void(*ErrorHandler)(std::string) = LogError;
    std::vector<HWND> Children = {};

    // return value for callbacks represents wether the message was handled or not
    std::function<bool(HWND, int, int)> CommandCb = nullptr;
    std::function<bool(HWND, int, int)> ControlScrollCb = nullptr;
    std::function<bool(HWND, int, int)> InputCb = nullptr;
    std::function<bool(HWND, WPARAM, LPARAM)> PaintCb = nullptr;
    std::function<bool(HWND, UINT, WPARAM, LPARAM, HBRUSH*)> ColorCtlCb = nullptr;
    std::function<bool(HWND, UINT, WPARAM, LPARAM)> OtherMessageCb = nullptr;

    LONG_PTR OriginalProc = NULL;

    WndCreatorW()
    {
        if (!DidCreate)
        {
            wc = {};
            DidCreate = false;
            Wnd = NULL;
            ErrorHandler = LogError;
        }
        else
        {
            this->~WndCreatorW();
        }
    }

    static LRESULT CALLBACK WindowProcW(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        WndCreatorW* Instance = (WndCreatorW*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

        switch (uMsg)
        {
            case WM_COMMAND:
            {
                if (Instance != nullptr && Instance->CommandCb != nullptr)
                {
                    if (Instance->CommandCb((HWND)lParam, LOWORD(wParam), HIWORD(wParam)))
                    {
                        return 0;
                    }
                }

                break;
            }
            case WM_PAINT:
            {
                if (Instance != nullptr && Instance->PaintCb != nullptr)
                {
                    if (Instance->PaintCb(hwnd, wParam, lParam))
                    {
                        return 0;
                    }
                }

                break;
            }
            case WM_HSCROLL:
            {
                if ((HWND)lParam != NULL && Instance->ControlScrollCb != nullptr)
                {
                    if (Instance->ControlScrollCb((HWND)lParam, LOWORD(wParam), HIWORD(wParam)))
                    {
                        return 0;
                    }
                }
                break;
            }
            case WM_VSCROLL:
            {
                if ((HWND)lParam != NULL && Instance->ControlScrollCb != nullptr)
                {
                    if (Instance->ControlScrollCb((HWND)lParam, LOWORD(wParam), HIWORD(wParam)))
                    {
                        return 0;
                    }

                }
                break;
            }
            case WM_CTLCOLOREDIT:
            case WM_CTLCOLORDLG:
            case WM_CTLCOLORLISTBOX:
            case WM_CTLCOLORMSGBOX:
            case WM_CTLCOLORSCROLLBAR:
            case WM_CTLCOLORSTATIC:
            case WM_CTLCOLORBTN:
            {
                if (Instance->ColorCtlCb != nullptr)
                {
                    HBRUSH newBrush = NULL;
                    if (Instance->ColorCtlCb(hwnd, uMsg, wParam, lParam, &newBrush))
                    {
                        return (LRESULT)newBrush;
                    }
                }

                break;
            }
            case WM_DESTROY: // called when DestroyWindow is called
            {
                PostQuitMessage(0);
                break; // calls DefWindowProcW: which will call WM_QUIT
            }
            case WM_CLOSE: // called when user clicks x button or alt f4
            {
                PostQuitMessage(0);
                break; // calls DefWindowProcW: which will call Destroy Window 
            }
            case WM_QUIT: // closes window
            {
                PostQuitMessage(0);
                break;
            }
            default:
                if (Instance != nullptr && Instance->OtherMessageCb != nullptr)
                {
                    if (Instance->OtherMessageCb(hwnd, uMsg, wParam, lParam))
                    {
                        return 0;
                    }
                }
                break;
        }

        if (Instance == nullptr || Instance->OriginalProc == NULL)
        {
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }
        else
        {
            return CallWindowProcW((WNDPROC)Instance->OriginalProc, hwnd, uMsg, wParam, lParam);
        }
    }

    WndCreatorW(const UINT ClassStyle, const std::wstring_view ClassName, const std::wstring_view WindowName, const HCURSOR Curs, const HICON Ico, const HBRUSH BackGround, const DWORD ExFlags, const DWORD WStyle, const int x, const int y, const int Width, const int Height, HWND HwndParent = 0, const HINSTANCE hInstance = GetModuleHandleW(NULL))
    {
        SecureZeroMemory(&wc, sizeof(WNDCLASSEXW));

        wc = { sizeof(WNDCLASSEXW), ClassStyle, this->WindowProcW, 0L, 0L, hInstance, Ico, Curs, BackGround, NULL, ClassName.data(), NULL };

        if (!RegisterClassExW(&wc))
        {
            if (GetLastError() != 1410)
            {
                ErrorHandler("[WndCreator] Failed To Register Window Class: " + std::to_string(GetLastError()));
            }
            return;
        }

        this->Wnd = CreateWindowExW(ExFlags, ClassName.data(), WindowName.data(), WStyle, x, y, Width, Height, HwndParent, 0, hInstance, 0);

        this->SetWndLong(GWLP_USERDATA, (LONG_PTR)this);

        if (!this->Wnd)
        {
            SafeDeleteObject(wc.hbrBackground);
            UnregisterClassW(wc.lpszClassName, wc.hInstance);
            ErrorHandler("[WndCreator] Failed To Create Window: " + std::to_string(GetLastError()));
            return;
        }

        this->DidCreate = true;
    }

    WndCreatorW(const HWND Wnd)
    {
        this->Wnd = Wnd;

        this->DidCreate = false;
    }

    void OverrideWndProc()
    {
        this->OriginalProc = this->SetWndLong(GWLP_WNDPROC, (LONG_PTR)this->WindowProcW);
        this->SetWndLong(GWLP_USERDATA, (LONG_PTR)this);
    }

    void SubclassProc(std::function<bool(HWND, UINT, WPARAM, LPARAM)> newProc)
    {
        this->OriginalProc = this->SetWndLong(GWLP_WNDPROC, (LONG_PTR)&newProc);
    }

    ~WndCreatorW()
    {
        if (this->DidCreate)
        {
            SafeDestroyWindow(Wnd);
            SafeDeleteObject(wc.hbrBackground);
            UnregisterClassW(wc.lpszClassName, wc.hInstance);
        }
    }

    static void LogError(std::string ErrorMsg)
    {
        MessageBoxA(NULL, ErrorMsg.c_str(), "", MB_OK);
    }

    // Copying is not allowed
    WndCreatorW(const WndCreatorW&) = default;
    WndCreatorW& operator=(const WndCreatorW&) = default;

    // Moving is allowed
    WndCreatorW(WndCreatorW&& Rhs) noexcept
    {
        this->wc = Rhs.wc;
        this->DidCreate = Rhs.DidCreate;
        this->Wnd = Rhs.Wnd;
        this->ErrorHandler = Rhs.ErrorHandler;
        this->Children = Rhs.Children;

        Rhs.Wnd = NULL;
    }

    WndCreatorW& operator=(WndCreatorW&& Rhs) noexcept
    {
        this->wc = Rhs.wc;
        this->DidCreate = Rhs.DidCreate;
        this->Wnd = Rhs.Wnd;
        this->ErrorHandler = Rhs.ErrorHandler;
        this->Children = Rhs.Children;

        Rhs.Wnd = NULL;

        return *this;
    }

    
    const bool Hide()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_HIDE);
    }

    
    const bool Show()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_SHOW);
    }

    
    const bool Maximize()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_MAXIMIZE);
    }

    
    const bool Minimize()
    {
        if (!this->Wnd)
        {
            return false;
        }

        return ShowWindow(this->Wnd, SW_MINIMIZE);
    }

    
    const bool AddStyleFlags(const LONG_PTR StyleToAdd)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrW(this->Wnd, GWL_STYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Hide();
        Style |= StyleToAdd;
        if (!SetWindowLongPtrW(this->Wnd, GWL_STYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }
        Show();

        return true;
    }

    
    const bool AddStyleFlagsEx(const LONG_PTR ExStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrW(this->Wnd, GWL_EXSTYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Hide();

        Style |= ExStyle;

        if (!SetWindowLongPtrW(this->Wnd, GWL_EXSTYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Show();

        return true;
    }

    
    const bool SubStyleFlags(const LONG_PTR StyleToSub)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrW(this->Wnd, GWL_STYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Style &= ~(StyleToSub);

        Hide();

        if (!SetWindowLongPtrW(this->Wnd, GWL_STYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Show();

        return true;
    }

    
    const bool SubStyleFlagsEx(const LONG_PTR ExStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        LONG_PTR Style = GetWindowLongPtrW(this->Wnd, GWL_EXSTYLE);

        if (!Style)
        {
            ErrorHandler("[WndCreator] Error Retrieving Window Style: " + std::to_string(GetLastError()));
            return false;
        }

        Style &= ~(ExStyle);

        this->Hide();
        if (!SetWindowLongPtrW(this->Wnd, GWL_EXSTYLE, Style))
        {
            ErrorHandler("[WndCreator] Error Setting Window Style: " + std::to_string(GetLastError()));
            return false;
        }
        this->Show();

        return true;
    }

    
    const bool ResetStyle(const LONG_PTR NewStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        this->Hide();
        SetLastError(0);
        if (!SetWindowLongPtrW(this->Wnd, GWL_STYLE, NewStyle) && GetLastError() != 0)
        {
            std::string err = std::to_string(GetLastError());
            ErrorHandler(std::string("[WndCreator] Error Setting Window Ex Style: (" + err + ")"));
            return false;
        }
        this->Show();

        return true;
    }

    
    const bool ResetStyleEx(const LONG_PTR NewExStyle)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        this->Hide();
        SetLastError(0);
        if (!SetWindowLongPtrW(this->Wnd, GWL_EXSTYLE, NewExStyle) && GetLastError() != 0)
        {
            std::string err = std::to_string(GetLastError());
            ErrorHandler(std::string("[WndCreator] Error Setting Window Ex Style: (" + err + ")"));
            return false;
        }
        this->Show();

        return true;
    }


    const bool SetWndSz(const int X, const int Y, const int Width, const int Height, const UINT SwFlags = SWP_SHOWWINDOW, const HWND WndZPos = HWND_NOTOPMOST)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        if (!SetWindowPos(this->Wnd, WndZPos, X, Y, Width, Height, SwFlags))
        {
            ErrorHandler("[WndCreator] Error with SetWindowPos: " + std::to_string(GetLastError()));
            return false;
        }

        return true;
    }


    const bool EnableWnd()
    {
        return EnableWindow(this->Wnd, true);
    }


    const bool DisableWnd()
    {
        return EnableWindow(this->Wnd, false);
    }


    const bool SetWndTitle(const std::wstring& Str)
    {
        return SetWindowTextW(this->Wnd, Str.c_str());
    }


    const ScreenDimensions GetClientArea()
    {
        RECT rect;
        if (GetClientRect(this->Wnd, &rect))
        {
            return{ rect.right, rect.bottom};
        }
        else
        {
            return { 0, 0 };
        }
    }

    
    const std::wstring GetWndTitle()
    {
        wchar_t Buffer[1024];
        GetWindowTextW(this->Wnd, Buffer, 1024);

        std::wstring Str = Buffer;
        return Str;
    }

    
    const bool HasFocus()
    {
        if (GetFocus() == this->Wnd)
        {
            return true;
        }

        return false;
    }

    
    const bool SetLayeredAttributes(const COLORREF CrKey, const BYTE Alpha, const DWORD DwFlags = LWA_COLORKEY)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return false;
        }

        if (!SetLayeredWindowAttributes(Wnd, CrKey, Alpha, DwFlags))
        {
            ErrorHandler("[WndCreator] Failed to set layered attributes: " + std::to_string(GetLastError()));
            return false;
        }

        return true;
    }
     

    const LONG_PTR SetWndLong(int Index, LONG_PTR Value)
    {
        return SetWindowLongPtrW(this->Wnd, Index, Value);
    }

    
    const LRESULT SendWndMessage(const UINT MSG, WPARAM WParam, LPARAM LParam)
    {
        if (!this->Wnd)
        {
            ErrorHandler("[WndCreator] Error Window Handle Is Null");
            return -1;
        }

        return SendMessageW(this->Wnd, MSG, WParam, LParam);
    }

    
    HWND CreateChildWindow(const DWORD ExFlags, const DWORD WStyle, const std::wstring_view ClassName, const std::wstring_view WndName, const int x, const int y, const int Width, const int Height, const int Hmenu)
    {
        HWND chwnd = CreateWindowExW(ExFlags, ClassName.data(), WndName.data(), WStyle, x, y, Width, Height, this->Wnd, (HMENU)Hmenu, (HINSTANCE)GetWindowLongPtrW(this->Wnd, GWLP_HINSTANCE), NULL);

        if (chwnd == INVALID_HANDLE_VALUE || chwnd == NULL)
        {
            DWORD ErrorVal = GetLastError();

            if (ErrorVal == 1407)
            {
                this->LogError("[WNDCREATOR] Failed to create child window: Invalid class name");
            }

            this->LogError("[WNDCREATOR] Failed to create child window: " + std::to_string(ErrorVal));
        }

        this->Children.push_back(chwnd);

        return chwnd;
    }


    void DestroyChild(HWND hwnd)
    {
        auto it = std::find(this->Children.begin(), this->Children.end(), hwnd);
        
        if (it != this->Children.end()) 
        {
            std::swap(*it, this->Children.back());

            HWND chwnd = this->Children.back();
            this->Children.pop_back();

            SafeDestroyWindow(chwnd);
        }
    }


    void UpdateChildren()
    {
        for (HWND W : this->Children)
        {
            UpdateWindow(W);
        }
    }


    const void Destroy()
    {
        SafeDestroyWindow(this->Wnd);
    }
};