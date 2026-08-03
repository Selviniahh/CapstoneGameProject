// Shared varying/attribute declaration for every ETG shader.
// The attribute set here matches ETG::GfxVertex (position, packed RGBA8 color, texture UV),
// which is also byte-for-byte compatible with ImGui's ImDrawVert.
vec4 v_color0    : COLOR0    = vec4(1.0, 1.0, 1.0, 1.0);
vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);

vec2 a_position  : POSITION;
vec4 a_color0    : COLOR0;
vec2 a_texcoord0 : TEXCOORD0;
