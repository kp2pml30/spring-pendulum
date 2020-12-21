# Simple spring pendulum model

##  How it works
|red|green|blue|
|---|---|---|
|Runge Kutta|Euler|Midpoint|

Note that during editor mod only Runge Kutta is enabled

## depends on:

Rendering:

* glfw
* glew
* opengl 2.0

Pendulum is dependency-free

## Notes
* Amplitude grows
* smaller respone delta means less error, less amlitude growth.

## Conclusion
Runge kutta is close to midpoint in the start. Euler has big error.

## Is euler a bug?
No it is not. I tested exponent (`plot-test.cpp`) with all methods and it is same with wikipedia plots.

