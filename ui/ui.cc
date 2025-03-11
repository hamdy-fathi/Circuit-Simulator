#include "ui.hh"
#include "../globals.hh"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include <string>
#include <algorithm>
#include <unordered_set>
#include <queue>
#include <iostream>
int currentValue = 0;
ImVec2 currentStart;                  // Temporary storage for the start of the current element
bool isDrawing = false;               // Is the user currently drawing an element?
// Snapping grid size
const int GRID_SIZE = 20;
// Define a structure for a line segment
class Dot;
class Item;
class CircuitGui;

bool overlapFound = false;
/*
================= Public realization of class Dot =================
*/

// Dots are invisible on the screen by default

Dot::Dot() {}

Dot::Dot(std::string name, int x, int y) : _name(name), _x(x), _y(y) {}

std::string Dot::getName() const { return _name; }

int Dot::getX() const { return _x; }

int Dot::getY() const { return _y; }

bool Dot::operator ==(const Dot& dot)
{
    if (_name == dot._name)
        return true;
    return false;
}

/*
================= Public realization of class Item =================
*/

Item::Item() { }

bool Item::operator==(const Item& item)
{
    if (_fDot == item._fDot && _sDot == item._sDot)
        return true;
    if (_fDot == item._sDot && _sDot == item._fDot)
        return true;

    return false;
}

Item::Item(int type, Dot* fDot, Dot* sDot)
{
    _type = type;
    _fDot = fDot;
    _sDot = sDot;
    _voltage = "0";
    _current = "0";
    _resistance = "0";

    switch (_type)
    {
    case CircuitGui::ItemsType::WIRE:
        _name = "Wire" + fDot->getName() + sDot->getName();
        break;
    case CircuitGui::ItemsType::RESISTOR:
        _name = "Resistor" + fDot->getName() + sDot->getName();
        break;
    case CircuitGui::ItemsType::VOLTAGE:
        _name = "Battery" + fDot->getName() + sDot->getName();
        break;
    };
}
CircuitGui::CircuitGui() {
    // Initialization code (if any)
    
    CircuitCore& _circuitCore = CircuitCore::getInstance();
}

// Destructor definition
CircuitGui::~CircuitGui() {
    // Cleanup code (if any)
    //delete _circuitCore;
}

void CircuitGui::addItem(int type, Dot* firstDot, Dot* secondDot)
{
    Item* foundItem = _itemsList.find(Item(type, firstDot, secondDot));

    if (!foundItem)
    {
        if (type != BLANK)
        {
            Item item(type, firstDot, secondDot);

            if (type == VOLTAGE) item._voltage = std::to_string(currentValue);
            if (type == RESISTOR) item._resistance = std::to_string(currentValue);
        
            _itemsList.pushBack(item);
            createAndSolveCircuit();
          //  createAndSolveCircuit();
        }
    }

    if (foundItem)
    {
        if (type == BLANK)
        {
            // Save it's name, because when we remove it from draw list,
            // it will be gone and we can't use it later for circuit
            std::string name = foundItem->_name;

            try
            {
                _itemsList.remove(*foundItem);
                createAndSolveCircuit();
            }
            catch (CircuitCore::Errors error)
            {
                _error = error;
            }
        }
    }
}
double CircuitGui::getDistance(Dot* dot1, Dot* dot2) const
{
    return getDistance(dot1->getX(), dot1->getY(), dot2->getX(), dot2->getY());
}

double CircuitGui::getDistance(int x1, int y1, int x2, int y2) const
{
    double t1 = pow(x1 - x2, 2);
    double t2 = pow(y1 - y2, 2);

    return sqrt(t1 + t2);
}

bool CircuitGui::hasMouseOverlap(Item& item,ImVec2 mousePos,ImVec2 canvasPos) const
{
    ImVec2 relativeMousePos = ImVec2(mousePos.x - canvasPos.x, mousePos.y - canvasPos.y);


    double dotsDistance = getDistance(item._fDot, item._sDot);


    double d1 = getDistance(relativeMousePos.x, relativeMousePos.y, item._fDot->getX(), item._fDot->getY());
    double d2 = getDistance(relativeMousePos.x, relativeMousePos.y, item._sDot->getX(), item._sDot->getY());

    double chord = sqrt(pow(d1, 2) + pow(d2, 2));

    if (chord <= dotsDistance)
        return true;

    return false;
}

