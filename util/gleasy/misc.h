#pragma once

#include <GL/glew.h>

#define GLEASY_GET_BYTE_SIZE_OF_TYPE(t) (  \
    (t) == GL_UNSIGNED_BYTE?  1:  \
    (t) == GL_BYTE?           1:  \
    (t) == GL_SHORT?          2:  \
    (t) == GL_UNSIGNED_SHORT? 2:  \
    (t) == GL_INT?            4:  \
    (t) == GL_UNSIGNED_INT?   4:  \
    (t) == GL_FLOAT?          4: 0)

#define GLEASY_GET_CHANNELS_OF_TEXTURE_FORMAT(f) (  \
    (f) == GL_RGBA? 4:  \
    (f) == GL_RGB?  3:  \
    (f) == GL_RG?   2:  \
    (f) == GL_RED?  1: 0)
