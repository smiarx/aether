#ifdef GL_ES
precision mediump float;
#endif

#define PI 3.14159

#ifndef NSPRINGS
#ifdef N
#define NSPRINGS N
#else
#define NSPRINGS 4
#endif
#endif

#ifndef SHOWSPRINGS
#define SHOWSPRINGS 3
#endif

#ifndef RMS_BUFFER_SIZE
#define RMS_BUFFER_SIZE 64
#endif

#ifndef BORDER_COLOR
#define BORDER_COLOR vec3(0.0, 0.0, 1.0)
#endif

#ifndef BACKGROUND_COLOR
#define BACKGROUND_COLOR \
    vec3(0.8235294117647058, 0.8392156862745098, 0.8470588235294118)
#endif

#define NITER 80

#ifdef DUMMY
const float u_coils     = 0.552;
const float u_radius    = 0.524;
const float u_shape     = 0.5;
const int u_aasubpixels = 1;
#else
uniform float u_rms[RMS_BUFFER_SIZE * NSPRINGS];
uniform int u_rmspos;
uniform float u_coils;
uniform float u_radius;
uniform float u_shape;
uniform int u_aasubpixels;
#endif
uniform float u_time;
uniform vec2 u_resolution;

// constants
const float springSize   = 0.3;
const float springRadius = 0.38 * springSize;

const float springCoilsMin = 35.0, springCoilsMax = 60.0;
float springCoils =
    springCoilsMin + u_coils * (springCoilsMax - springCoilsMin);
const float coilRadiusMin = 0.007, coilRadiusMax = 0.014;
float coilRadius = coilRadiusMin + u_radius * (coilRadiusMax - coilRadiusMin);

float roundedBox(vec2 p, vec2 size, float cornerSize)
{
    vec2 q    = abs(p) - size + cornerSize;
    float sdf = min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - cornerSize;
    return sdf;
}

#ifndef DUMMY
float getRMS(float x, int springId)
{
    float lgth = 0.6; // rms buffer length used

    // rms from buffer
    float xpos  = lgth * float(RMS_BUFFER_SIZE) * (x + 1.0) / 2.0;
    int ixpos   = int(xpos);
    float fxpos = xpos - float(ixpos);
    int ixpos0  = (u_rmspos - ixpos) & (RMS_BUFFER_SIZE - 1);
    int ixpos1  = (u_rmspos - (ixpos + 1)) & (RMS_BUFFER_SIZE - 1);
    float rms0  = u_rms[ixpos0 * N + springId];
    float rms1  = u_rms[ixpos1 * N + springId];
    float rms   = rms0 + fxpos * (rms1 - rms0);

    // scale & clamp
    rms = pow(rms, 1.0 / 2.5);

    return 5.0 * rms;
}
#endif

vec3 transformSpace(vec3 p, float x)
{
    // divide space into 3 springs
    p.y /= springSize;
    p.y += 0.5;
    int id;
    if (p.y < 2.0 && p.y > -1.0) {
        id  = int(p.y + 1.0);
        p.y = (fract((p.y)) - 0.5) * springSize;
    }

// spring movement function
#ifdef DUMMY
    float springMove = sin(x + u_time * 4.0 + float(id) * 12.3);
#else
    float springMove = getRMS(x, id);
#endif

    // window to multuply displacement
    float winoverflow = 0.85;
    float winpower    = .8;
    float win         = pow(cos(x * winoverflow * PI / 2.0), winpower);
    springMove *= win;

    float fid = float(id);

    p.x = p.x * springCoils - springMove + (fid - 0.394) * 24.1498;

    float transverse = springMove * 0.006;
    float rot  = 2 * PI * u_time * (8.123210 + 2.323 * fid) + fid * 124.32;
    float crot = cos(rot), srot = sin(rot);
    p.y += crot * transverse;
    p.z += srot * transverse;

    return p;
}

float map(vec3 p, float x)
{
    p = transformSpace(p, x);

    float cylinder = length(p.yz) - springRadius;
    float coils    = (sin(u_shape * atan(p.y, p.z) - p.x)) / springCoils;
    float dist     = length(vec2(cylinder, coils)) - coilRadius;
    ;
    return dist;
}

