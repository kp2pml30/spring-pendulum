#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <numbers>

#include "Solvers.h"

using vec2 = mth::vec2<double>;

namespace
{
	static constexpr double TO = 4;
	static constexpr double STEP = 1;
	void error_callback(int error, char const* description)
	{
		std::cerr << "Error : " << description << std::endl;
	}

	vec2 Map(vec2 a)
	{
		/*
		 * 0, 1   -> -1, -1
		 * 4, e(4) -> 1, 1
		 */
		a.Y = (a.Y - 1) / (std::exp(TO) - 1) * 2 - 1;
		a.X = a.X / TO * 2 - 1;
		return a;
	}

	template<typename F>
	void Draw(F const& f, double d, double U)
	{
		{
			auto f0 = f(0);
			auto v2 = Map(f(0));
			glVertex2f(v2.X, v2.Y);
		}
		for (double i = 0; i <= U; i += d)
		{
			auto v2 = Map(f(d));
			glVertex2f(v2.X, v2.Y);
		}
	}
	double EDelta(double v, double h)
	{
		return v;
	}
}

int main()
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return 1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(640, 480, "spring pendulum", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (auto err = glewInit(); err != GLEW_OK)
	{
		std::cerr << "glew init error " << glewGetErrorString(err) << std::endl;
		return 1;
	}
	glClearColor(0, 0, 0, 0);
	glEnable(GL_LINE_WIDTH);

	while (!glfwWindowShouldClose(window))
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		glLineWidth(5);
		glBegin(GL_LINE_STRIP);
		{
			glColor3f(1, 1, 1);
			double cur = 0;
			Draw([&](double d)
					{
						auto r = vec2(cur, std::exp(cur));
						cur += d;
						return r;
					},
					STEP,
					TO);
		}
		glEnd();
		glLineWidth(3);
		glBegin(GL_LINE_STRIP);
		{
			glColor3f(0, 1, 0);
			double cur = 0;
			double prev = 1;
			Draw([&](double d)
					{
						prev += Euler(EDelta, prev, d);
						cur += d;
						auto r = vec2(cur, prev);
						return r;
					},
					STEP,
					TO);
		}
		glEnd();
		glBegin(GL_LINE_STRIP);
		{
			glColor3f(1, 0, 0);
			double cur = 0;
			double prev = 1;
			Draw([&](double d)
					{
						prev += RungeKutta(EDelta, prev, d);
						cur += d;
						auto r = vec2(cur, prev);
						return r;
					},
					STEP,
					TO);
		}
		glEnd();
		glBegin(GL_LINE_STRIP);
		{
			glColor3f(0.2, 0.2, 1);
			double cur = 0;
			double prev = 1;
			Draw([&](double d)
					{
						prev += Midpoint(EDelta, prev, d);
						cur += d;
						auto r = vec2(cur, prev);
						return r;
					},
					STEP,
					TO);
		}
		glEnd();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}
