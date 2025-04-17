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
#define BACKGROUND_COLOR vec3(0.0)
#endif

#define NITER 40
#define AA    3

#ifdef DUMMY
uniform float u_time;
const float u_coils  = 0.552;
const float u_radius = 0.524;
const float u_shape  = 1.5;
#else
uniform float u_rms[RMS_BUFFER_SIZE * NSPRINGS];
uniform int u_rmspos;
uniform float u_coils;
uniform float u_radius;
uniform float u_shape;
#endif
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

    // window to multuply displacement
    float winoverflow = 0.8;
    float winpower    = .8;
    float win         = pow(cos(x * winoverflow * PI / 2.0), winpower);

    return 5.0 * rms * win;
}
#endif

float map(vec3 p, float x)
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

    float cylinder = length(p.yz) - springRadius;
    float coils =
        (sin(u_shape * atan(p.y, p.z) - p.x * springCoils + springMove)) /
        springCoils;
    float dist = length(vec2(cylinder, coils)) - coilRadius;
    ;
    return dist;
}

void main()
{

    vec3 color = vec3(0.0);

    // perform multiple pass per pixel for antialiasing
    for (int aax = 0; aax < AA; ++aax) {
        for (int aay = 0; aay < AA; ++aay) {
            // subpixel
            vec2 aa = vec2(float(aax), float(aay)) / float(AA);
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
                float delta = map(p, xpos);
                if (delta < 0.0015) {
                    dist -= 4.0;
                    color += vec3(0.82 * pow(1.0 / dist, 2.0)) -
                             0.8 * float(i) / float(NITER);
                    break;
                }
                dist += delta;
                if (dist > 6.0) {
                    color += BACKGROUND_COLOR;
                    break;
                }
            }
        }
    }
    color /= float(AA) * float(AA);

    vec2 st = (2.0 * (gl_FragCoord.xy) + -u_resolution) / u_resolution.xy;
    // vignette
    color *= 1.0 - 0.5 * pow(length(st) * 0.9, 4.);

    // border
    vec3 borderColor = BORDER_COLOR;
    st.x             = pow(st.x, u_resolution.x / u_resolution.y);
    float border     = roundedBox(st, vec2(1.0), 0.3);
    border           = smoothstep(0.0, 0.03, border);
    color            = mix(color, borderColor, border);

    gl_FragColor = vec4(color, 1.0);
}
