
// Execution path controls.
uniform int useBumpMap;
uniform int crawlDiffMap;
uniform int crawlBumpMap;

uniform sampler2D diffMap;
uniform sampler2D bumpMap;
uniform float osg_FrameTime;

varying vec3 pos_worldspace;
varying vec3 n_worldspace;
varying vec3 t_worldspace;
varying vec3 b_worldspace;

void main()
{
    gl_FragData[0] = vec4(pos_worldspace, gl_FragCoord.z);
    vec3 nn = vec3(1.0);
    if (useBumpMap > 0)
    {
        // Convert [0; 1] range to [-1; 1].
        if (crawlBumpMap == 0)
            nn = 2.0 * texture2D(bumpMap, gl_TexCoord[0].xy).xyz - vec3(1.0);
        else
        {
            vec2 uv = vec2(0.4 * sin(osg_FrameTime), 0.4 * cos(osg_FrameTime));
            nn = 2.0 * texture2D(bumpMap, gl_TexCoord[0].xy + uv).xyz - vec3(1.0);
        }
    }
    // Convert Tangent space to World space with TBN matrix.
    gl_FragData[1] = vec4(nn.x * t_worldspace + nn.y * b_worldspace + nn.z * n_worldspace, 1.0);
    if (crawlDiffMap == 0)
        gl_FragData[2] = texture2D(diffMap, gl_TexCoord[0].xy);
    else
    {
        vec2 uv = vec2(0.4 * sin(osg_FrameTime), 0.4 * cos(osg_FrameTime));
        gl_FragData[2] = texture2D(diffMap, gl_TexCoord[0].xy + uv);
    }
}

