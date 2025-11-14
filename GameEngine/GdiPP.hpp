#include <iostream>
#include <windows.h>
#include <memory>

#define GDIPP_FILLRECT 0x1
#define GDIPP_REDRAW 0x2
#define GDIPP_INVALIDATE 0x3
#define GDIPP_PIXELCLEAR 0x4

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
#define SafeSelectObject(DC, New, Old, Type)  if(New != NULL && New != INVALID_HANDLE_VALUE) { Old = (Type)SelectObject(DC, New); } 

#ifdef _MSC_VER
__pragma(warning(default:6387))
#endif

#define NoDepends
    //#define PARALLEL_OMP

    // TODO: 
    // 1. add aa using ms algo 
    //     1a. Create original bitmap for drawing
    //     1b. Create temporary bitmap (2x, 4x or 8x) larger than the original bitmap
    //     1c. Render your graphics to this large temporary bitmap 
    //        (use any GDI method, pen or brush you like) 
    //        but scale the graphics appropriately
    //     1d. Draw this temporary bitmap on the original bitmap scaled 
    //        (i.e. using StretchDIBits() method or any other you like), 
    //        but call SetStretchBltMode(HALFTONE)
    //        before this last step for the original DC 
    //        (which holds the original bitmap), and after scaling restore it back
    //
    // 2. 



    COLORREF GetBrushColor(HBRUSH brush)
{
    LOGBRUSH lbr;
    if (GetObject(brush, sizeof(lbr), &lbr) != sizeof(lbr)) {
        // Not even a brush!
        return RGB(0, 0, 0);
    }
    if (lbr.lbStyle != BS_SOLID) {
        // Not a solid color brush.
        return RGB(0, 0, 0);
    }
    return lbr.lbColor;
}

#define PixelRound(Val) (int)std::roundf(Val)

class BrushPP
{
public:
    BrushPP() = default;

    BrushPP(const HBRUSH HBrush)
    {
        if (HBrush != NULL)
        {
            Brush = std::shared_ptr<HBRUSH>(new HBRUSH(HBrush), [](HBRUSH* ThisHBRUSH)
                {
                    if (*ThisHBRUSH != NULL)
                    {
                        DeleteObject(*ThisHBRUSH);
                    }
                    delete ThisHBRUSH;
                });
        }
    }

    BrushPP(const COLORREF Clr)
    {
        Brush = std::shared_ptr<HBRUSH>(new HBRUSH(CreateSolidBrush(Clr)), [](HBRUSH* ThisBrush)
            {
                if (*ThisBrush != NULL)
                {
                    DeleteObject(*ThisBrush);
                }
                delete ThisBrush;
            });
    }

    BrushPP(const int Hatch, const COLORREF Clr)
    {
        Brush = std::shared_ptr<HBRUSH>(new HBRUSH(CreateHatchBrush(Hatch, Clr)), [](HBRUSH* ThisBrush)
            {
                if (*ThisBrush != NULL)
                {
                    DeleteObject(*ThisBrush);
                }
                delete ThisBrush;
            });
    }

    BrushPP(const HBITMAP Bmp)
    {
        Brush = std::shared_ptr<HBRUSH>(new HBRUSH(CreatePatternBrush(Bmp)), [](HBRUSH* ThisBrush)
            {
                if (*ThisBrush != NULL)
                {
                    DeleteObject(*ThisBrush);
                }
                delete ThisBrush;
            });
    }

    // Copy constructor
    BrushPP(const BrushPP& other) : Brush(other.Brush)
    {

    }

    // Copy assignment
    BrushPP& operator=(const BrushPP& Rhs)
    {
        if (this != &Rhs)
        {
            Brush = Rhs.Brush;
        }
        return *this;
    }

    // Move constructor
    BrushPP(BrushPP&& Rhs) noexcept = default;

    // Move assignment
    BrushPP& operator=(BrushPP&& Rhs) noexcept = default;

    ~BrushPP() = default;

