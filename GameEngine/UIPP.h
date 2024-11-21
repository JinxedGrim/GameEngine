#pragma once
#include "WndCreator.hpp"

const static int StartID = 10000;
static int* UIPPIDCounter;

typedef enum class UiObjectTypes
{
	UiTypeButton = 0,
};

class UiObjectW
{
	UiObjectTypes Type;

	HBRUSH CtlColor = (HBRUSH)GetStockObject(WHITE_BRUSH);
	COLORREF TextColor = RGB(255, 0, 0);
	WndCreatorW* ParentWnd = nullptr;

	public:
	int ID = 0;
	WndCreatorW ObjectWnd = 0;
	std::function<bool(HWND, WPARAM, LPARAM)> CustomPaintCb = nullptr;
	std::function<bool(HWND, UINT, WPARAM, LPARAM)> CustomOtherMsgCb = nullptr;


	virtual bool PaintCb(HWND hwnd, WPARAM wParam, LPARAM lParam)
	{
		if (this->CustomPaintCb != nullptr)
		{
			CustomPaintCb(hwnd, wParam, lParam);
			return true;
		}

		return false;
	}


	virtual bool OtherMessageCb(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (this->CustomOtherMsgCb != nullptr)
		{
			CustomOtherMsgCb(hwnd, uMsg, wParam, lParam);
			return true;
		}

		return false;
	}


	virtual bool HandleWndCommand(HWND wnd, int Notification)
	{
		return false;
	}


	virtual bool HandleColorCommand(HDC Hdc, HBRUSH* OutBrush)
	{
		return false;
	}


	virtual bool HandleScrollCommand(int HIWORD, int LOWORD)
	{
		return false;
	}


	virtual void SpawnWindow(WndCreatorW* ParentWindow, int x, int y, int Width, int Height, DWORD Style, std::wstring ClassName, std::wstring WindowNane)
	{
		this->ParentWnd = ParentWindow;
		this->ObjectWnd = WndCreatorW(ParentWindow->CreateChildWindow((DWORD)WndExModes::ChildEx, (DWORD)WndModes::Child | (DWORD)WndModes::ClipChildren | Style, ClassName, WindowNane, x, y, Width, Height, this->ID));
		this->ObjectWnd.OverrideWndProc();

		auto BoundFunc = [this](HWND hwnd, WPARAM x, LPARAM y)
		{
			return this->PaintCb(hwnd, x, y);
		};

		this->ObjectWnd.PaintCb = BoundFunc;

		auto BoundFunc2 = [this](HWND hwnd, UINT uMsg, WPARAM x, LPARAM y)
		{
			return this->OtherMessageCb(hwnd, uMsg, x, y);
		};

		this->ObjectWnd.OtherMessageCb = BoundFunc2;
	}


	void SetCtlClr(HBRUSH newBrush)
	{
		this->CtlColor = newBrush;
	}


	void SetTextClr(COLORREF Clr)
	{
		this->TextColor = Clr;
	}


	HBRUSH GetCtlClr()
	{
		return this->CtlColor;
	}


	COLORREF GetTextClr()
	{
		return this->TextColor;
	}


	UiObjectW(UiObjectTypes _Type, int Id)
	{
		this->Type = _Type;
		this->ID = Id;

		this->CtlColor = (HBRUSH)GetStockObject(WHITE_BRUSH);
		this->TextColor = RGB(255, 0, 0);
	}


	virtual void SetPosAndSz(int X, int Y, int Width, int Height)
	{
		this->ObjectWnd.SetWndSz(X, Y, Width, Height);
	}


	virtual ScreenDimensions GetPos()
	{
		return this->ObjectWnd.GetClientArea();
	}


	virtual void Enable()
	{
		this->ObjectWnd.EnableWnd();
	}


	virtual void Disable()
	{
		this->ObjectWnd.DisableWnd();
	}


	virtual void SetText(std::wstring Str)
	{
		this->ObjectWnd.SetWndTitle(Str);
	}


	virtual std::wstring GetText()
	{
		return this->ObjectWnd.GetWndTitle();
	}


	virtual void DestroyObject()
	{
		this->ParentWnd->DestroyChild(this->ObjectWnd.Wnd);
	}


	~UiObjectW()
	{
		this->DestroyObject();
	}
};

