#pragma once
#include <d3d9.h>
#include "../imgui/imgui.h"
#include "../Core/Core.h"

namespace ui {
	void init(LPDIRECT3DDEVICE9);
	void render();
}

class Dot
{
public:
	Dot();
	Dot(std::string name, int x, int y);

	std::string getName() const;
	int getX() const;
	int getY() const;

	bool operator ==(const Dot& dot);

private:
	std::string _name = "";
	int _x = -1;
	int _y = -1;

};

class Item
{
public:
	Item();
	Item(int type, Dot* fDot, Dot* sDot);

	Dot* _fDot = nullptr;
	Dot* _sDot = nullptr;
	int _type = 0;
	std::string _name;
	std::string _voltage;
	std::string _current;
	std::string _resistance;

	bool operator ==(const Item& item);
};
namespace ui {
	inline LPDIRECT3DDEVICE9 dev;
	inline const char* window_title = "Loader base";
}

namespace ui {
	
	inline ImVec2 screen_res{ 000, 000 };
	inline ImVec2 window_pos{ 0, 0 };
	inline ImVec2 window_size{ 400, 300 };
	inline DWORD  window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
}



class CircuitGui 
{
public:

	static CircuitGui& getInstance() {
		static CircuitGui instance; // Singleton instance
		return instance;
	}
	void HandleInput(ImVec2 canvasPos, ImVec2 canvasSize);
	void DrawCircuit(ImDrawList* drawList, ImVec2 canvasPos);
	bool DrawItemInfo(Item& item);
	enum Coordination
	{
		HORIZONTAL,
		VERTICAL,
		CO_RIGHT,
		CO_LEFT,
		CO_DOWN,
		CO_UP,
	};

	enum ItemsType
	{
		NONE,
		BLANK,
		WIRE,
		RESISTOR,
		VOLTAGE,
	};

public:
	CircuitGui();
	~CircuitGui();

private:
	void addItem(int type, Dot* firstDot, Dot* secondDot);
	void checkKeyEvent();
	void checkMouseEvent();
	void selectWire();
	void selectResistor();
	void selectVoltageSource();
	void selectBlank();

private:
	void drawTitleBar();
	void drawToolBar();
	void drawField();
	void drawBorders();
	void drawItemEditDialog(Item* item);
	void drawAllItems();


private:
	void createAndSolveCircuit();

	void updateItem(Item* item);
	Dot* getNearDot(int x, int y);

	int getCoord(Dot* dot1, Dot* dot2) const;
	double getDistance(Dot* dot1, Dot* dot2) const;
	double getDistance(int x1, int y1, int x2, int y2) const;
	bool hasMouseOverlap(Item& item, ImVec2 mousePos, ImVec2 canvasPos) const;
	double roundDecimal(double number) const;
	std::string removeNumberTrailer(std::string number) const;

public:
	const int _screenWidth = 900;
	const int _screenHeight = 600;

	const int _titleBarWidth = 900;
	const int _titleBarHeight = 30;

	const int _toolBarOffsetX = 30;
	const int _toolBarOffsetY = 40;
	const int _toolBarWidth = 110;
	const int _toolBarHeight = 460;
	const int _dotDistance = 40;


	mf::LinkedList<Dot> _dots;
	mf::LinkedList<Item> _itemsList;

	CircuitCore* _circuitCore = nullptr;
	Item* _overlappedItem = nullptr;
	Dot* _firstDotSelected = nullptr;
	Dot* _secondDotSelected = nullptr;

	int _error = -1;
	int _selectedItem = NONE;
	bool _editDialogOpen = false;
	std::string _editDialogTextBoxString = "";
	bool _decimal = false;
	
};