    operator HBRUSH() const
    {
        return *Brush;
    }

    explicit operator bool() const noexcept
    {
        return Brush != nullptr;
    }

private:
    std::shared_ptr<HBRUSH> Brush;
};

class PenPP
{
public:
    PenPP()
    {
        if (NeedsDestroyed)
        {
            if (Pen != NULL)
            {
                DeleteObject(Pen);
            }
        }

        NeedsDestroyed = false;
    }

    PenPP(const HPEN HPen)
    {
        if (NeedsDestroyed == true)
        {
            if (Pen != NULL)
            {
                DeleteObject(Pen);
            }
        }
        if (HPen != NULL)
        {
            Pen = HPen;
            NeedsDestroyed = true;
        }
        else
        {
            NeedsDestroyed = false;
        }
    }

    PenPP(const int Style, const int Width, COLORREF Clr)
    {
        if (NeedsDestroyed == true)
        {
            if (Pen != NULL)
            {
                DeleteObject(Pen);
            }
        }
        Pen = CreatePen(Style, Width, Clr);
        NeedsDestroyed = true;
    }

    // Disable copy assignment && Copy construct
    PenPP(const PenPP&) = delete;
    PenPP& operator=(const PenPP&) = delete;

    // Move constructor
    PenPP(PenPP&& Rhs) noexcept
    {
        if (Rhs.Pen != NULL)
        {
            Pen = Rhs.Pen;
            Rhs.Pen = nullptr;
        }
    }

    // Destructor
    ~PenPP()
    {
        if (NeedsDestroyed)
        {
            if (Pen != NULL)
            {
                DeleteObject(Pen);
            }
            NeedsDestroyed = false;
        }
    }

    operator HPEN() const
    {
        return Pen;
    }

    explicit operator bool() const
    {
        if (Pen != NULL && Pen != INVALID_HANDLE_VALUE)
            return true;
        return false;
    }

    // Move assignment
    PenPP& operator = (PenPP&& Rhs) noexcept
    {
        if (this != &Rhs)
        {
            if (NeedsDestroyed == true)
            {
                if (Pen != NULL)
                {
                    DeleteObject(Pen);
                }
            }

            Pen = Rhs.Pen;
            Rhs.Pen = nullptr;
            NeedsDestroyed = true;
        }
        return *this;
    }

private:
    HPEN Pen{};
    bool NeedsDestroyed = false;
    int RefCount = 0;
};

class GdiPP
{
    HPEN OldPen = NULL;
    HBRUSH OldBrush = NULL;
    bool DoubleBuffered = false;

public:
    HWND Wnd = NULL;
    HDC ScreenDC = NULL;
    RECT ClientRect = {};
    POINT ScreenSz = {};
    HDC MemDC = NULL;
    HBITMAP MemBM = NULL;
    HBITMAP OldBM = NULL;
    void(*ErrorHandler)(std::string) = LogError;
    int Stride = 0;
    BITMAPINFO bi;

private:
    BYTE* PixelBuffer = nullptr;
    BYTE* GdiBuffer = nullptr;
    bool NeedsPixelsDrawn = false;
    COLORREF ClearColor = RGB(0, 0, 0);

    void MergePixelBuffers(PBYTE Output, PBYTE Input, int Width, int Height, COLORREF IgnoreColor)
    {
        unsigned char IgnoreB = GetBValue(IgnoreColor);
        unsigned char IgnoreG = GetGValue(IgnoreColor);
        unsigned char IgnoreR = GetRValue(IgnoreColor);

#ifdef PARALLEL_OMP
#pragma omp parallel for
#endif
        for (int i = 0; i < Width * Height; i++)
        {
            PBYTE OutIdx = Output + i * 3;
            PBYTE InIdx = Input + i * 3;

            if (OutIdx[0] == IgnoreB && OutIdx[1] == IgnoreG && OutIdx[2] == IgnoreR)
            {
                std::memcpy(OutIdx, InIdx, 3);  // using memcpy for copying can sometimes be faster
            }
        }
    }



public:

    static void LogError(std::string ErrorMsg)
    {
        std::cout << ErrorMsg << std::endl;
    }

    GdiPP() // Default Ctor
    {
        this->~GdiPP();
    }

    GdiPP(const HWND& Wnd, const bool IsDoubleBuffered = false, HBRUSH ClearBrush = (HBRUSH)GetStockObject(BLACK_BRUSH)) // ctor
    {
        DoubleBuffered = IsDoubleBuffered;
        this->Wnd = Wnd;

        ScreenDC = GetDC(Wnd);

        if (!ScreenDC)
        {
            ErrorHandler("[GdiPP]  Failed to retrieve dc from HWND");
            SafeReleaseDC(Wnd, ScreenDC);
            return;
        }

        this->ClearColor = GetBrushColor(ClearBrush);

        ZeroMemory(&bi, sizeof(bi));
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = this->ScreenSz.x;
        bi.bmiHeader.biHeight = -this->ScreenSz.y;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 24;
        bi.bmiHeader.biCompression = BI_RGB;

        if (IsDoubleBuffered == true)
        {
            MemDC = CreateCompatibleDC(ScreenDC);

            // optimized this TODO 
            //MemBM = CreateCompatibleBitmap(ScreenDC, this->ScreenSz.x, this->ScreenSz.y);

            this->UpdateClientRgn();

            if (!MemDC || !MemBM || !OldBM)
            {
                ErrorHandler("[GdiPP]  Failed to initialize DoubleBuffering");
                SafeReleaseDC(Wnd, ScreenDC);
                SafeDeleteDC(MemDC);
                SafeDeleteObject(MemBM);
                SafeDeleteObject(OldBM);
                // TODO: Throw?
            }
        }
        else
        {
            this->UpdateClientRgn();
        }
    }

    // disable copy
    //GdiPP(const GdiPP&) = delete;
    //GdiPP& operator=(const GdiPP&) = delete;

    ~GdiPP() // dtor 
    {
        SelectObject(MemDC, OldBM); // i dont think this line is needed TODO
        SelectObject(MemDC, OldPen); // i dont think this line is needed TODO
        SelectObject(MemDC, OldBrush); // i dont think this line is needed TODO
        SafeDeleteObject(MemBM);
        SafeDeleteObject(OldBM);
        SafeDeleteObject(OldPen);
        SafeDeleteObject(OldBrush);
        SafeDeleteDC(MemDC);
        SafeReleaseDC(Wnd, ScreenDC);
    }

    bool __inline __fastcall ChangePen(const HPEN NewPen)
    {
        if (!DoubleBuffered && NewPen)
        {
            if (ScreenDC)
            {
                SafeSelectObject(this->ScreenDC, NewPen, OldPen, HPEN);

                if (!OldPen)
                {
                    ErrorHandler("[GdiPP]  Failed to create pen");
                    return false;
                }
            }
        }
        else
        {
            if (MemDC)
            {
                SafeSelectObject(this->MemDC, NewPen, OldPen, HPEN);

                if (!OldPen)
                {
                    ErrorHandler("[GdiPP]  Failed to create pen");
                    return false;
                }
            }
        }

        return true;
    }

    bool __inline __fastcall ChangeBrush(const HBRUSH NewBrush)
    {
        if (!DoubleBuffered && NewBrush)
        {
            if (ScreenDC)
            {
                SafeSelectObject(this->ScreenDC, NewBrush, OldBrush, HBRUSH);

                if (!OldBrush)
                {
                    ErrorHandler("[GdiPP]  Error creating solid brush");
                    return false;
                }
            }
        }
        else
        {
            if (MemDC)
            {
                SafeSelectObject(this->MemDC, NewBrush, OldBrush, HBRUSH);

                if (!OldBrush)
                {
                    ErrorHandler("[GdiPP]  Error creating solid brush");
                    return false;
                }
            }
        }
        return true;
    }


