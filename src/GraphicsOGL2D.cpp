#include "GraphicsOGL2D.h"
#include "GLUtils.h"
//#include "OGLTexture.h"

GraphicsOGL2D::GraphicsOGL2(uint32_t renderWidth, uint32_t renderHeight, View::ViewportType viewportType)
        : GraphicsOGLIniter(renderWidth, renderHeight, viewportType, DISPLAY_ATTRIBS),
          orhtoProj(), vertices(makeUnique<Vertex[]>(NUM_VERTICES_TO_BATCH))
{
    Vector2f spriteVertices[] = { {0.0f, 0.0f},
                                  {1.0f, 0.0f},
                                  {1.0f, 1.0f},
                                  {0.0f, 1.0f} };
    for(uint32_t i = 0; i < NUM_VERTICES_TO_BATCH; i += 4)
    {
        vertices[i + 0].position = spriteVertices[0];
        vertices[i + 1].position = spriteVertices[1];
        vertices[i + 2].position = spriteVertices[2];
        vertices[i + 3].position = spriteVertices[3];
    }
}

void GraphicsOGL2D::draw(const Sprite& sprite)
{
    assert(!isFastRectDrawing);
    assert(vb);

    const OGLTexture* texture = (const OGLTexture*)sprite.getTexture();
    assert(*texture);

    if(currentBoundTexture != texture->getTextureId())
    {
        if(nSpritesBatched != 0)
            flush();

        texture->bind();
        currentBoundTexture = texture->getTextureId();
    }
    else if(nSpritesBatched >= NUM_SPRITES_TO_BATCH)
    {
        flush();
    }

    float texRectLeft = ((float)sprite.getTextureRect().left) / texture->getWidth();
    float texRectTop = ((float)sprite.getTextureRect().bottom) / texture->getHeight();
    float texRectRight = ((float)sprite.getTextureRect().getRight()) / texture->getWidth();
    float texRectBottom = ((float) sprite.getTextureRect().getTop()) / texture->getHeight();
    Vector2f texCoord[4] = { { texRectLeft, texRectTop },
                             { texRectRight, texRectTop },
                             { texRectRight, texRectBottom },
                             { texRectLeft, texRectBottom } };

    float red = sprite.color.r;
    float green = sprite.color.g;
    float blue = sprite.color.b;
    float alpha = sprite.color.a;

    Mat4x4 mv = sprite.getTransform();

    uint32_t plusI = nVerticesBatched();

    for(uint32_t i = 0; i < 4; ++i)
    {
        //TODO: Think about: pass the pos, scl, angle in directly and compute the mv in the vertexShader

        //TODO: Can add the active texture unit this sprite belongs to and batch sprites with different
        // textures!
        vertices[i + plusI].tex = texCoord[i];
        vertices[i + plusI].colorR = red;
        vertices[i + plusI].colorG = green;
        vertices[i + plusI].colorB = blue;
        vertices[i + plusI].colorA = alpha;
        vertices[i + plusI].mvMatrix[0] = mv.matrix[0];
        vertices[i + plusI].mvMatrix[1] = mv.matrix[1];
        vertices[i + plusI].mvMatrix[2] = mv.matrix[4];
        vertices[i + plusI].mvMatrix[3] = mv.matrix[5];
        vertices[i + plusI].mvMatrix[4] = mv.matrix[12];
        vertices[i + plusI].mvMatrix[5] = mv.matrix[13];
    }

    ++nSpritesBatched;
}

void GraphicsOGL2D::draw(const RectangleShape& rect)
{
    assert(isFastRectDrawing);
    assert(vb);

    if(nSpritesBatched >= NUM_SPRITES_TO_BATCH)
    {
        flush();
    }

    float red = rect.fillColor.r;
    float green = rect.fillColor.g;
    float blue = rect.fillColor.b;
    float alpha = rect.fillColor.a;

    Mat4x4 mv = rect.getTransform();

    uint32_t plusI = nVerticesBatched();

    for(uint32_t i = 0; i < 4; ++i)
    {
        //vertices[i + plusI].tex = { 0.0f, 0.0f };
        vertices[i + plusI].colorR = red;
        vertices[i + plusI].colorG = green;
        vertices[i + plusI].colorB = blue;
        vertices[i + plusI].colorA = alpha;
        vertices[i + plusI].mvMatrix[0] = mv.matrix[0];
        vertices[i + plusI].mvMatrix[1] = mv.matrix[1];
        vertices[i + plusI].mvMatrix[2] = mv.matrix[4];
        vertices[i + plusI].mvMatrix[3] = mv.matrix[5];
        vertices[i + plusI].mvMatrix[4] = mv.matrix[12];
        vertices[i + plusI].mvMatrix[5] = mv.matrix[13];
    }

    ++nSpritesBatched;
}