double CircuitGui::roundDecimal(double number) const
{
    return std::ceil(number * 1000.0) / 1000.0;
}


ImDrawList* drawList;



// Function to snap a point to the nearest grid intersection



// Function to draw the snapping grid
void DrawGrid(ImDrawList* drawList, ImVec2 canvasPos, ImVec2 canvasSize) {
    const ImU32 gridColor = IM_COL32(100, 100, 100, 255);
    float gridOffsetY = 0.0f;
    for (float x = canvasPos.x; x < canvasPos.x + canvasSize.x; x += GRID_SIZE) {
        drawList->AddLine(ImVec2(x, canvasPos.y), ImVec2(x, canvasPos.y + canvasSize.y), gridColor);
    }
    for (float y = canvasPos.y + gridOffsetY; y < canvasPos.y + canvasSize.y; y += GRID_SIZE) {
        drawList->AddLine(ImVec2(canvasPos.x, y), ImVec2(canvasPos.x + canvasSize.x, y), gridColor);
    }
}

void DrawVoltageSource(ImDrawList* drawList,  ImVec2& center, std::string value, bool isVertical) {
    float radius = 10.0f; // Radius of the voltage source symbol
    ImU32 Color = IM_COL32(255, 0, 0, 255);
    ImU32 ColorGreen = IM_COL32(0, 255, 0, 255);
    // Draw the outer circle
    drawList->AddCircle(center, radius,Color, 12, 2.0f); // Circle in yellow

    if (isVertical) {
        // Draw positive terminal (top)
        drawList->AddLine(center + ImVec2(0, -radius), center + ImVec2(0, -radius - 10),Color, 2.0f);
        drawList->AddLine(center + ImVec2(-5, -radius - 10), center + ImVec2(5, -radius - 10),Color, 2.0f);

        // Draw negative terminal (bottom)
        drawList->AddLine(center + ImVec2(0, radius), center + ImVec2(0, radius + 10),Color, 2.0f);
        drawList->AddLine(center + ImVec2(-5, radius + 10), center + ImVec2(5, radius + 10),Color, 2.0f);
    }
    else {
        // Draw positive terminal (right)
        drawList->AddLine(center + ImVec2(radius, 0), center + ImVec2(radius + 10, 0),Color, 2.0f);
        drawList->AddLine(center + ImVec2(radius + 10, -5), center + ImVec2(radius + 10, 5),Color, 2.0f);

        // Draw negative terminal (left)
        drawList->AddLine(center + ImVec2(-radius, 0), center + ImVec2(-radius - 10, 0),Color, 2.0f);
        drawList->AddLine(center + ImVec2(-radius - 10, -5), center + ImVec2(-radius - 10, 5),Color, 2.0f);
    }

    center.x -= 7;
    center.y -= 7;

    // Draw voltage value
    drawList->AddText(center,Color, ("+-"));

    center.y -= 18;
    center.x += 2;
    drawList->AddText(center, ColorGreen,value.append("V").c_str());
}
int CircuitGui::getCoord(Dot* firstDot, Dot* secondDot) const
{
    if (secondDot->getX() > firstDot->getX())
        return CO_RIGHT;
    if (secondDot->getX() < firstDot->getX())
        return CO_LEFT;
    if (secondDot->getY() > firstDot->getY())
        return CO_DOWN;
    if (secondDot->getY() < firstDot->getY())
        return CO_UP;
}
// Step 5: Draw a zigzag resistor between two points
void DrawZigzagResistor(ImDrawList* drawList, const ImVec2& start, const ImVec2& end, float value) {
    const float zigzagHeight = 10.0f;  // Fixed height of each zigzag (vertical displacement)
    const float zigzagWidth = 10.0f;   // Fixed width of each zigzag (horizontal displacement)

    float totalLength = std::hypot(end.x - start.x, end.y - start.y);  // Diagonal distance between points

    // Number of zigzags, based on the total length and the fixed zigzag width
    int numZags = static_cast<int>(totalLength / zigzagWidth);
    if (numZags < 2) numZags = 2;  // Ensure at least two zigzags are drawn

    float segmentLength = totalLength / numZags;  // Length of each zigzag segment

    ImVec2 currentPos = start;
    bool isUpward = true;  // Toggle the direction of the zigzags

    // Determine the type of zigzag (horizontal or vertical)
    bool isHorizontal = abs(end.x - start.x) > abs(end.y - start.y);

    // Draw the zigzag resistor
    for (int i = 0; i < numZags; i++) {
        float angle = std::atan2(end.y - start.y, end.x - start.x);  // Angle to the target position
        float dx = std::cos(angle) * segmentLength;  // Calculate the horizontal segment length
        float dy = std::sin(angle) * segmentLength;  // Calculate the vertical segment length

        // Move to the next point in the zigzag
        ImVec2 nextPos = ImVec2(currentPos.x + dx, currentPos.y + dy);

        // Zigzag along the Y (for horizontal placement)
        if (isHorizontal) {
            float yOffset = isUpward ? zigzagHeight : -zigzagHeight;
            nextPos.y += yOffset;
        }
        else {
            // Zigzag along the X (for vertical placement)
            float xOffset = isUpward ? zigzagWidth : -zigzagWidth;
            nextPos.x += xOffset;
        }

        drawList->AddLine(currentPos, nextPos, IM_COL32(0, 0, 0, 255), 2.0f);  // Draw the line for the zigzag
        currentPos = nextPos;

        // Toggle the zigzag direction
        isUpward = !isUpward;
    }

    // Label the resistor value in the middle of the resistor
    ImVec2 mid = (start + end) * 0.5f;

    if (isHorizontal)
        mid.y -= 15;
    else
        mid.x += 10;
    drawList->AddText(mid, IM_COL32(255, 0, 0, 255), ("R=" + std::to_string(static_cast<int>(value))).c_str());
}
// Step 6: Handle placing the resistor at different angles based on mouse movement


