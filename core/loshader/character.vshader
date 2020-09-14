layout (location = 0) in float i_character_id;
layout (location = 1) in float i_from_motion_id;
layout (location = 2) in float i_to_motion_id;
layout (location = 3) in float i_motion_time;
layout (location = 4) in float i_marker;
layout (location = 5) in vec2  i_marker_offset;
layout (location = 6) in vec2  i_pos;
layout (location = 7) in vec2  i_size;
layout (location = 8) in vec4  i_color;

out vec4 v_color;

const int vertex_count_ = 54;

const vec2[] marker_ = vec2[](
    vec2( 0.,  1.), vec2(-1., -1.), vec2( 1., -1.)
);
const vec4 marker_color_ = vec4(1., 1., 1., .6);

vec2 player_stand1(void) {
  const vec2[] verts = vec2[](
      vec2( 0.125,  0.986), vec2(-0.125,  0.957), vec2( 0.250,  0.900),
      vec2(-0.125,  0.957), vec2(-0.175,  0.857), vec2( 0.250,  0.900),
      vec2(-0.175,  0.857), vec2(-0.275,  0.571), vec2( 0.075,  0.886),
      vec2(-0.050,  0.843), vec2(-0.275,  0.571), vec2( 0.000,  0.371),
      vec2( 0.150,  0.843), vec2( 0.150,  0.757), vec2( 0.225,  0.843),
      vec2( 0.075,  0.871), vec2( 0.125,  0.757), vec2( 0.125,  0.886),
      vec2( 0.075,  0.871), vec2( 0.025,  0.814), vec2( 0.125,  0.757),
      vec2( 0.000,  0.786), vec2( 0.000,  0.729), vec2( 0.075,  0.771),
      vec2( 0.025,  0.714), vec2( 0.025,  0.386), vec2( 0.200,  0.571),
      vec2( 0.125,  0.457), vec2( 0.025,  0.371), vec2( 0.200,  0.157),
      vec2(-0.025,  0.371), vec2(-0.200,  0.186), vec2( 0.225,  0.086),
      vec2(-0.200,  0.157), vec2( 0.150, -0.371), vec2( 0.225,  0.071),
      vec2( 0.150, -0.371), vec2(-0.075, -0.571), vec2( 0.100, -0.857),
      vec2( 0.100, -0.857), vec2( 0.000, -1.000), vec2( 0.500, -1.000),
      vec2(-0.200,  0.157), vec2( 0.150, -0.371), vec2( 0.225,  0.071),
      vec2( 0.150, -0.371), vec2(-0.075, -0.571), vec2( 0.100, -0.857),
      vec2( 0.100, -0.857), vec2( 0.000, -1.000), vec2( 0.500, -1.000)
  );
  vec2 p = gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
  return p*vec2(4./7., 1.);
}
vec2 player_stand2(void) {
  return player_stand1()*vec2(1., .95) + vec2(0., -.05);
}
vec2 player_walk(void) {
  const vec2[] verts = vec2[](
      vec2( 0.125,  0.986), vec2(-0.125,  0.957), vec2( 0.250,  0.900),
      vec2(-0.125,  0.957), vec2(-0.175,  0.857), vec2( 0.250,  0.900),
      vec2(-0.175,  0.857), vec2(-0.275,  0.571), vec2( 0.075,  0.886),
      vec2(-0.050,  0.843), vec2(-0.275,  0.571), vec2( 0.000,  0.371),
      vec2( 0.150,  0.843), vec2( 0.150,  0.757), vec2( 0.225,  0.843),
      vec2( 0.075,  0.871), vec2( 0.125,  0.757), vec2( 0.125,  0.886),
      vec2( 0.075,  0.871), vec2( 0.025,  0.814), vec2( 0.125,  0.757),
      vec2( 0.000,  0.786), vec2( 0.000,  0.729), vec2( 0.075,  0.771),
      vec2( 0.025,  0.714), vec2( 0.025,  0.386), vec2( 0.200,  0.571),
      vec2( 0.125,  0.457), vec2( 0.025,  0.371), vec2( 0.200,  0.157),
      vec2(-0.025,  0.371), vec2(-0.200,  0.186), vec2( 0.225,  0.086),
      vec2(-0.200,  0.157), vec2( 0.450, -0.371), vec2( 0.225,  0.071),
      vec2( 0.450, -0.371), vec2( 0.325, -0.571), vec2( 0.350, -0.857),
      vec2( 0.350, -0.857), vec2( 0.275, -1.000), vec2( 0.950, -1.000),
      vec2(-0.200,  0.157), vec2(-0.200, -0.371), vec2( 0.225,  0.071),
      vec2(-0.200, -0.371), vec2(-0.450, -0.471), vec2(-0.600, -0.857),
      vec2(-0.625, -0.857), vec2(-0.625, -1.000), vec2(-0.250, -1.000)
  );
  vec2 p = gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
  return p*vec2(4./7., .95) + vec2(0., -.05);
}
vec2 player(in float motion_id) {
  return
      (motion_id == 0.)? player_stand1():
      (motion_id == 1.)? player_stand2():
      (motion_id == 2.)? player_walk():
      vec2(0.);
}

