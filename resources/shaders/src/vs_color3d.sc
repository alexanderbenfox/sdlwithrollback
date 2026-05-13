$input a_position, a_normal, a_color0
$output v_color0

#include <bgfx_shader.sh>

void main()
{
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    // Simple directional lighting from above
    vec3 lightDir = normalize(vec3(0.0, 1.0, 0.5));
    vec3 normal = normalize(mul(u_model[0], vec4(a_normal, 0.0)).xyz);
    float ndotl = max(dot(normal, lightDir), 0.0);
    float lighting = 0.4 + 0.6 * ndotl;
    v_color0 = vec4(a_color0.rgb * lighting, a_color0.a);
}
