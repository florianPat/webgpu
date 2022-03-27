#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include "String.h"
#include "RenderTexture.h"
#include "Graphics.h"
#include "Vector2.h"

class Font
{
    struct GlyphRegion
    {
        Vector2f bitmapTopLeft;
        Vector2ui xy;
        Vector2ui size;
        uint32_t advanceX = 0;
    };
private:
    static constexpr int32_t NUM_GLYPHS = '~' - ' ' + 1;
    uint32_t size = 0;
    uint32_t faceHeight = 0;
    RenderTexture renderTexture;
    //NOTE: Stack overflow danger?
    GlyphRegion regions[NUM_GLYPHS];
    FT_Library library = nullptr;
    Graphics2D* gfx = nullptr;
private:
    bool createGlyphRenderTextureAndMap(FT_Face& face);
    bool loadFaceFromLibrary(const void* fileBuffer, uint64_t fileSize, FT_Face& face);
    void destructFace(FT_Face& face);
    bool loadGlyphIntoMap(char c, GlyphRegion& glyphRegion, FT_Face& face, Vector2ui& xy);
public:
    bool reloadFromFile(const String& filename);
public:
    Font(const String& filename, int32_t size = 0);
    Font(const Font& other) = delete;
    Font(Font&& other) noexcept;
    Font& operator=(const Font& rhs) = delete;
    Font& operator=(Font&& rhs) noexcept;
    uint64_t getSize() const { return (sizeof(Font)); }
    void drawText(const String& text, const Vector2f& pos, Color color = Colors::White);
    void drawText(const String& text, const Vector2f& pos, int32_t pixelSize, Color color = Colors::White);
    explicit operator bool() const;
    Vector2f computeTextDimension(const String& text, int32_t pixelSize);
};