    bool __inline __fastcall ChangeBitmap(const HBITMAP NewBitMap)
    {
        if (!DoubleBuffered && NewBitMap)
        {
            if (ScreenDC)
            {
                SafeSelectObject(this->ScreenDC, NewBitMap, OldBM, HBITMAP);

                if (!OldBM)
                {
                    ErrorHandler("[GdiPP]  Error creating solid brush");
                    return false;
                }
            }
        }
        else
        {
            if (MemDC)
            {
                SafeSelectObject(this->MemDC, NewBitMap, OldBM, HBITMAP);

                if (!OldBM)
                {
                    ErrorHandler("[GdiPP]  Error creating solid brush");
                    return false;
                }
            }
        }
    }


    //Unfilled Shapes
    const bool DrawRectangle(const int X, const int Y, const int Width, const int Height)
    {
        if (!DoubleBuffered)
        {
            if (!ScreenDC)
                return false;

            return Rectangle(ScreenDC, X, Y, X + Width, Y + Height);
        }
        else
        {
            if (!MemDC)
                return false;

            return Rectangle(MemDC, X, Y, X + Width, Y + Height);
        }
    }


    const bool DrawRectangle(const int X, const int Y, const int Width, const int Height, HPEN Line)
    {
        bool Status = false;

        this->ChangePen(Line);

        if (!DoubleBuffered)
        {
            if (!ScreenDC)
                return false;

            Status = Rectangle(ScreenDC, X, Y, X + Width, Y + Height);

            this->ChangePen(this->OldPen);

            return Status;
        }
        else
        {
            if (!MemDC)
                return false;

            Status = Rectangle(MemDC, X, Y, X + Width, Y + Height);

            this->ChangePen(this->OldPen);

            return Status;
        }
    }


    const bool DrawStringA(const int X, const int Y, const std::string_view Text, const COLORREF TextColor, const int BkMode)
    {
        if (!DoubleBuffered)
        {
            if (!ScreenDC)
                return false;

            if (!SetBkMode(ScreenDC, BkMode))
            {
                ErrorHandler("[GdiPP]  Failed to set text background mode");
                return false;
            }
            if (SetTextColor(ScreenDC, TextColor) == CLR_INVALID)
            {
                ErrorHandler("[GdiPP]  Failed to set text color");
                return false;
            }
            return TextOutA(ScreenDC, X, Y, Text.data(), (int)Text.length());
        }
        else
        {
            if (!MemDC)
                return false;

            if (!SetBkMode(MemDC, BkMode))
            {
                ErrorHandler("[GdiPP]  Failed to set text background mode");
                return false;
            }
            if (SetTextColor(MemDC, TextColor) == CLR_INVALID)
            {
                ErrorHandler("[GdiPP]  Failed to set text color");
                return false;
            }
            return TextOutA(MemDC, X, Y, Text.data(), (int)Text.length());
        }
    }


    const bool DrawStringW(const int X, const int Y, const std::wstring_view Text, const COLORREF TextColor, const int BkMode)
    {
        if (!DoubleBuffered)
        {
            if (!ScreenDC)
                return false;

            if (!SetBkMode(ScreenDC, BkMode))
            {
                ErrorHandler("[GdiPP]  Failed to set text background mode");
                return false;
            }
            if (SetTextColor(ScreenDC, TextColor) == CLR_INVALID)
            {
                ErrorHandler("[GdiPP]  Failed to set text color");
                return false;
            }
            return TextOutW(ScreenDC, X, Y, Text.data(), (int)Text.length());
        }
        else
        {
            if (!MemDC)
                return false;

            if (!SetBkMode(MemDC, BkMode))
            {
                ErrorHandler("[GdiPP]  Failed to set text background mode");
                return false;
            }
            if (SetTextColor(MemDC, TextColor) == CLR_INVALID)
            {
                ErrorHandler("[GdiPP]  Failed to set text color");
                return false;
            }
            return TextOutW(MemDC, X, Y, Text.data(), (int)Text.length());
        }
    }


