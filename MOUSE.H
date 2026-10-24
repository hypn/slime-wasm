#ifndef _MOUSE_H_
#define _MOUSE_H_

const int MOUSE_ENTERED       = 1;
const int MOUSE_LEFT          = 2;
const int MOUSE_OUTSIDE       = 3;
const int MOUSE_INSIDE        = 4;
const int BUTTON_DOWN         = 1;
const int BUTTON_UP           = 2;
const int BUTTON_STILL_DOWN   = 3;
const int BUTTON_STILL_UP     = 4;

class TMouse
{
public:

                    TMouse();
                    ~TMouse();
    void            show();
    void            hide();
    void            hide(int x1, int x2, int y1, int y2);
    void            setMousePosition(int x, int y);
    void            update();
    int             checkInside(int x1, int y1, int x2, int y2);
    unsigned int    x, y;
    unsigned int    leftDown, rightDown;
    unsigned int    oldLeftDown, oldRightDown;
    int             leftClickState, rightClickState;

private:
    int             visible;
};

#endif