#pragma once

inline char vertexShader[] = R"delim(
void main(void)
{
	// gl_FrontColor = gl_Color;
	vec3 ldir = normalize(vec3(-1, -1, -1));
	vec3 col_base = gl_Color.xyz;
	vec3 fong = col_base * (0.1 + -dot(ldir, gl_Normal.xyz));
	fong = max(vec3(0), fong);
	fong = min(vec3(1), fong);
	gl_FrontColor = vec4(fong, 1);
	gl_Position = ftransform();
}
)delim";

inline char pixelShader[] = R"delim(
void main(void)
{
	// gl_FragColor = vec4(color, 1);
	// gl_FragColor = vec4(norm, 1);
	gl_FragColor = vec4(gl_Color.xyz, 1);
}
)delim";