    const bool DrawLine(const int StartX, const int StartY, const int EndX, const int EndY)
    {
        bool Status = false;

        if (!DoubleBuffered)
        {
            if (!ScreenDC)
                return false;

            POINT OldPos;
            Status = MoveToEx(ScreenDC, StartX, StartY, &OldPos);
            Status = LineTo(ScreenDC, EndX, EndY);
            Status = MoveToEx(ScreenDC, OldPos.x, OldPos.y, nullptr);
            if (!Status)
            {
                ErrorHandler("[GdiPP]  Failed to DrawLine");
                return false;
            }
            return true;
        }
        else
        {
            if (!MemDC)
                return false;

            POINT OldPos;
            Status = MoveToEx(MemDC, StartX, StartY, &OldPos);
            Status = LineTo(MemDC, EndX, EndY);
            Status = MoveToEx(MemDC, OldPos.x, OldPos.y, nullptr);
            if (!Status)
            {
                ErrorHandler("[GdiPP]  Failed to DrawLine");
                return false;
            }
            return true;
        }
    }


    const bool DrawLine(const int StartX, const int StartY, const int EndX, const int EndY, HPEN Line)
    {
        bool Status = false;

        this->ChangePen(Line);

        if (!DoubleBuffered)
        {
            if (!ScreenDC)
                return false;

            POINT OldPos;
            Status = MoveToEx(ScreenDC, StartX, StartY, &OldPos);
            Status = LineTo(ScreenDC, EndX, EndY);
            Status = MoveToEx(ScreenDC, OldPos.x, OldPos.y, nullptr);
            if (!Status)
            {
                ErrorHandler("[GdiPP]  Failed to DrawLine");
                return false;
            }
            this->ChangePen(this->OldPen);
            return Status;
        }
        else
        {
            if (!MemDC)
                return false;

            POINT OldPos;
            Status = MoveToEx(MemDC, StartX, StartY, &OldPos);
            Status = LineTo(MemDC, EndX, EndY);
            Status = MoveToEx(MemDC, OldPos.x, OldPos.y, nullptr);
            if (!Status)
            {
                ErrorHandler("[GdiPP]  Failed to DrawLine");
                return false;
            }
            this->ChangePen(this->OldPen);
            return Status;
        }
    }


    const bool DrawEllipse(const int X, const int Y, const int Width, const int Height)
    {
        if (!DoubleBuffered)
        {
            if (!ScreenDC)
                return false;
            return Ellipse(ScreenDC, X, Y, X + Width, Y + Height);
        }
        else
        {
            if (!ScreenDC)
                return false;

            return Ellipse(MemDC, X, Y, X + Width, Y + Height);
        }
    }


    const bool DrawEllipse(const int X, const int Y, const int Width, const int Height, HPEN Line)
    {
        bool Status = false;

        this->ChangePen(Line);

        if (!DoubleBuffered)
        {
            if (!ScreenDC)
                return false;

            Status = Ellipse(ScreenDC, X, Y, X + Width, Y + Height);

            this->ChangePen(this->OldPen);

            return Status;
        }
        else
        {
            if (!MemDC)
                return false;

            Status = Ellipse(MemDC, X, Y, X + Width, Y + Height);

            this->ChangePen(this->OldPen);

            return Status;
        }
    }


    const bool DrawTriangle(const int X1, const int Y1, const int X2, const int Y2, const int X3, const int Y3)
    {
        bool Status = false;

        Status = DrawLine(X1, Y1, X2, Y2);
        Status = DrawLine(X2, Y2, X3, Y3);
        Status = DrawLine(X3, Y3, X1, Y1);

        return Status;
    }


    const bool DrawTriangle(const int X1, const int Y1, const int X2, const int Y2, const int X3, const int Y3, HPEN Line)
    {
        bool Status = false;

        this->ChangePen(Line);

        Status = DrawLine(X1, Y1, X2, Y2);
        Status = DrawLine(X2, Y2, X3, Y3);
        Status = DrawLine(X3, Y3, X1, Y1);

        this->ChangePen(this->OldPen);

        return Status;
    }