void CircuitGui::createAndSolveCircuit()
{
  
    for (Item& item : _itemsList)
    {
        try
        {
            switch (item._type)
            {
            case WIRE:
                CircuitCore::getInstance().addWire(item._name, item._fDot->getName(), item._sDot->getName());
                break;
            case RESISTOR:
                CircuitCore::getInstance().addResistor(item._name, std::stod(item._resistance), item._fDot->getName(), item._sDot->getName());
             
                break;
            case VOLTAGE:
                CircuitCore::getInstance().addBattery(item._name, std::stod(item._voltage), item._fDot->getName(), item._sDot->getName());
                break;
            }
        }
        catch (CircuitCore::Errors error)
        {
            _error = error;
        }
    }

    try
    {
        CircuitCore::getInstance().solve();
    }
    catch (CircuitCore::Errors error)
    {
        delete _circuitCore;
        _circuitCore = nullptr;
        _error = error;
    }
}

void DrawConnectionPoints(ImDrawList* drawList, ImVec2 point) {
    const float radius = 5.0f;  // Size of node
    const ImU32 nodeColor = IM_COL32(0, 255, 0, 255);  // Green color for nodes
    drawList->AddCircleFilled(point, radius, nodeColor);
}

ImVec2 SnapToGrid(ImVec2 point) {

    float snappedY = round(point.y / GRID_SIZE) * GRID_SIZE;
    if(snappedY < 0.0f)
        snappedY = 0.0f;
    return ImVec2(round(point.x / GRID_SIZE) * GRID_SIZE,snappedY );
}

Dot* CircuitGui::getNearDot(int x, int y) {
    Dot* closestDot = nullptr;
    float minDist = FLT_MAX;

    for (Dot& dot : _dots) {
        printf("Existing Dot: (%.2f, %.2f)\n", dot.getX(), dot.getY());
        float dist = getDistance(x, y, dot.getX(), dot.getY());
        if (dist < _dotDistance) {
            if (dist < minDist) {
                minDist = dist;
                closestDot = &dot;
            }
        }
    }
    return closestDot;
}

