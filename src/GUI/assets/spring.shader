// #extension GL_OES_standard_derivatives:enable

#ifdef GL_ES
precision mediump float;
#endif

#define PI 3.14159

#ifndef NSPRINGS
#define NSPRINGS 4
#endif

#ifndef RMS_BUFFER_SIZE
#define RMS_BUFFER_SIZE 64
#endif

#ifndef BORDER_COLOR
#define BORDER_COLOR vec3(0., 0., 1.)
#endif

uniform vec2 u_resolution;
uniform float u_length;
uniform float u_density;
uniform float u_rms[RMS_BUFFER_SIZE * NSPRINGS];
uniform int u_rmspos;

float roundedBox(vec2 p, vec2 size, float cornerSize)
{
    vec2 q    = abs(p) - size + cornerSize;
    float sdf = min(max(q.x, q.y), 0.0) + length(max(q, 0.)) - cornerSize;
    return sdf;
}

void main()
{
    vec2 st    = (2. * gl_FragCoord.xy - u_resolution) / u_resolution.xy;
    vec2 stBck = st;
    vec2 stBorder =
        st; //(2.* gl_FragCoord.xy - u_resolution) / u_resolution.xx;

    float springWidth  = 0.598;
    float springFreq   = 0.6 / sqrt(u_resolution.y);
    float springHeight = 0.89;

    // divide image in NSPRINGS parts vertycally
    st.y = (st.y + 1.) / 2.;
    st.y *= float(NSPRINGS);
    int springId = int(st.y);
    st.y         = fract(st.y) * 2. - 1.;

#ifdef DUMMY
    // dummy rms
    float rms =
        sin(u_time * 4. * (sin(float(springId + 1) * 10000.39)) +
            st.x * (4. + (1. + sin(float(springId + 1) * 4923043.)) * 1.3));
    rms = rms * rms;
#else
    // rms from buffer
    float xpos  = u_length * float(RMS_BUFFER_SIZE) * (st.x + 1.0) / 2.0;
    int ixpos   = int(xpos);
    float fxpos = xpos - float(ixpos);
    int ixpos0  = (u_rmspos - ixpos) & (RMS_BUFFER_SIZE - 1);
    int ixpos1  = (u_rmspos - (ixpos + 1)) & (RMS_BUFFER_SIZE - 1);
    float rms0  = u_rms[ixpos0 * N + springId];
    float rms1  = u_rms[ixpos1 * N + springId];
    float rms   = rms0 + fxpos * (rms1 - rms0);
#endif

    // scale & clamp
    rms = pow(rms, 1. / 3.3);

    // window to multuply displacement
    float winoverflow = 0.5;
    float winpower    = .8;
    float winmult     = 0.25;
    float win         = pow(cos(st.x * winoverflow * PI / 2.), winpower);
    win *= winmult;

    // displace x axis;
    float springshift = 0.888;
    float xshift      = -rms * win;
    st.x += xshift;
    st.x += float(springId) * springshift;

    float th    = 2. * PI * st.x * u_resolution.x * springFreq;
    float costh = cos(th);
    float sinth = sin(th);
    st.y += sinth * springHeight;

    float d = abs(st.y);
    // d -= mix(0.*w, w, pow(abs(costh),2.2));

    // springs
    float width = mix(.0, springWidth, pow(abs(costh), 3.2));
    width /= (1. + abs(dFdx(xshift)) * 50.0);
    float dAA = fwidth(d);

    float springs      = smoothstep(width + dAA, width - dAA, d);
    float alphaSprings = springs;

    // shadow of springs
    float springsDark   = 0.1;
    float springsBright = 0.9;
    // c *= smoothstep(-0.1,0.1,cth)*.5+.5;
    springs *= mix(springsDark, springsBright, costh * .5 + .5);
    // c *= mix(1.,0.28,pow(d,1.8));
    // c *= mix(0.1,1., pow(.5*cos(th)+.5,1.8));

    // springs color
    vec3 springsColor = vec3(springs);

    // background
    vec3 backgroundDark   = vec3(0.124, 0.124, 0.150);
    vec3 backgroundBright = vec3(0.379, 0.443, 0.475);
    stBck.x               = pow(stBck.x, 1.3);
    stBck.x               = pow(stBck.x, 1.3);
    float background      = 1. - pow(length(stBck), 1.2);
    vec3 backgroundColor  = mix(backgroundDark, backgroundBright, background);

    // border
    vec3 borderColor = BORDER_COLOR;
    stBorder.x       = pow(stBorder.x, u_resolution.x / u_resolution.y);
    float border     = roundedBox(stBorder, vec2(1.), 0.3);
    border           = smoothstep(0.0, 0.01, border);

    // final color
    vec3 color = mix(backgroundColor, springsColor, alphaSprings);
    color      = mix(color, borderColor, border);

    gl_FragColor = vec4(color, 1.0);
}