void GraphicsOGL2D::render()
{
    flush();

    if (view.updated())
    {
        orhtoProj = view.getOrthoProj();
        shaderSprite.setUniformMat4f("u_proj", orhtoProj);
    }
}

void GraphicsOGL2D::setupGfxGpu()
{
    orhtoProj = view.getOrthoProj();

    CallGL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &nTextureUnits));

    new (&shaderSprite) Shader("SSprite", Vector<ShortString>{ "position", "texCoord", "color",
                                                               "mvMatrixSclRot", "mvMatrixPos" });
    new (&shaderRectShape) Shader("SRectShape", Vector<ShortString>{ "position", "texCoord", "color",
                                                                     "mvMatrixSclRot", "mvMatrixPos" });

    uint32_t indices[6 * NUM_SPRITES_TO_BATCH];
    for(uint32_t i = 0, counter = 0; i < NUM_SPRITES_TO_BATCH; ++i, counter += 6)
    {
        indices[counter + 0] = 0 + i * 4;
        indices[counter + 1] = 2 + i * 4;
        indices[counter + 2] = 3 + i * 4;
        indices[counter + 3] = 0 + i * 4;
        indices[counter + 4] = 1 + i * 4;
        indices[counter + 5] = 2 + i * 4;
    }
    new (&ib) IndexBuffer(indices, arrayCount(indices));
    ib.bind();

    new (&vb) VertexBuffer(nullptr, sizeof(Vertex) * NUM_VERTICES_TO_BATCH, GL_DYNAMIC_DRAW);
    vb.bind();

    setupSpriteRendering();

    VertexLayouts va;
    va.addAttribute(2, GL_FLOAT);
    va.addAttribute(2, GL_FLOAT);
    va.addAttribute(4, GL_FLOAT);
    va.addAttribute(4, GL_FLOAT);
    va.addAttribute(2, GL_FLOAT);
    va.set();
}

void GraphicsOGL2D::freeGfxGpu()
{
    shaderSprite.~Shader();
    shaderRectShape.~Shader();

    ib.~IndexBuffer();
    vb.~VertexBuffer();
}

void GraphicsOGL2D::bindOtherOrthoProj(const Mat4x4& otherOrthoProj)
{
    assert(shaderSprite);
    shaderSprite.setUniformMat4f("u_proj", otherOrthoProj);
}

void GraphicsOGL2D::unbindOtherOrthoProj()
{
    assert(shaderSprite);
    shaderSprite.setUniformMat4f("u_proj", orhtoProj);
}

void GraphicsOGL2D::setupSpriteRendering()
{
    assert(shaderSprite);

    shaderSprite.bind();
    shaderSprite.setUniformMat4f("u_proj", orhtoProj);
}

void GraphicsOGL2D::flush()
{
    assert(ib);
    assert(shaderSprite);

    vb.subData(0, sizeof(Vertex) * nVerticesBatched(), vertices.get());

    CallGL(glDrawElements(GL_TRIANGLES, 6 * nSpritesBatched, GL_UNSIGNED_INT, 0));
    nSpritesBatched = 0;
}

uint32_t GraphicsOGL2::nVerticesBatched() const
{
    return (nSpritesBatched * 4);
}

//TODO: Implement
void GraphicsOGL2D::draw(const CircleShape& circle)
{
}

void GraphicsOGL2D::startFastRectDrawing()
{
    flush();

    shaderRectShape.bind();
    shaderRectShape.setUniformMat4f("u_proj", orhtoProj);

    isFastRectDrawing = true;
}

void GraphicsOGL2D::stopFastRectDrawing()
{
    flush();

    isFastRectDrawing = false;
    setupSpriteRendering();
}