    const bool __inline __fastcall SetPixel(const int& X, const int& Y, COLORREF Clr)
    {
        if (DoubleBuffered)
        {
            if (!MemDC || !PixelBuffer)
                return false;

            int index = (Y * this->Stride) + (X * 3);

            PBYTE pixelPtr = &PixelBuffer[index];
            *pixelPtr++ = GetBValue(Clr);
            *pixelPtr++ = GetGValue(Clr);
            *pixelPtr = GetRValue(Clr);
            this->NeedsPixelsDrawn = true;
            return true;
        }
        else
        {
            if (!ScreenDC)
                return false;

            return SetPixelV(ScreenDC, X, Y, Clr);
        }
    }


    void __inline __fastcall QuickSetPixel(const int& X, const int& Y, COLORREF Clr)
    {
        int index = (Y * this->Stride) + (X * 3);

        PBYTE pixelPtr = &PixelBuffer[index];
        *pixelPtr++ = GetBValue(Clr);
        *pixelPtr++ = GetGValue(Clr);
        *pixelPtr = GetRValue(Clr);
        this->NeedsPixelsDrawn = true;
    }


    const bool __inline __fastcall SetBits(const int Width, const int Height, void* Bits, const int StartHeight = 0)
    {
        static BITMAPINFO BI;
        ZeroMemory(&BI, sizeof(BI));
        BI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        BI.bmiHeader.biWidth = Width;
        BI.bmiHeader.biHeight = -Height;
        BI.bmiHeader.biPlanes = 1;
        BI.bmiHeader.biBitCount = 24;
        BI.bmiHeader.biCompression = BI_RGB;

        if (!DoubleBuffered)
        {
            if (!ScreenDC)
                return false;

            SetDIBits(this->ScreenDC, this->MemBM, StartHeight, Height, Bits, &BI, DIB_RGB_COLORS);
        }
        else
        {
            if (!MemDC)
                return false;

            SetDIBits(this->MemDC, this->MemBM, StartHeight, Height, Bits, &BI, DIB_RGB_COLORS);
        }
    }


    // Filled Shapes
    // Note: It is the responsibility of the caller to free the brush 
    const bool DrawFilledRect(const int X, const int Y, const int Width, const int Height, HBRUSH BG)
    {
        if (!DoubleBuffered)
        {
            if (!ScreenDC)
                return false;

            RECT r = {};

            SetRect(&r, X, Y, X + Width, Y + Height);

            return FillRect(ScreenDC, &r, BG);
        }
        else
        {
            if (!MemDC)
                return false;

            RECT r = {};

            SetRect(&r, X, Y, X + Width, Y + Height);

            return FillRect(MemDC, &r, BG);
        }
    }


    const bool DrawFilledRect(const int X, const int Y, const int Width, const int Height)
    {
        if (!DoubleBuffered)
        {
            if (!ScreenDC)
                return false;

            // Top Left, Bottom Left, Bottom Right, Top Right
            POINT Verts[4] = { {X, Y}, {X, Y + Height}, {X + Width, Y + Height}, {X + Width, Y} };

            return Polygon(ScreenDC, Verts, 4);
        }
        else
        {
            if (!MemDC)
                return false;

            POINT Verts[4] = { {X, Y}, {X, Y + Height}, {X + Width, Y + Height}, {X + Width, Y} };

            return Polygon(MemDC, Verts, 4);
        }
    }


