#ifndef __TEXT_UTILS_H__
#define __TEXT_UTILS_H__
#include <raylib.h>

extern Font LoadCyrillicTTF(const char* fontname, int fontsize);

extern void DrawTextBoxed(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint);
// Draw text using font inside rectangle limits with support for text selection
extern void DrawTextBoxedSelectable(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint);

#endif