void CircuitGui::HandleInput(ImVec2 canvasPos, ImVec2 canvasSize) {
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 relativeMousePos = mousePos - canvasPos;

    if (relativeMousePos.x < 0 || relativeMousePos.x > canvasSize.x ||
        relativeMousePos.y < 0 || relativeMousePos.y > canvasSize.y) return;

    if (ImGui::IsMouseClicked(0)) {
        ImVec2 snapped = SnapToGrid(relativeMousePos);
        _firstDotSelected = getNearDot(snapped.x, snapped.y);
        printf("Mouse Clicked: (%.2f, %.2f) -> Snapped: (%.2f, %.2f), Relative: (%.2f, %.2f)\n",
            mousePos.x, mousePos.y, snapped.x, snapped.y, relativeMousePos.x, relativeMousePos.y);
        if (_firstDotSelected) {
            currentStart = ImVec2(_firstDotSelected->getX(), _firstDotSelected->getY());
            isDrawing = true;
        }
    }

    if (ImGui::IsMouseReleased(0) && isDrawing) {
        ImVec2 snapped = SnapToGrid(relativeMousePos);
        _secondDotSelected = getNearDot(snapped.x, snapped.y);

        if (_firstDotSelected && _secondDotSelected && _firstDotSelected != _secondDotSelected) {
            addItem(_selectedItem, _firstDotSelected, _secondDotSelected);
        }

        isDrawing = false;
    }

}

void CircuitGui::DrawCircuit(ImDrawList* drawList, ImVec2 canvasPos) {


    
    bool overlapFound = false;
    _overlappedItem = nullptr;

    {
        for (Item& item : _itemsList) {
            ImVec2 mousePos = ImGui::GetMousePos();
             if(hasMouseOverlap(item, mousePos, canvasPos) && !overlapFound && item._type!= WIRE && CircuitCore::getInstance().FinalVoltage) {
                //  MessageBox(0, "OverLapping ", 0, 0);
                DrawItemInfo(item);
                overlapFound = true;
                _overlappedItem = &item;
            }
            
            {
                if (!item._fDot || !item._sDot) continue;
                ImVec2 start = SnapToGrid(ImVec2(item._fDot->getX(), item._fDot->getY())) + canvasPos;
                ImVec2 end = SnapToGrid(ImVec2(item._sDot->getX(), item._sDot->getY())) + canvasPos;

                if (item._type == WIRE) {
                    drawList->AddLine(start, end, IM_COL32(255, 255, 255, 255), 2.0f);
                }
                else if (item._type == RESISTOR) {
                    DrawZigzagResistor(drawList, start, end, std::stod(item._resistance));
                }
                else if (item._type == VOLTAGE)
                {
                    bool isVertical = (item._fDot->getX() == item._sDot->getX());
                    ImVec2 mid = (ImVec2(item._fDot->getX(), item._fDot->getY()) +
                        ImVec2(item._sDot->getX(), item._sDot->getY())) * 0.5f + canvasPos;
                    mid = (mid - canvasPos) * 1.0 + canvasPos;
                    DrawVoltageSource(drawList, mid, item._voltage, isVertical);
                }
            }

        }

        if (isDrawing) {
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 relativeMousePos = mousePos - canvasPos;

            // If there is a zoom factor, adjust the cursor position accordingly.
            relativeMousePos = (relativeMousePos - canvasPos) * (1.0f / 1.0) + canvasPos;

            // Snap the mouse position to the grid
            ImVec2 snappedMousePos = SnapToGrid(relativeMousePos);

            // Draw the wire from the start position to the snapped position.
            drawList->AddLine(currentStart + canvasPos, snappedMousePos + canvasPos, IM_COL32(255, 255, 0, 255), 2.0f);
        }
}
}

std::string CircuitGui::removeNumberTrailer(std::string number) const
{
    if (number.find('.') == std::string::npos) return number;

    while (true)
    {
        if (number[number.length() - 1] == '0') number.resize(number.length() - 1);
        else
        {
            break;
        }

        if (number[number.length() - 1] == '.')
        {
            number.resize(number.length() - 1);
            break;
        }
    }

    return number;
}

