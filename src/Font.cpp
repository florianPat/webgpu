#include "Font.h"
#include "Ifstream.h"
#include "Globals.h"
#include FT_IMAGE_H
#include "Sprite.h"
#include "Window.h"

bool Font::createGlyphRenderTextureAndMap(FT_Face& face)
{
    assert(gfx != nullptr);

    //NOTE: -1 because ' ' does not get rendered!
    uint32_t renderTextureSize = (uint32_t) size * (NUM_GLYPHS - 1);

    uint32_t maxTextureSize = 0;
    uint32_t renderTextureRowSize = size;
#ifdef OGL_GRAPHICS
    //glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
#else
    maxTextureSize = GraphicsVKIniter::max2dTextureSize;
#endif
    if(renderTextureSize > maxTextureSize)
    {
        uint32_t nRows = (uint32_t)ceil(((float)renderTextureSize) / maxTextureSize);
        assert((nRows * size) < maxTextureSize);
        renderTextureSize = (uint32_t)ceil(((float)renderTextureSize) / nRows);
        renderTextureRowSize = size * nRows;
    }

    new (&renderTexture) RenderTexture(renderTextureSize, renderTextureRowSize, &Globals::window->getGfx(), gfx->id);
    Mat4x4 orthoProj = Mat4x4::orthoProj(0.0f, 0.0f, (float)renderTextureSize, (float)renderTextureRowSize);
    renderTexture.clear();
    gfx->bindOtherFramebuffer(renderTexture.getFramebufferDrawStruct());
    gfx->bindOtherOrthoProj(orthoProj);
    uint32_t* pixels = (uint32_t*) malloc((size_t)size * size * sizeof(uint32_t));

    Vector2ui xy = { 0, 0 };
    Texture texture;
    Sprite sprite;

    if(!loadGlyphIntoMap(' ', regions[' ' - ' '], face, xy))
        return false;

    for(char c = '!'; c <= '~'; ++c)
    {
        GlyphRegion& glyphRegion = regions[c - ' '];
        if(!loadGlyphIntoMap(c, glyphRegion, face, xy))
            return false;

        uint16_t numGrayLevels = face->glyph->bitmap.num_grays;
        for(uint32_t y = 0, pixelY = glyphRegion.size.y - 1; y < glyphRegion.size.y; ++y, --pixelY)
        {
            for(uint32_t x = 0; x < glyphRegion.size.x; ++x)
            {
                uint8_t currentPixelGrayLevel = face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + x];
                //NOTE: Premultiplied alpha for free ;)
                uint8_t color = (uint8_t) ((float) currentPixelGrayLevel / numGrayLevels * 255.0f);
                // NOTE: This needs to be pixelY for bottom up Opengl.
                pixels[y * glyphRegion.size.x + x] = (color | (color << 8u) |
                    (color << 16u) | (color << 24u));
            }
        }

        texture = Texture(pixels, glyphRegion.size.x, glyphRegion.size.y);
        sprite.setTexture(&texture, true);
        sprite.pos = (Vector2f) xy;
        gfx->draw(sprite);
        gfx->flush();

        xy.x += texture.getWidth();
        if(xy.x >= renderTextureSize)
        {
            xy.x = 0;
            xy.y += size;
            assert(xy.y <= renderTextureRowSize);
        }
    }

    gfx->unbindOtherFramebuffer();
    gfx->unbindOtherOrthoProj();
    renderTexture.changeToShaderReadMode();
    free(pixels);
    destructFace(face);

    return true;
}

bool Font::reloadFromFile(const String& filename)
{
    faceHeight = 0;
    renderTexture.~RenderTexture();

    FT_Face face = nullptr;

    library = Globals::window->getFontLibrary();

    Ifstream file(filename);
    assert(file);

    if(!loadFaceFromLibrary(file.getFullData(), file.getSize(), face))
        return false;

    return createGlyphRenderTextureAndMap(face);
}

void Font::drawText(const String& text, const Vector2f& pos, int32_t pixelSize, Color color)
{
    Vector2f pen = { 0.0f, 0.0f };
    float scale = (float)pixelSize / size;
    Sprite sprite;
    sprite.color = color;
    sprite.setTexture(&renderTexture.getTexture());
    sprite.setScale(scale);

    for(uint32_t i = 0; i < text.size(); ++i)
    {
        char c = text[i];

        if(c == '\n')
        {
            pen.y -= faceHeight * scale;
            pen.x = 0;
        }
        else if(c == ' ')
        {
            const GlyphRegion& region = regions[c - ' '];
            pen.x += region.advanceX * scale;
        }
        else
        {
            assert(c >= '!' && c <= '~');
            const GlyphRegion& region = regions[c - ' '];
            // TODO: region.xy.y and region.size.y are wrong. Probably because of the top down switch?
            sprite.setTextureRect(IntRect(region.xy.x + 1, 0, region.size.x - 1, size));

            sprite.pos = pos + pen; //+ region.bitmapTopLeft; TODO: The y is wrong!

            pen.x += region.advanceX * scale;

            gfx->draw(sprite);
        }
    }
}

