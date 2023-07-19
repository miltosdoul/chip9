#include "lib/types.h"
#include "chip9/chip9.h"

static const bool THROTTLED = false;
static const int FPS = 60;

static const bool SCALED = true;
static const int SCALING = SCALED ? 10 : 0;

// Improves FPS for lower resolutions
static const bool DRAW_ALL_POINTS_AT_ONCE = false;
// Use rects to draw scaled output, needs "SCALED" to be true
// Improves performance
static const bool USE_RECTS = true;
// Requires SCALED and USE_RECTS to be true
// Improves performance at scale
static const bool DRAW_ALL_RECTS_AT_ONCE = true;

u16 IBM_LOGO[] = {
    0x00e0, 0xa22a, 0x600c, 0x6108, 0xd01f, 0x7009, 0xa239, 0xd01f,
    0xa248, 0x7008, 0xd01f, 0x7004, 0xa257, 0xd01f, 0x7008, 0xa266,
    0xd01f, 0x7008, 0xa275, 0xd01f, 0x1228, 0xff00, 0xff00, 0x3c00,
    0x3c00, 0x3c00, 0x3c00, 0xff00, 0xffff, 0x00ff, 0x0038, 0x003f,
    0x003f, 0x0038, 0x00ff, 0x00ff, 0x8000, 0xe000, 0xe000, 0x8000,
    0x8000, 0xe000, 0xe000, 0x80f8, 0x00fc, 0x003e, 0x003f, 0x003b,
    0x0039, 0x00f8, 0x00f8, 0x0300, 0x0700, 0x0f00, 0xbf00, 0xfb00,
    0xf300, 0xe300, 0x43e0, 0x00e0, 0x0080, 0x0080, 0x0080, 0x0080,
    0x00e0, 0x00e0
};

int main(int argc, char** argv) {
    Chip9* chip9 = new Chip9(THROTTLED,
                             FPS,
                             SCALED,
                             SCALING,
                             DRAW_ALL_POINTS_AT_ONCE,
                             USE_RECTS,
                             DRAW_ALL_RECTS_AT_ONCE);

    return chip9->run(IBM_LOGO);
}