bool CircuitGui::DrawItemInfo(Item & item)
{
    ImGui::Begin("Item Info");
   // ImGui::SetCursorPos(ImVec2(400, 130));
    Element* element = CircuitCore::getInstance().searchElement(item._name);
    if (element == nullptr)	return false;
    item._voltage = removeNumberTrailer(std::to_string(roundDecimal(abs(element->getVoltage()))));
    item._current = removeNumberTrailer(std::to_string(roundDecimal(abs(element->getCurrent()))));
    item._resistance = removeNumberTrailer(std::to_string(roundDecimal(abs(element->getResistance()))));
    ImGui::Text("Current ::%.2f Ampere", element->getCurrent());

    ImGui::Text("Resistance ::%.2f Ohm" , element->getResistance());
    ImGui::Text("Voltage ::%.2f Volt", element->getVoltage());
    ImGui::End();
    return true;

}
bool doonce = true;
void CircuitSimulator() {
    AllocConsole();
    CircuitGui& gui = CircuitGui::getInstance();
   
  
    // Redirect the output to the new console window
    FILE* pCout;
    freopen_s(&pCout, "CONOUT$", "w", stdout);

   /* CircuitCore::getInstance().printElements();
    CircuitCore::getInstance().printNodes();
    CircuitCore::getInstance().printConnections();*/

    // Now you can print 
    // 
    // Create a window for circuit drawing
    ImGui::SetNextWindowSize(ImVec2(700, 500));
    ImGui::Begin("Circuit Simulator", nullptr, ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoMove);


    ImGui::Separator();
   // std::cout << CircuitCore::getInstance().EquivalentResistance;
   // ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(rg198, 40, 40, 255));
    ImGui::Text("Element Type :" , CircuitCore::getInstance().EquivalentResistance);
  //  ImGui::PopStyleColor();

  
   // ImGui::Text("S::%f ", parallelR);
    if (ImGui::RadioButton("Wire", gui._selectedItem ==gui.WIRE)) {
        gui._selectedItem = gui.WIRE;
        currentValue = 0.0f;
    }
    if (ImGui::RadioButton("Resistance", gui._selectedItem == gui.RESISTOR)) {
        gui._selectedItem = gui.RESISTOR;
    }
   
    if (gui._selectedItem == gui.RESISTOR)
    {
        ImGui::SameLine();
        ImGui::PushItemWidth(150);
        ImGui::InputInt("Ohm", &currentValue);
    }
    if (ImGui::RadioButton("Voltage", gui._selectedItem == gui.VOLTAGE)) {
        gui._selectedItem = gui.VOLTAGE;
    }
    if (gui._selectedItem == gui.VOLTAGE)
    {
        ImGui::SameLine();
        ImGui::PushItemWidth(150);
        ImGui::InputInt("Volt", &currentValue);
    }

    if (ImGui::RadioButton("Delete Element", gui._selectedItem == gui.BLANK)) {
        gui._selectedItem = gui.BLANK;  // Toggle deletion mode
    }
    if (gui._selectedItem != gui.WIRE) {
     //   ImGui::InputFloat("Value", &currentValue);
    }

    ImGui::Separator();
  

    
    // Get the drawing list for this window
     drawList = ImGui::GetWindowDrawList();

    
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + 100 , canvasPos.y+100), IM_COL32(255, 0, 0, 255));
    // Draw a filled background
    drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
        IM_COL32(50, 50, 50, 255));

    // Draw the snapping grid
    DrawGrid(drawList, canvasPos, canvasSize);

   
    ImGui::SetCursorPos(ImVec2(400, 50));

    ImGui::Text("Equivalent Resistance :: %.2f Ohm", CircuitCore::getInstance().EquivalentResistance);

    ImGui::SetCursorPos(ImVec2(400, 70));
    ImGui::Text("Equivalent Voltage :: %.2f Volt", CircuitCore::getInstance().FinalVoltage);

    ImGui::SetCursorPos(ImVec2(400, 90));
    ImGui::Text("Equivalent Current :: %.2f Ampere", CircuitCore::getInstance().FinalCurrent);


   




    if (doonce == true)
    {

        int startX = 20;
        int startY = 20;
        for (int i = 0; i < 14; ++i)
        {
            for (int j = 0; j < 19; ++j)
            {
                std::string name = "Dot";
                name += std::to_string(i) + "-" + std::to_string(j);
                gui._dots.pushBack(Dot(name, startX + gui._dotDistance * j, startY + gui._dotDistance * i));
            }
        }

        doonce = false;
    }
    gui.HandleInput(canvasPos, canvasSize);
 



    // Set up a canvas for drawing
    ImGui::PushClipRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), true);

    
   // IdentifyNodesAndCalculateResistance(elements);
    // Draw the circuit
    gui.DrawCircuit(drawList,canvasPos);
   
  
    // Display node indices using ImGui
 
    //DetectSeriesAndParallelResistors();

    ImGui::PopClipRect();
    ImGui::End();
}