    const bool DrawFilledRect(const int X, const int Y, const int Width, const int Height, HPEN OutLine, HBRUSH BG = (HBRUSH)INVALID_HANDLE_VALUE)
    {
        bool Result = false;

        if (BG != (HBRUSH)INVALID_HANDLE_VALUE)
            this->ChangeBrush(BG);

        this->ChangePen(OutLine);

        if (!DoubleBuffered)
        {
            if (!ScreenDC)
                return false;

            // Top Left, Bottom Left, Bottom Right, Top Right
            POINT Verts[4] = { {X, Y}, {X, Y + Height}, {X + Width, Y + Height}, {X + Width, Y} };

            Result = Polygon(ScreenDC, Verts, 4);

            this->ChangePen(this->OldPen);
            if (BG != (HBRUSH)INVALID_HANDLE_VALUE)
            {
                this->ChangeBrush(this->OldBrush);
            }

            return Result;
        }
        else
        {
            if (!MemDC)
                return false;

            POINT Verts[4] = { {X, Y}, {X, Y + Height}, {X + Width, Y + Height}, {X + Width, Y} };

            Result = Polygon(MemDC, Verts, 4);

            this->ChangePen(this->OldPen);
            if (BG != (HBRUSH)INVALID_HANDLE_VALUE)
            {
                this->ChangeBrush(this->OldBrush);
            }

            return Result;
        }
    }


    const bool __fastcall DrawFilledTriangle(const int X1, const int Y1, const int X2, const int Y2, const int X3, const int Y3, const HBRUSH BG = nullptr, const HPEN OutLine = nullptr)
    {
        bool Result = false;

        this->ChangePen(OutLine);
        this->ChangeBrush(BG);

        if (!this->DoubleBuffered)
        {
            if (!ScreenDC)
                return false;

            POINT Verts[3] = { {X1, Y1}, {X2, Y2}, {X3, Y3} };

            Result = Polygon(ScreenDC, Verts, 3);
        }
        else
        {
            if (!MemDC)
                return false;

            POINT Verts[3] = { {X1, Y1}, {X2, Y2}, {X3, Y3} };

            Result = Polygon(MemDC, Verts, 3);
        }
        this->ChangePen(this->OldPen);
        this->ChangeBrush(this->OldBrush);


        return Result;
    }


    /*
    const bool DrawFilledTriangle(const int X1, const int Y1, const int X2, const int Y2, const int X3, const int Y3)
    {
        POINT Verts[3];
        Verts[0] = { X1, Y1 };
        Verts[1] = { X2, Y2 };
        Verts[2] = { X3, Y3 };

        if (!this->DoubleBuffered)
        {
            if (!ScreenDC)
                return false;

            return Polygon(ScreenDC, Verts, sizeof(Verts) / sizeof(Verts[0]));
        }
        else
        {
            if (!MemDC)
                return false;

            return Polygon(MemDC, Verts, sizeof(Verts) / sizeof(Verts[0]));
        }
    }
    */

    // Draws a polygon using the vertices
    // Vertices (In): Specifys the vertices to be used  
    // Returns true if successful
    const bool DrawPolygon(POINT Vertices[], int Count)
    {
        if (!this->DoubleBuffered)
        {
            if (!ScreenDC)
                return false;

            return Polygon(ScreenDC, Vertices, Count);
        }
        else
        {
            if (!MemDC)
                return false;

            return Polygon(MemDC, Vertices, Count);
        }
    }


    // Draws any content in the double buffer
    // DoTransparentBlt (in, optional): Preforms a transparent blt  
    // TransColor (in, optional): Specifys color to make transparent in the transparent blt
    // ROP (in, optional): Specifys copy mode to use in the blt call
    const bool __fastcall DrawDoubleBuffer(const DWORD ROP, const bool DoTransparentBlt = false, const COLORREF TransColor = RGB(0, 0, 0))
    {
        if (!DoTransparentBlt)
        {
            if (!BitBlt(ScreenDC, 0, 0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top, MemDC, 0, 0, ROP))
            {
                ErrorHandler("[GdiPP]  Failed to BitBlt");
                return false;
            }
            return true;
        }
        else
        {
#ifndef NoDepends
            GetClientRect(this->Wnd, &ClientRect);
            if (!TransparentBlt(ScreenDC, 0, 0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top, MemDC, 0, 0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top, RGB(0, 0, 0)))
            {
                ErrorHandler("[GdiPP]  Failed to TransparentBlt");
                return false;
            }
            return true;
#endif
        }

        return false;
    }