Font::Font(Font&& other) noexcept : size(std::exchange(other.size, 0)), faceHeight(std::exchange(other.faceHeight, 0)),
                           renderTexture(std::exchange(other.renderTexture, RenderTexture())),
                           library(std::exchange(other.library, nullptr)), gfx(std::exchange(other.gfx, nullptr))
{
    for(int i = 0; i < NUM_GLYPHS; ++i)
    {
        new (&regions[i]) GlyphRegion(std::exchange(other.regions[i], GlyphRegion()));
    }
}

Font& Font::operator=(Font&& rhs) noexcept
{
    this->~Font();

    faceHeight = std::exchange(rhs.faceHeight, 0);
    size = std::exchange(rhs.size, 0);
    renderTexture = std::exchange(rhs.renderTexture, RenderTexture());
    library = std::exchange(rhs.library, nullptr);
    gfx = std::exchange(rhs.gfx, nullptr);

    for(int i = 0; i < NUM_GLYPHS; ++i)
    {
        new (&regions[i]) GlyphRegion(std::exchange(rhs.regions[i], GlyphRegion()));
    }

    return *this;
}

bool Font::loadFaceFromLibrary(const void* fileBuffer, uint64_t fileSize, FT_Face& face)
{
    int32_t error = FT_New_Memory_Face(library, (const uint8_t*) fileBuffer, (FT_Long)fileSize, 0, &face);
    if(error)
    {
        utils::log("could not create face");
        destructFace(face);
        return false;
    }

    error = FT_Set_Pixel_Sizes(face, size, 0);
    if(error)
    {
        utils::log("could not set pixel size of face!");
        destructFace(face);
        return false;
    }

    faceHeight = (uint32_t) face->size->metrics.height >> 6u;

    if(face->charmap == nullptr)
    {
        error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
        if(error)
        {
            utils::log("could not load a charmap");

            error = FT_Select_Charmap(face, FT_ENCODING_OLD_LATIN_2);
            if(error)
            {
                utils::log("could not load fallback charmap");
                destructFace(face);
                return false;
            }
        }
    }
    assert(face->charmap->encoding == FT_ENCODING_UNICODE || face->charmap->encoding == FT_ENCODING_OLD_LATIN_2);

    return true;
}

Font::operator bool() const
{
    return (faceHeight != 0);
}

Vector2f Font::computeTextDimension(const String& text, int32_t pixelSize)
{
    float maxX = 0.0f;
    float scale = (float)pixelSize / size;
    Vector2f result = { 0.0f, faceHeight * scale };

    for (uint32_t i = 0; i < text.size(); ++i)
    {
        char c = text[i];

        if (c == '\n')
        {
            result.y += faceHeight * scale;
            if (result.x > maxX)
            {
                maxX = result.x;
            }
            result.x = 0;
        }
        else if (c == ' ')
        {
            const GlyphRegion& region = regions[c - ' '];
            result.x += region.advanceX * scale;
        }
        else
        {
            assert(c >= '!' && c <= '~');
            const GlyphRegion& region = regions[c - ' '];

            result.x += region.advanceX * scale;
        }
    }

    if (maxX > result.x)
    {
        result.x = maxX;
    }

    return result;
}

void Font::destructFace(FT_Face& face)
{
    FT_Done_Face(face);
}

bool Font::loadGlyphIntoMap(char c, GlyphRegion& glyphRegion, FT_Face& face, Vector2ui& xy)
{
    uint32_t glyphIndex = FT_Get_Char_Index(face, c);
    int32_t error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
    if(error)
    {
        utils::log("could not load char from face");

        destructFace(face);
        return false;
    }

    error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    if (error)
    {
        utils::log("could not render char from face");

        destructFace(face);
        return false;
    }

    glyphRegion.xy = xy;
    glyphRegion.size = {(uint32_t) face->glyph->bitmap.width,
                        (uint32_t) face->glyph->bitmap.rows};
    glyphRegion.bitmapTopLeft.x = (float)face->glyph->bitmap_left;
    glyphRegion.bitmapTopLeft.y = (float)(face->glyph->bitmap_top - glyphRegion.size.y);
    glyphRegion.advanceX = ((uint32_t) face->glyph->advance.x) >> 6u;

    assert(face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);
    assert(glyphRegion.size.x <= size && glyphRegion.size.y <= size);

    return true;
}

void Font::drawText(const String& text, const Vector2f& pos, Color color)
{
    drawText(text, pos, size, color);
}

Font::Font(const String& filename, int32_t size) : size(size)
{
    library = Globals::window->getFontLibrary();
    gfx = Globals::window->getGfx().getRenderer<Graphics2D>();
    assert(library != nullptr && size != 0 && gfx != nullptr);

    FT_Face face = nullptr;

    Ifstream file(filename);
    assert(file);

    if(!loadFaceFromLibrary(file.getFullData(), file.getSize(), face))
        InvalidCodePath;

    if(!createGlyphRenderTextureAndMap(face))
        InvalidCodePath;
}