void ui::render() {
    if (!globals.active) return;

	CircuitSimulator();
}

void ui::init(LPDIRECT3DDEVICE9 device) {
    dev = device;
	
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
   
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.WindowRounding = 5.0f;
    style.WindowBorderSize = 1.0f;
    style.WindowMinSize = ImVec2(32.0f, 32.0f);
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ChildRounding = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.FrameRounding = 5.0f;
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);
    style.IndentSpacing = 20.0f;
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 12.89999961853027f;
    style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 8.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 4.0f;
    style.TabBorderSize = 1.0f;
    style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    style.Colors[ImGuiCol_Text] = ImVec4(0.9019607901573181f, 0.7058823704719543f, 0.3137255012989044f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.9019607901573181f, 0.7058823704719543f, 0.3137255012989044f, 0.501960813999176f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.03921568766236305f, 0.05490196123719215f, 0.0784313753247261f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f, 0.9399999976158142f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.07450980693101883f, 0.09019608050584793f, 0.1294117718935013f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.250980406999588f, 0.2588235437870026f, 0.2784313857555389f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.250980406999588f, 0.2588235437870026f, 0.2784313857555389f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.501960813999176f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.05882352963089943f, 0.07450980693101883f, 0.1019607856869698f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.501960813999176f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.04313725605607033f, 0.05490196123719215f, 0.0784313753247261f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f, 0.5299999713897705f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.407843142747879f, 0.407843142747879f, 0.407843142747879f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5098039507865906f, 0.5098039507865906f, 0.5098039507865906f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.2470588237047195f, 0.6980392336845398f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.9019607901573181f, 0.7058823704719543f, 0.3137255012989044f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.5607843399047852f, 0.250980406999588f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.250980406999588f, 0.2588235437870026f, 0.2784313857555389f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3098039329051971f, 0.3176470696926117f, 0.3372549116611481f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.250980406999588f, 0.2588235437870026f, 0.2784313857555389f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.250980406999588f, 0.2588235437870026f, 0.2784313857555389f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3098039329051971f, 0.3176470696926117f, 0.3372549116611481f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.3098039329051971f, 0.3176470696926117f, 0.3372549116611481f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.250980406999588f, 0.2588235437870026f, 0.2784313857555389f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3098039329051971f, 0.3176470696926117f, 0.3372549116611481f, 1.0f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.250980406999588f, 0.2588235437870026f, 0.2784313857555389f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2470588237047195f, 0.6980392336845398f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.1333333402872086f, 0.4117647111415863f, 0.5490196347236633f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.1333333402872086f, 0.4117647111415863f, 0.5490196347236633f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.07450980693101883f, 0.09019608050584793f, 0.1294117718935013f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.250980406999588f, 0.2588235437870026f, 0.2784313857555389f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.250980406999588f, 0.2588235437870026f, 0.2784313857555389f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667014360428f, 0.1019607856869698f, 0.1450980454683304f, 0.9724000096321106f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1333333402872086f, 0.2588235437870026f, 0.4235294163227081f, 1.0f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.2470588237047195f, 0.6980392336845398f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.2470588237047195f, 0.6980392336845398f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.1333333402872086f, 0.4117647111415863f, 0.5490196347236633f, 1.0f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.250980406999588f, 0.2588235437870026f, 0.2784313857555389f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.250980406999588f, 0.2588235437870026f, 0.2784313857555389f, 1.0f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.03921568766236305f, 0.05490196123719215f, 0.0784313753247261f, 1.0f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.03921568766236305f, 0.05490196123719215f, 0.0784313753247261f, 1.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.06666667014360428f, 0.1098039224743843f, 0.1607843190431595f, 1.0f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.2470588237047195f, 0.6980392336845398f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.9764705896377563f, 0.2588235437870026f, 0.2588235437870026f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);

	if (window_pos.x == 0) {
		RECT screen_rect{};
		GetWindowRect(GetDesktopWindow(), &screen_rect);
		screen_res = ImVec2(float(screen_rect.right), float(screen_rect.bottom));
		window_pos = (screen_res - window_size) * 0.5f;

		// init images here
	}
}