#pragma once

#include <glad/glad.h>
#include <iostream>

class FrameBuffer {
public:
    unsigned int fbo;
    unsigned int texture;
    unsigned int rbo;

    FrameBuffer(int width, int height);

    void Bind();
    void Unbind();
};