    const bool __inline __fastcall DrawDoubleBufferPO()
    {
        if (this->NeedsPixelsDrawn)
        {
            SetDIBitsToDevice(this->ScreenDC, 0, 0, this->ScreenSz.x, this->ScreenSz.y, 0, 0, 0, this->ScreenSz.y, PixelBuffer, &bi, DIB_RGB_COLORS);

            this->NeedsPixelsDrawn = false;
            return true;
        }

        if (BitBlt(ScreenDC, 0, 0, this->ScreenSz.x, this->ScreenSz.y, MemDC, 0, 0, SRCCOPY))
        {
            return true;
        }

        ErrorHandler("[GdiPP]  Failed to BitBlt");
        return false;
    }

    // Clears different ways depending on ClearMode
    // ClearMode (in): Specifys how to clear dc
    // CearBrush (in, optional): Brush to clear with (only used with GDIPP_FILLRECT)
    // Note: It is the responsibility of the caller to free the brush 
    void Clear(const DWORD ClearMode = GDIPP_PIXELCLEAR, const HBRUSH ClearBrush = (HBRUSH)GetStockObject(BLACK_BRUSH))
    {
        LOGBRUSH lb;

        if (GetObject(ClearBrush, sizeof(LOGBRUSH), &lb) != 0)
            this->ClearColor = lb.lbColor;
        else
            this->ErrorHandler("[GDIPP] Unable to get brush color");

        ZeroMemory(PixelBuffer, this->Stride * ScreenSz.y);

        if (ClearMode == GDIPP_PIXELCLEAR)
        {
            if (DoubleBuffered)
            {
                SetDIBitsToDevice(this->MemDC, 0, 0, this->ScreenSz.x, this->ScreenSz.y, 0, 0, 0, this->ScreenSz.y, PixelBuffer, &bi, DIB_RGB_COLORS);
            }
            else
                SetDIBitsToDevice(this->ScreenDC, 0, 0, this->ScreenSz.x, this->ScreenSz.y, 0, 0, 0, this->ScreenSz.y, PixelBuffer, &bi, DIB_RGB_COLORS);
            return;
        }
        else if (ClearMode == GDIPP_FILLRECT)
        {
            if (!ClearBrush)
                return;

            if (!DoubleBuffered)
                FillRect(ScreenDC, &ClientRect, ClearBrush);
            else
                FillRect(MemDC, &ClientRect, ClearBrush);

            return;
        }
        else if (ClearMode == GDIPP_INVALIDATE)
        {
            InvalidateRect(Wnd, NULL, TRUE);
            UpdateWindow(Wnd);
            return;
        }
        else if (ClearMode == GDIPP_REDRAW)
        {
            RedrawWindow(Wnd, NULL, NULL, RDW_ERASENOW | RDW_ERASE | RDW_INVALIDATE);
            return;
        }
    }


    // Updates the ClientRect variable
    // Returns true if successful
    bool UpdateClientRgn()
    {
        bool Stat = GetClientRect(Wnd, &this->ClientRect);
        int bytesPerPixel = 3; // 24-bit RGB image

        this->ScreenSz.x = ClientRect.right - ClientRect.left;
        this->ScreenSz.y = ClientRect.bottom - ClientRect.top;
        this->Stride = ((this->ScreenSz.x * bytesPerPixel + 3) / 4) * 4; // Calculate stride

        bi.bmiHeader.biWidth = this->ScreenSz.x;
        bi.bmiHeader.biHeight = -this->ScreenSz.y;

        MemBM = CreateDIBSection(this->ScreenDC, &bi, DIB_RGB_COLORS, (void**)&PixelBuffer, NULL, 0);
        std::string str = std::to_string(GetLastError());
        OldBM = (HBITMAP)SelectObject(MemDC, MemBM);

        return Stat;
    }
};