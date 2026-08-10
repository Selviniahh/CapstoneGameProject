$input v_color0, v_texcoord0

// Solid-colour flash. The sprite's own colours are pulled towards one flat colour while its alpha
// is left alone, so what is left on screen is the artwork's silhouette filled in - which is what
// makes a one-frame hit read as a hit and not as a tint. See ETG::ShaderEffect::Flash and the
// component that drives it, ETG::ShaderEffectComponent.
//
// u_effectParams.rgb  the flash colour, 0..1
// u_effectParams.a    how far towards it, 0 = untouched, 1 = the flat colour
//
// Unlike the grayscale program's amount, these arrive per draw (GraphicsDevice::ShaderEffectParams):
// two enemies hit in the same frame can be flashing different colours, and each of them is its own
// draw call for the length of the flash.
//
// Deliberately written against the lowest common denominator (no textureLod, no derivatives,
// no integer maths) so the exact same source compiles for GLSL ES 1.00/3.00 (WebGL, Android,
// iOS via Metal), desktop GLSL, SPIR-V and D3D bytecode.
#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

uniform vec4 u_effectParams;

void main()
{
	vec4 texel = texture2D(s_texColor, v_texcoord0) * v_color0;

	gl_FragColor = vec4(mix(texel.rgb, u_effectParams.rgb, u_effectParams.w), texel.a);
}
