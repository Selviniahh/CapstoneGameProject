$input v_color0, v_texcoord0

// Plain textured sprite: the texel tinted by the per-vertex colour. Untextured draws
// (shapes, lines, bounds) go through the same shader against a 1x1 white texture.
#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

void main()
{
	gl_FragColor = texture2D(s_texColor, v_texcoord0) * v_color0;
}