vec3 getColor(vec3 p, float x)
{
    p = transformSpace(p, x);
    p.yz /= springRadius;

    vec3 color;

    float theta = 0.29 * PI;
    float cth = cos(theta), sth = sin(theta);
    p.yz *= mat2(cth, sth, -sth, cth);

    const vec3 baseColor = vec3(0.851, 0.92, 1.000);
    const vec3 specColor = vec3(0.648, 0.706, 0.760);

    color = baseColor * (0.25 + 0.10 * (1.0 + (length(p.yz) - 1.0) *
                                                  springRadius / coilRadius));

    p.x -= 0.66 + u_radius * 0.5;
    p.z = max(-p.z, 0.) - sin(2.0 * (p.x + u_shape * atan(p.y, p.z))) * 0.10;

    p.z = abs(p.z);
    color += baseColor * pow(p.z * 0.25, 1.0);
    color += specColor * (pow(p.z * 0.845, 60.0));

    return color;
}

void main()
{

    vec3 color = vec3(0.0);

    // perform multiple pass per pixel for antialiasing
    for (int aax = 0; aax < u_aasubpixels; ++aax) {
        for (int aay = 0; aay < u_aasubpixels; ++aay) {
            // subpixel
            vec2 aa = vec2(float(aax), float(aay)) / float(u_aasubpixels);
            // cordinnates
            vec2 st = (2.0 * (gl_FragCoord.xy + aa) + -u_resolution);
            // value in the x axis
            float xpos = st.x / u_resolution.x;
            st /= u_resolution.yy;

            // raymarching
            vec3 ro = vec3(0, 0, -5);
            vec3 rd = normalize(vec3(st, 10));

            float dist = 0.0;
            for (int i = 0; i < NITER; ++i) {
                vec3 p      = ro + dist * rd;
                float delta = 0.95 * map(p, xpos);
                dist += delta;
                if (delta < 0.001) {
                    p = ro + dist * rd;
                    color += getColor(p, xpos);
                    break;
                }
                if (dist > 6.0) {
                    vec3 backgroundColor = BACKGROUND_COLOR;
                    float shade     = 0.02 + u_radius * 0.10 + u_coils * 0.14;
                    float shadeSize = 1.70;
                    float shadeY    = (st.y) * shadeSize;
                    shadeY += 0.86;
                    if (shadeY < 2.0 && shadeY > -1.0) {
                        shadeY = (fract(shadeY) - 0.5) / shadeSize;
                        shadeY = abs(shadeY);
                    }
                    backgroundColor *=
                        (1.0 - shade) +
                        shade * min(1.0, pow(shadeY * 5.9,
                                             1.5 + 0.5 * (u_coils + u_radius)));
                    color += backgroundColor;
                    break;
                }
            }
        }
    }
    color /= float(u_aasubpixels) * float(u_aasubpixels);

    float cornerSize = 0.3;

    vec2 st = (2.0 * (gl_FragCoord.xy) + -u_resolution) / u_resolution.xy;
    // vignette
    color *= pow(1.0 - 0.6 * max(0.0, roundedBox(st + vec2(0.035), vec2(0.85),
                                                 cornerSize)),
                 3.0);

    // border
    vec3 backgroundColor = BACKGROUND_COLOR;
    float xyratio        = u_resolution.x / u_resolution.y;
    st.x *= xyratio;
    float box = roundedBox(st, vec2(xyratio, 1.0), cornerSize);

    float border      = smoothstep(-0.03, -0.01, box);
    float borderShade = 0.6;
    vec3 borderColor =
        backgroundColor *
        ((1.0 - borderShade) + borderShade * (2.0 - st.x - st.y) * 0.5);
    color = mix(color, borderColor, border);

    float background = smoothstep(0.0, 0.02, box);
    color            = mix(color, backgroundColor, background);

    gl_FragColor = vec4(color, 1.0);
}