class UiPushButtonW : public UiObjectW
{
	public:

	std::function<void()> ButtonClickedHandler = nullptr;

	bool HandleWndCommand(HWND wnd, int Notification) override
	{
		if (Notification == BN_CLICKED && ButtonClickedHandler != nullptr)
		{
			ButtonClickedHandler();
			return true;
		}

		return false;
	}


	bool HandleColorCommand(HDC Hdc, HBRUSH* BrushOut) override
	{
		SetTextColor(Hdc, this->GetTextClr());
		*BrushOut = this->GetCtlClr();

		return false;
	}


	UiPushButtonW(WndCreatorW* ParentWindow, int x, int y, int Width, int Height, ButtonStyles ButtonStyle, std::wstring ButtonText, int Id, std::function<void()> _ButtonClickedHandler) : UiObjectW(UiObjectTypes::UiTypeButton, Id)
	{
		ButtonClickedHandler = _ButtonClickedHandler;
		this->SpawnWindow(ParentWindow, x, y, Width, Height, (DWORD)ButtonStyle | BS_PUSHBUTTON, L"BUTTON", ButtonText);
	}
};

class UiRadioButtonW : public UiObjectW
{
	public:

	std::function<void(bool*)> ButtonClickedHandler = nullptr;
	bool* Checked = nullptr;

	bool HandleWndCommand(HWND wnd, int Notification) override
	{
		if (Notification == BN_CLICKED)
		{
			*Checked = !*Checked;

			if (ButtonClickedHandler != nullptr)
			{
				ButtonClickedHandler(Checked);
			}

			return true;
		}

		return false;
	}


	UiRadioButtonW(WndCreatorW* ParentWindow, int x, int y, int Width, int Height, RadioButtonStyles ButtonStyle, std::wstring ButtonText, int Id, bool* _Checked, std::function<void(bool*)> _ButtonClickedHandler = nullptr) : UiObjectW(UiObjectTypes::UiTypeButton, Id)
	{
		this->Checked = _Checked;
		this->ButtonClickedHandler = _ButtonClickedHandler;
		this->SpawnWindow(ParentWindow, x, y, Width, Height, (DWORD)ButtonStyle | BS_AUTORADIOBUTTON, L"BUTTON", ButtonText);
	}
};

class UiCheckBoxW : public UiObjectW
{
	public:

	std::function<void(bool*)> ButtonClickedHandler;
	bool* State = nullptr;

	bool HandleWndCommand(HWND wnd, int Notification) override
	{
		if (Notification == BN_CLICKED)
		{
			*State = !*State;

			if (ButtonClickedHandler != nullptr)
			{
				ButtonClickedHandler(State);
			}

			return true;
		}

		return false;
	}

	UiCheckBoxW(WndCreatorW* ParentWindow, int x, int y, int Width, int Height, CheckboxStyles ButtonStyle, std::wstring ButtonText, int Id, bool* _State, std::function<void(bool*)> _ButtonClickedHandler = nullptr) : UiObjectW(UiObjectTypes::UiTypeButton, Id)
	{
		this->State = _State;
		this->ButtonClickedHandler = _ButtonClickedHandler;
		this->SpawnWindow(ParentWindow, x, y, Width, Height, (DWORD)ButtonStyle | BS_AUTOCHECKBOX, L"BUTTON", ButtonText);
	}
};

class UiTextInputW : public UiObjectW
{
	public:

	std::function<void(std::wstring*)> TextChangedHandler;
	std::wstring* InputBuffer = nullptr;

	bool HandleWndCommand(HWND wnd, int Notification) override
	{
		if (Notification == EN_UPDATE)
		{
			if (InputBuffer != nullptr)
			{
				*InputBuffer = this->GetText();
			}

			if (TextChangedHandler != nullptr)
			{
				TextChangedHandler(InputBuffer);
			}

			return false;
		}

		return false;
	}

	UiTextInputW(WndCreatorW* ParentWindow, int x, int y, int Width, int Height, EditControlStyles EditStyle, std::wstring StartString, int Id, std::wstring* Buffer, std::function<void(std::wstring*)> _TextChangedHandler = nullptr) : UiObjectW(UiObjectTypes::UiTypeButton, Id)
	{
		this->InputBuffer = Buffer;
		this->TextChangedHandler = _TextChangedHandler;
		this->SpawnWindow(ParentWindow, x, y, Width, Height, (DWORD)EditStyle, L"EDIT", StartString);
	}
};

