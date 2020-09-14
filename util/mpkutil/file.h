#pragma once

#include <stdbool.h>
#include <stdio.h>

#include <msgpack.h>

bool
mpkutil_file_unpack_with_unpacker(
    msgpack_unpacked* obj,
    FILE*             fp,
    msgpack_unpacker* unpacker
);

bool
mpkutil_file_unpack(
    msgpack_unpacked* obj,
    FILE*             fp
);
