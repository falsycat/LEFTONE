#version 330
#extension GL_ARB_explicit_uniform_location : enable

#define PI radians(180.)

precision mediump float;

layout(std140) uniform uniblock {
  vec2  resolution;
  vec2  dpi;
  float aa;

  mat4  proj;
  mat4  cam;
  vec4  pos;  /* chunk x, chunk y, fract x, fract y*/

  float time;  /* %60sec */
} uni;

/* To make it explicit which errors happened in header or body,
   adds 10000 to body's line number. */
#line 100001