class UiSliderFloatW : public UiObjectW
{
	public:

	float* Value = nullptr;
	float MinVal = 0.0f;
	float MaxVal = 5.0f;

	bool HandleScrollCommand(int HIWORD, int LOWORD) override
	{
		int intPos = this->ObjectWnd.SendWndMessage(TBM_GETPOS, 0, 0);
		*this->Value = MinVal + (MaxVal - MinVal) * (intPos / 100.0f);

		return true;
	}

	UiSliderFloatW(WndCreatorW* ParentWindow, int x, int y, int Width, int Height, SliderStyles SliderStyle, int Id, float* Value, float MinVal, float MaxVal) : UiObjectW(UiObjectTypes::UiTypeButton, Id)
	{
		this->Value = Value;
		this->MinVal = MinVal;
		this->MaxVal = MaxVal;
		this->SpawnWindow(ParentWindow, x, y, Width, Height, (DWORD)SliderStyle | TBS_AUTOTICKS, TRACKBAR_CLASSW, L"");
		this->ObjectWnd.SendWndMessage(TBM_SETRANGE, TRUE, MAKELONG(0, 100));
		this->ObjectWnd.SendWndMessage(TBM_SETPOS, TRUE, *Value);
	}
};

class UiSliderIntW : public UiObjectW
{
	public:

	int* Value = nullptr;
	int MinVal = 0.0f;
	int MaxVal = 5.0f;

	bool HandleScrollCommand(int HIWORD, int LOWORD) override
	{
		int intPos = this->ObjectWnd.SendWndMessage(TBM_GETPOS, 0, 0);
		*this->Value = intPos;

		return true;
	}

	UiSliderIntW(WndCreatorW* ParentWindow, int x, int y, int Width, int Height, SliderStyles Style, int Id, int* Value, int MinVal, int MaxVal) : UiObjectW(UiObjectTypes::UiTypeButton, Id)
	{
		this->Value = Value;
		this->MinVal = MinVal;
		this->MaxVal = MaxVal;
		this->SpawnWindow(ParentWindow, x, y, Width, Height, (DWORD)Style | (DWORD)SliderStyles::AutoTicks, TRACKBAR_CLASSW, L"");
		this->ObjectWnd.SendWndMessage(TBM_SETRANGE, TRUE, MAKELONG(MinVal, MaxVal));
		this->ObjectWnd.SendWndMessage(TBM_SETPOS, TRUE, *Value);
	}
};

class UiLabelW : public UiObjectW
{
	public:

	UiLabelW(WndCreatorW* ParentWindow, int x, int y, int Width, int Height, LabelStyles Style, int Id, std::wstring Value) : UiObjectW(UiObjectTypes::UiTypeButton, Id)
	{
		this->SpawnWindow(ParentWindow, x, y, Width, Height, (DWORD)Style, L"STATIC", Value);
	}
};

class UiPPW
{
	WndCreatorW* ParentWindow;
	std::vector<UiObjectW*> UiElements;
	HBRUSH BackgroundBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
	public:

	UiPPW(WndCreatorW* Wnd)
	{
		if (UIPPIDCounter == nullptr)
		{
			UIPPIDCounter = new int;
			*UIPPIDCounter = 1;
		}

		this->ParentWindow = Wnd;

		auto BoundFunc = [this](HWND hwnd, int x, int y)
		{
			return this->CommandCb(hwnd, x, y);
		};

		auto BoundFunc2 = [this](HWND hwnd, int x, int y)
		{
			return this->ScrollCb(hwnd, x, y);
		};

		auto BoundFunc3 = [this](HWND x, UINT y, WPARAM z, LPARAM w, HBRUSH* a)
		{
			return this->ColorCtlCb(x, y, z, w, a);
		};

		this->ParentWindow->CommandCb = BoundFunc;
		this->ParentWindow->ControlScrollCb = BoundFunc2;

		this->ParentWindow->ColorCtlCb = BoundFunc3;
	}


	bool CommandCb(HWND wnd, int CtrlID, int NotifCode)
	{
		for (UiObjectW* Obj : UiElements)
		{
			if (Obj->ID == CtrlID)
			{
				return Obj->HandleWndCommand(wnd, NotifCode);
			}
		}

		return false;
	}