vec2 encephalon(in float motion_id) {
  const vec2[] verts = vec2[](
      vec2(-0.833, -0.783), vec2(-1.000, -1.000), vec2( 1.000, -1.000),
      vec2(-.8333, -0.783), vec2( 1.000, -1.000), vec2( 0.833, -0.783),
      vec2( 0.117, -0.367), vec2( 0.000, -0.750), vec2( 0.300, -0.533),
      vec2( 0.117,  0.050), vec2(-0.083, -0.117), vec2( 0.367, -0.517),
      vec2(-0.050,  0.283), vec2(-0.133, -0.117), vec2( 0.117,  0.083),
      vec2( 0.500,  0.500), vec2(-0.050,  0.333), vec2( 0.150,  0.133),
      vec2( 0.250,  0.917), vec2(-0.233,  0.317), vec2( 0.500,  0.533),
      vec2( 0.217,  0.917), vec2(-0.300,  0.817), vec2(-0.283,  0.333),
      vec2(-0.333,  0.783), vec2(-0.500,  0.567), vec2(-0.333,  0.333)
  );
  return gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
}

vec2 cavia_stand1(void) {
  const vec2[] verts = vec2[](
      vec2( 0.06,  1.00), vec2(-0.16,  0.72), vec2( 0.36,  0.58),
      vec2( 0.00,  0.66), vec2(-0.20,  0.46), vec2(-0.04,  0.20),
      vec2( 0.02,  0.58), vec2( 0.00,  0.20), vec2( 0.14,  0.42),
      vec2(-0.20,  0.40), vec2(-0.20,  0.12), vec2( 0.12, -0.04),
      vec2( 0.06,  0.24), vec2( 0.00,  0.16), vec2( 0.06,  0.08),
      vec2(-0.18,  0.08), vec2( 0.06, -0.40), vec2( 0.06, -0.04),
      vec2( 0.06, -0.44), vec2(-0.06, -0.58), vec2( 0.08, -0.90),
      vec2( 0.08, -0.90), vec2( 0.00, -1.00), vec2( 0.30, -1.00),
      vec2(-0.18,  0.08), vec2( 0.06, -0.40), vec2( 0.06, -0.04),
      vec2( 0.06, -0.44), vec2(-0.06, -0.58), vec2( 0.08, -0.90),
      vec2( 0.08, -0.90), vec2( 0.00, -1.00), vec2( 0.30, -1.00)
  );
  return gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
}
vec2 cavia_walk(void) {
  const vec2[] verts = vec2[](
      vec2( 0.06,  1.00), vec2(-0.16,  0.72), vec2( 0.36,  0.58),
      vec2( 0.00,  0.66), vec2(-0.20,  0.46), vec2(-0.04,  0.20),
      vec2( 0.02,  0.58), vec2( 0.00,  0.20), vec2( 0.14,  0.42),
      vec2(-0.20,  0.40), vec2(-0.20,  0.12), vec2( 0.12, -0.04),
      vec2( 0.06,  0.24), vec2( 0.00,  0.16), vec2( 0.06,  0.08),
      vec2(-0.18,  0.08), vec2(-0.18, -0.40), vec2( 0.06, -0.04),
      vec2(-0.20, -0.42), vec2(-0.36, -0.42), vec2(-0.58, -0.90),
      vec2(-0.58, -0.90), vec2(-0.62, -1.00), vec2(-0.32, -1.00),
      vec2(-0.18,  0.08), vec2( 0.28, -0.40), vec2( 0.06, -0.04),
      vec2( 0.28, -0.40), vec2( 0.20, -0.56), vec2( 0.42, -0.90),
      vec2( 0.42, -0.90), vec2( 0.40, -1.00), vec2( 0.70, -1.00)
  );
  vec2 p = gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
  return p*vec2(1., .95) + vec2(0., -.05);
}
vec2 cavia_attack1(void) {
  return cavia_stand1()*vec2(1., 1.05) + vec2(0., .05);
}
vec2 cavia_attack2(void) {
  const vec2[] verts = vec2[](
      vec2( 0.18,  0.94), vec2(-0.08,  0.70), vec2( 0.34,  0.46),
      vec2( 0.06,  0.64), vec2(-0.20,  0.46), vec2(-0.04,  0.20),
      vec2( 0.08,  0.56), vec2( 0.00,  0.20), vec2( 0.14,  0.42),
      vec2(-0.20,  0.40), vec2(-0.20,  0.12), vec2( 0.12, -0.04),
      vec2( 0.06,  0.24), vec2( 0.00,  0.16), vec2( 0.06,  0.08),
      vec2(-0.18,  0.08), vec2( 0.06, -0.40), vec2( 0.06, -0.04),
      vec2( 0.06, -0.44), vec2(-0.06, -0.58), vec2( 0.08, -0.90),
      vec2( 0.08, -0.90), vec2( 0.00, -1.00), vec2( 0.30, -1.00),
      vec2(-0.18,  0.08), vec2( 0.06, -0.40), vec2( 0.06, -0.04),
      vec2( 0.06, -0.44), vec2(-0.06, -0.58), vec2( 0.08, -0.90),
      vec2( 0.08, -0.90), vec2( 0.00, -1.00), vec2( 0.30, -1.00)
  );
  return gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
}
vec2 cavia_sit(void) {
  const vec2[] verts = vec2[](
      vec2( 0.30,  0.40), vec2( 0.00,  0.18), vec2( 0.58, -0.02),
      vec2( 0.12,  0.10), vec2(-0.22, -0.08), vec2(-0.10, -0.28),
      vec2( 0.10, -0.02), vec2(-0.08, -0.30), vec2( 0.12, -0.20),
      vec2(-0.24, -0.10), vec2(-0.28, -0.38), vec2(-0.04, -0.48),
      vec2( 0.04, -0.26), vec2(-0.10, -0.32), vec2(-0.04, -0.42),
      vec2(-0.26, -0.42), vec2( 0.00, -1.00), vec2(-0.06, -0.50),
      vec2( 0.00, -1.00), vec2(-0.08, -0.86), vec2(-0.50, -0.92),
      vec2(-0.50, -0.92), vec2(-0.54, -0.80), vec2(-0.76, -1.00),
      vec2(-0.26, -0.42), vec2( 0.00, -1.00), vec2(-0.06, -0.50),
      vec2( 0.00, -1.00), vec2(-0.08, -0.86), vec2(-0.50, -0.92),
      vec2(-0.50, -0.92), vec2(-0.54, -0.80), vec2(-0.76, -1.00)
  );
  return gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
}
vec2 cavia_down(void) {
  const vec2[] verts = vec2[](
      vec2( 0.78, -0.50), vec2( 0.46, -0.44), vec2( 0.66, -1.00),
      vec2( 0.50, -0.58), vec2(-0.16, -0.38), vec2(-0.10, -0.54),
      vec2( 0.38, -0.60), vec2(-0.02, -0.56), vec2( 0.24, -0.70),
      vec2(-0.20, -0.40), vec2(-0.28, -0.60), vec2(-0.14, -0.60),
      vec2(-0.02, -0.60), vec2(-0.10, -0.58), vec2(-0.08, -0.60),
      vec2(-0.38, -0.62), vec2( 0.00, -1.00), vec2(-0.16, -0.62),
      vec2( 0.00, -1.00), vec2(-0.18, -0.86), vec2(-0.50, -0.92),
      vec2(-0.50, -0.92), vec2(-0.54, -0.80), vec2(-0.76, -1.00),
      vec2(-0.38, -0.62), vec2( 0.00, -1.00), vec2(-0.16, -0.62),
      vec2( 0.00, -1.00), vec2(-0.18, -0.86), vec2(-0.50, -0.92),
      vec2(-0.50, -0.92), vec2(-0.54, -0.80), vec2(-0.76, -1.00)
  );
  return gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
}
vec2 cavia(in float motion_id) {
  return
      (motion_id == 0.)? cavia_stand1():
      (motion_id == 2.)? cavia_walk():
      (motion_id == 3.)? cavia_attack1():
      (motion_id == 4.)? cavia_attack2():
      (motion_id == 5.)? cavia_sit():
      (motion_id == 6.)? cavia_down():
      vec2(0.);
}

