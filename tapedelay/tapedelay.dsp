/* dummy */

delay = hslider("delay[0]", 0.5, 0.01, 1., 0.001);
feedback = hslider("feedback[1]", 0.7, 0, 1, 0.01);
pingpong = checkbox("pingpong[2]");

process = delay*feedback*pingpong;