	bool ScrollCb(HWND wnd, int HIWORD, int LOWORD)
	{
		for (UiObjectW* Obj : UiElements)
		{
			if (Obj->ObjectWnd.Wnd == wnd)
			{
				return Obj->HandleScrollCommand(HIWORD, LOWORD);
			}
		}

		return false;
	}


	bool ColorCtlCb(HWND wnd, UINT uMsg, WPARAM wParam, LPARAM lParam, HBRUSH* OutBrush)
	{
		if ((HDC)wParam == NULL || (HDC)wParam == INVALID_HANDLE_VALUE || (HWND)lParam == NULL || (HWND)lParam == INVALID_HANDLE_VALUE)
			return false;

		for (UiObjectW* Obj : UiElements)
		{
			if (Obj->ObjectWnd.Wnd == (HWND)lParam)
			{
				return Obj->HandleColorCommand((HDC)wParam, OutBrush);
			}
		}

		return false;
	}


	void AddPushButton(int x, int y, int Width, int Height, ButtonStyles Style, std::wstring ButtonText, std::function<void()> ButtonClick)
	{
		UiElements.push_back(new UiPushButtonW(this->ParentWindow, x, y, Width, Height, Style, ButtonText, StartID + *UIPPIDCounter, ButtonClick));
		*UIPPIDCounter += 1;
	}


	void AddRadioButton(int x, int y, int Width, int Height, RadioButtonStyles Style, std::wstring ButtonText, bool* IsChecked, std::function<void(bool*)> ButtonClick = nullptr)
	{
		UiElements.push_back(new UiRadioButtonW(this->ParentWindow, x, y, Width, Height, Style, ButtonText, StartID + *UIPPIDCounter, IsChecked, ButtonClick));
		*UIPPIDCounter += 1;
	}


	void AddCheckBox(int x, int y, int Width, int Height, CheckboxStyles Style, std::wstring ButtonText, bool* IsChecked, std::function<void(bool*)> ButtonClick = nullptr)
	{
		UiElements.push_back(new UiCheckBoxW(this->ParentWindow, x, y, Width, Height, Style, ButtonText, StartID + *UIPPIDCounter, IsChecked, ButtonClick));
		*UIPPIDCounter += 1;
	}


	void AddEditControl(int x, int y, int Width, int Height, EditControlStyles Style, std::wstring StartString, std::wstring* InputBuffer, std::function<void(std::wstring*)> TextChanged)
	{
		UiElements.push_back(new UiTextInputW(this->ParentWindow, x, y, Width, Height, Style, StartString, StartID + *UIPPIDCounter, InputBuffer, TextChanged));
		*UIPPIDCounter += 1;
	}


	void AddFloatSlider(int x, int y, int Width, int Height, SliderStyles Style, float* Value, float Min, float Max)
	{
		UiElements.push_back(new UiSliderFloatW(this->ParentWindow, x, y, Width, Height, Style, StartID + *UIPPIDCounter, Value, Min, Max));
		*UIPPIDCounter += 1;
	}

	void AddIntSlider(int x, int y, int Width, int Height, SliderStyles Style, int* Value, int Min, int Max)
	{
		UiElements.push_back(new UiSliderIntW(this->ParentWindow, x, y, Width, Height, Style, StartID + *UIPPIDCounter, Value, Min, Max));
		*UIPPIDCounter += 1;
	}

	void AddLabel(int x, int y, int Width, int Height, LabelStyles Style, std::wstring LabelText)
	{
		UiElements.push_back(new UiLabelW(this->ParentWindow, x, y, Width, Height, Style, StartID + *UIPPIDCounter, LabelText));
		*UIPPIDCounter += 1;
	}


	void SpawnInputThread()
	{

	}


	void DestroyAllChildren()
	{
		//std::vector<UiObjectW*> Copied = this->UiElements;
		this->UiElements.clear();

		//for (int idx = 0; idx < Copied.size(); idx++)
		//{
		//	this->ParentWindow->DestroyChild(Copied.at(idx)->ObjectWnd.Wnd);
		//}
	}


	~UiPPW()
	{

	}
};

#ifdef UNICODE
#define UiPP UiPPW
#else
#define UiPP UiPPA
#endif // UNICODE