vec2 scientist_stand1(void) {
  const vec2[] verts = vec2[](
    vec2(-0.050,  0.982), vec2(-0.150,  0.764), vec2( 0.400,  0.655),
    vec2(-0.050,  0.727), vec2(-0.475,  0.436), vec2( 0.225,  0.143),
    vec2(-0.475,  0.418), vec2(-0.400, -0.709), vec2( 0.225,  0.127),
    vec2( 0.225,  0.111), vec2(-0.400, -0.725), vec2( 0.225, -0.709),
    vec2( 0.025,  0.673), vec2( 0.200,  0.255), vec2( 0.175,  0.636),
    vec2( 0.000, -0.418), vec2(-0.150, -0.509), vec2(-0.075, -0.909),
    vec2(-0.075, -0.909), vec2(-0.150, -1.000), vec2( 0.200, -1.000),
    vec2( 0.000, -0.418), vec2(-0.150, -0.509), vec2(-0.075, -0.909),
    vec2(-0.075, -0.909), vec2(-0.150, -1.000), vec2( 0.200, -1.000)
  );
  return gl_VertexID < verts.length()?
      verts[gl_VertexID]*vec2(.8, 1.): vec2(0.);
}
vec2 scientist_stand2(void) {
  return scientist_stand1() + (gl_VertexID < 5*3? vec2(0., -.05): vec2(0.));
}
vec2 scientist_attack1(void) {
  const vec2[] verts = vec2[](
    vec2(-0.475,  0.982), vec2(-0.400,  0.727), vec2( 0.225,  0.927),
    vec2(-0.225,  0.764), vec2(-0.675,  0.455), vec2( 0.225,  0.127),
    vec2(-0.700,  0.418), vec2(-0.400, -0.709), vec2( 0.225,  0.127),
    vec2( 0.225,  0.127), vec2(-0.400, -0.709), vec2( 0.350, -0.709),
    vec2(-0.125,  0.727), vec2( 0.150,  0.309), vec2( 0.050,  0.691),
    vec2( 0.000, -0.418), vec2(-0.150, -0.509), vec2(-0.075, -0.909),
    vec2(-0.075, -0.909), vec2(-0.150, -1.000), vec2( 0.200, -1.000),
    vec2( 0.000, -0.418), vec2(-0.150, -0.509), vec2(-0.075, -0.909),
    vec2(-0.075, -0.909), vec2(-0.150, -1.000), vec2( 0.200, -1.000)
  );
  return gl_VertexID < verts.length()?
      verts[gl_VertexID]*vec2(.8, 1.): vec2(0.);
}
vec2 scientist_sit(void) {
  const vec2[] verts = vec2[](
    vec2(-0.050,  0.782), vec2(-0.150,  0.564), vec2( 0.400,  0.455),
    vec2(-0.050,  0.527), vec2(-0.475,  0.236), vec2( 0.225, -0.143),
    vec2(-0.475,  0.218), vec2(-0.400, -0.909), vec2( 0.225, -0.127),
    vec2( 0.225, -0.111), vec2(-0.400, -0.925), vec2( 0.225, -0.909),
    vec2( 0.025,  0.473), vec2( 0.200,  0.055), vec2( 0.175,  0.436),
    vec2( 0.000, -0.418), vec2(-0.150, -0.509), vec2(-0.075, -0.909),
    vec2(-0.075, -0.909), vec2(-0.150, -1.000), vec2( 0.200, -1.000),
    vec2( 0.000, -0.418), vec2(-0.150, -0.509), vec2(-0.075, -0.909),
    vec2(-0.075, -0.909), vec2(-0.150, -1.000), vec2( 0.200, -1.000)
  );
  return gl_VertexID < verts.length()?
      verts[gl_VertexID]*vec2(.8, 1.): vec2(0.);
}
vec2 scientist(in float motion_id) {
  return
      (motion_id == 0.)? scientist_stand1():
      (motion_id == 1.)? scientist_stand2():
      (motion_id == 3.)? scientist_attack1():
      (motion_id == 5.)? scientist_sit():
      vec2(0.);
}

