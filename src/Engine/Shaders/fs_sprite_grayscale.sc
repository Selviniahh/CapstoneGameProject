$input v_color0, v_texcoord0

// Grayscale sprite. Same as fs_sprite, but the RGB is pulled towards its luminance before
// being written out. Characters (Hero, every EnemyBase) draw with this program; see
// ETG::ShaderEffect and ETG::ShaderLibrary::SetGrayscaleAmount.
//
// u_effectParams.x  grayscale amount, 0 = untouched, 1 = fully desaturated.
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

	// Rec. 601 luma: the weights the human eye actually applies to R, G and B.
	float luma = dot(texel.rgb, vec3(0.299, 0.587, 0.114));

	gl_FragColor = vec4(mix(texel.rgb, vec3_splat(luma), u_effectParams.x), texel.a);
}
