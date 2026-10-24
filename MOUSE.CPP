#include "mouse.h"
#include "int33.h"

TMouse::TMouse()
{
    visible = 0;
    int33_set_x_range(0, 639);
    int33_set_y_range(0, 199);
    setMousePosition(159, 99);
    leftDown = 0;
    rightDown = 0;
    show();
}

TMouse::~TMouse()
{
    hide(0, 0, 639, 479);
}


int TMouse::checkInside(int x1, int y1, int x2, int y2)
{
    if ((x >= x1) && (x <= x2) && (y >= y1) && (y <= y2))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void TMouse::hide(int x1, int y1, int x2, int y2)
{
    if (visible)
    {
        update();
        if ((x > x1) && (x < x2) && (y > y1) && (y < y2))
        {
            visible = 0;
            int33_hide_cursor();
        }
    }
}

void TMouse::show()
{
    if (!visible)
    {
        visible = 1;
        int33_show_cursor();
    }
}

void TMouse::hide()
{
    if (visible)
    {
        {
            visible = 0;
            int33_hide_cursor();
        }
    }
}

void TMouse::setMousePosition(int x, int y)
{
    int33_set_position(x, y);
}

void TMouse::update()
{
    unsigned int xPos,yPos,bState;
    int33_get_status(&bState, &xPos, &yPos);
    if (bState == 0)
    {
        oldLeftDown = leftDown;
        leftDown = 0;
        oldRightDown = rightDown;
        rightDown = 0;
    }
    if (bState == 1)
    {
        oldLeftDown = leftDown;
        leftDown = 1;
    }
    if (bState == 2)
    {
        oldRightDown = rightDown;
        rightDown = 1;
    }
    if (bState == 3)
    {
        oldLeftDown = leftDown;
        leftDown = 1;
        oldRightDown = rightDown;
        rightDown = 1;
    }
    x = xPos / 2;
    y = yPos;
}