vec2 warder_stand1(void) {
  const vec2[] verts = vec2[](
      vec2(-0.10,  0.96), vec2(-0.24,  0.76), vec2( 0.22,  0.94),
      vec2( 0.08,  0.84), vec2(-0.20,  0.70), vec2( 0.04,  0.70),
      vec2(-0.10,  0.68), vec2(-0.24,  0.48), vec2(-0.08,  0.10),
      vec2(-0.08,  0.60), vec2(-0.04,  0.12), vec2( 0.06,  0.42),
      vec2(-0.22,  0.32), vec2(-0.26,  0.16), vec2(-0.06,  0.04),
      vec2( 0.02,  0.20), vec2(-0.04,  0.04), vec2( 0.04,  0.00),
      vec2(-0.24,  0.12), vec2( 0.00, -0.40), vec2( 0.06, -0.04),
      vec2( 0.00, -0.42), vec2(-0.14, -0.54), vec2( 0.00, -0.90),
      vec2( 0.00, -0.90), vec2(-0.04, -1.00), vec2( 0.24, -1.00),
      vec2(-0.24,  0.12), vec2( 0.00, -0.40), vec2( 0.06, -0.04),
      vec2( 0.00, -0.42), vec2(-0.14, -0.54), vec2( 0.00, -0.90),
      vec2( 0.00, -0.90), vec2(-0.04, -1.00), vec2( 0.24, -1.00),
      vec2(-0.26, -0.04), vec2(-0.24, -0.08), vec2( 0.58,  0.20)
  );
  return gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
}
vec2 warder_walk(void) {
  const vec2[] verts = vec2[](
      vec2(-0.10,  0.96), vec2(-0.24,  0.76), vec2( 0.22,  0.94),
      vec2( 0.08,  0.84), vec2(-0.20,  0.70), vec2( 0.04,  0.70),
      vec2(-0.10,  0.68), vec2(-0.24,  0.48), vec2(-0.08,  0.10),
      vec2(-0.08,  0.60), vec2(-0.04,  0.12), vec2( 0.06,  0.42),
      vec2(-0.22,  0.32), vec2(-0.26,  0.16), vec2(-0.06,  0.04),
      vec2( 0.02,  0.20), vec2(-0.04,  0.04), vec2( 0.04,  0.00),
      vec2(-0.24,  0.12), vec2(-0.34, -0.40), vec2( 0.06, -0.04),
      vec2(-0.34, -0.42), vec2(-0.50, -0.46), vec2(-0.62, -0.90),
      vec2(-0.62, -0.90), vec2(-0.66, -1.00), vec2(-0.40, -1.00),
      vec2(-0.24,  0.12), vec2( 0.30, -0.40), vec2( 0.06, -0.04),
      vec2( 0.30, -0.40), vec2( 0.22, -0.56), vec2( 0.44, -0.90),
      vec2( 0.44, -0.90), vec2( 0.40, -1.00), vec2( 0.66, -1.00),
      vec2(-0.08, -0.08), vec2(-0.06, -0.12), vec2( 0.74,  0.20)
  );
  vec2 p = gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
  return p*vec2(1., .95) + vec2(0., -.05);
}
vec2 warder_attack1(void) {
  const vec2[] verts = vec2[](
      vec2(-0.10,  0.96), vec2(-0.24,  0.76), vec2( 0.22,  0.94),
      vec2( 0.08,  0.84), vec2(-0.20,  0.70), vec2( 0.04,  0.70),
      vec2(-0.10,  0.68), vec2(-0.24,  0.48), vec2(-0.08,  0.10),
      vec2(-0.08,  0.60), vec2(-0.04,  0.12), vec2( 0.06,  0.42),
      vec2(-0.22,  0.32), vec2(-0.26,  0.16), vec2(-0.06,  0.04),
      vec2( 0.02,  0.20), vec2(-0.04,  0.04), vec2( 0.04,  0.00),
      vec2(-0.24,  0.12), vec2(-0.34, -0.40), vec2( 0.06, -0.04),
      vec2(-0.34, -0.42), vec2(-0.50, -0.46), vec2(-0.62, -0.90),
      vec2(-0.62, -0.90), vec2(-0.66, -1.00), vec2(-0.40, -1.00),
      vec2(-0.24,  0.12), vec2( 0.30, -0.40), vec2( 0.06, -0.04),
      vec2( 0.30, -0.40), vec2( 0.22, -0.56), vec2( 0.44, -0.90),
      vec2( 0.44, -0.90), vec2( 0.40, -1.00), vec2( 0.66, -1.00),
      vec2(-0.56,  0.32), vec2(-0.56,  0.28), vec2( 0.30,  0.30)
  );
  vec2 p = gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
  return p*vec2(1., .95) + vec2(0., -.05);
}
vec2 warder_attack2(void) {
  const vec2[] verts = vec2[](
      vec2(-0.10,  0.96), vec2(-0.24,  0.76), vec2( 0.22,  0.94),
      vec2( 0.08,  0.84), vec2(-0.20,  0.70), vec2( 0.04,  0.70),
      vec2(-0.10,  0.68), vec2(-0.24,  0.48), vec2(-0.08,  0.10),
      vec2(-0.08,  0.60), vec2(-0.04,  0.12), vec2( 0.06,  0.42),
      vec2(-0.22,  0.32), vec2(-0.26,  0.16), vec2(-0.06,  0.04),
      vec2( 0.02,  0.20), vec2(-0.04,  0.04), vec2( 0.04,  0.00),
      vec2(-0.24,  0.12), vec2(-0.34, -0.40), vec2( 0.06, -0.04),
      vec2(-0.34, -0.42), vec2(-0.50, -0.46), vec2(-0.62, -0.90),
      vec2(-0.62, -0.90), vec2(-0.66, -1.00), vec2(-0.40, -1.00),
      vec2(-0.24,  0.12), vec2( 0.30, -0.40), vec2( 0.06, -0.04),
      vec2( 0.30, -0.40), vec2( 0.22, -0.56), vec2( 0.44, -0.90),
      vec2( 0.44, -0.90), vec2( 0.40, -1.00), vec2( 0.66, -1.00),
      vec2( 0.12,  0.32), vec2( 0.12,  0.28), vec2( 0.96,  0.30)
  );
  return gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
}
vec2 warder_down(void) {
  const vec2[] verts = vec2[](
      vec2(-0.80, -1.00), vec2(-0.64, -1.00), vec2(-0.86, -0.62),
      vec2(-0.76, -0.72), vec2(-0.58, -1.00), vec2(-0.64, -0.72),
      vec2(-0.58, -0.86), vec2(-0.44, -0.98), vec2( 0.00, -0.82),
      vec2(-0.50, -0.82), vec2( 0.00, -0.80), vec2(-0.36, -0.70),
      vec2(-0.18, -0.92), vec2( 0.04, -1.00), vec2( 0.04, -0.82),
      vec2(-0.12, -0.72), vec2( 0.04, -0.80), vec2( 0.04, -0.52),
      vec2( 0.06, -1.00), vec2( 0.30, -0.46), vec2( 0.06, -0.54),
      vec2( 0.30, -0.46), vec2( 0.30, -0.74), vec2( 0.50, -0.90),
      vec2( 0.50, -0.90), vec2( 0.46, -1.00), vec2( 0.70, -1.00),
      vec2( 0.06, -1.00), vec2( 0.30, -0.46), vec2( 0.06, -0.54),
      vec2( 0.30, -0.46), vec2( 0.30, -0.74), vec2( 0.50, -0.90),
      vec2( 0.50, -0.90), vec2( 0.46, -1.00), vec2( 0.70, -1.00),
      vec2(-0.06, -0.96), vec2(-0.06, -1.00), vec2( 0.80, -1.00)
  );
  return gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
}
vec2 warder(in float motion_id) {
  return
      (motion_id == 0.)? warder_stand1():
      (motion_id == 2.)? warder_walk():
      (motion_id == 3.)? warder_attack1():
      (motion_id == 4.)? warder_attack2():
      (motion_id == 6.)? warder_down():
      vec2(0.);
}

vec2 get_vert(in float motion_id) {
  return
      (i_character_id == 0.)? player(motion_id):
      (i_character_id == 1.)? encephalon(motion_id):
      (i_character_id == 2.)? cavia(motion_id):
      (i_character_id == 3.)? scientist(motion_id):
      (i_character_id == 4.)? warder(motion_id):
      vec2(0.);
}
void main(void) {
  if (gl_VertexID < vertex_count_-3) {
    v_color = i_color;

    vec2 p = mix(
        get_vert(i_from_motion_id),
        get_vert(i_to_motion_id),
        i_motion_time);
    gl_Position = uni.proj * uni.cam * vec4(p*i_size+i_pos, 0., 1.);

  } else {
    v_color    = marker_color_;
    v_color.a *= i_marker;

    vec2 p = marker_[gl_VertexID - (vertex_count_-3)];
    p     *= uni.dpi*.03 / uni.resolution * 2.;

    gl_Position     = uni.proj * uni.cam * vec4(i_pos+i_marker_offset, 0., 1.);
    gl_Position.xy += p;
  }
}
