#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include <memory>

#include "pendulum.h"

#ifndef __FAST_MATH__
# warning fast math is not enabled
#endif

namespace
{
	struct WindowData
	{
		double dt = 0;
		double posz = 0;
		double zang = 30 * mth::PI / 180;
		double camrad = 5;
		Pendulum* pend = nullptr;

		int selected = 0;
		bool edit = false;

		vec editorPos = {};
		vec GetEditorPos() noexcept
		{
			auto r = editorPos;
			editorPos = {0, 0, 0};
			return r;
		}
	};

	WindowData& GetWData(GLFWwindow* ptr)
	{
		return *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(ptr));
	}

	void error_callback(int error, char const* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}

	void DrawVertAt(vec const& v) noexcept
	{
		glVertex3f(v.X, v.Y, v.Z);
	}

	void DrawCS() noexcept
	{
		glDisable(GL_DEPTH_TEST);
		glBegin(GL_LINES);
			glLineWidth(1);
			glColor3f(1, 0, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(1, 0, 0);

			glColor3f(0, 1, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 1, 0);

			glColor3f(0, 0, 1);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 0, 1);

#if 0
			glColor3f(1, 1, 1);
			glVertex3f(0, 0, 0);
			DrawVertAt(G);
#endif
		glEnd();
		glEnable(GL_DEPTH_TEST);
	}

	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept
	{
		auto& data = GetWData(window);
		// pause
		if (key == GLFW_KEY_P && action == GLFW_PRESS)
			data.pend->frozen ^= 1;
		// editor toggle
		else if (key == GLFW_KEY_T && action == GLFW_PRESS)
			data.selected = (data.selected + 1) % 3;
		else if (key == GLFW_KEY_F && action == GLFW_PRESS)
			data.edit ^= 1;
		// cam
		else if (key == GLFW_KEY_A && action != GLFW_RELEASE)
			data.zang -= data.dt * 4;
		else if (key == GLFW_KEY_D && action != GLFW_RELEASE)
			data.zang += data.dt * 4;
		else if (key == GLFW_KEY_W && action != GLFW_RELEASE)
			data.posz += 6 * data.dt;
		else if (key == GLFW_KEY_S && action != GLFW_RELEASE)
			data.posz -= 6 * data.dt;
		else if (key == GLFW_KEY_Q && action != GLFW_RELEASE)
			data.camrad += 6 * data.dt;
		else if (key == GLFW_KEY_E && action != GLFW_RELEASE && data.camrad > 0.1)
			data.camrad -= 6 * data.dt;
		// editor move
		else if (key == GLFW_KEY_LEFT && action != GLFW_RELEASE)
			data.editorPos.X -= 6 * data.dt;
		else if (key == GLFW_KEY_RIGHT && action != GLFW_RELEASE)
			data.editorPos.X += 6 * data.dt;
		else if (key == GLFW_KEY_UP && action != GLFW_RELEASE)
			data.editorPos.Y += 6 * data.dt;
		else if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE)
			data.editorPos.Y -= 6 * data.dt;
		else if (key == GLFW_KEY_PAGE_UP && action != GLFW_RELEASE)
			data.editorPos.Z += 6 * data.dt;
		else if (key == GLFW_KEY_PAGE_DOWN && action != GLFW_RELEASE)
			data.editorPos.Z -= 6 * data.dt;
	}

	void ShowHelp() noexcept
	{
		std::cout
			<< "WASDQE -- controls"
			<< "T -- select ball"
			<< "F -- toggle edit mode"
			<< "P -- pause"
			<< std::endl
			;
	}
}

int main()
{
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return 1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return 1;
	}
	Pendulum pend;
	pend.AddBall({5, 3, 30});
	pend.AddBall({2, 4, 10});

	WindowData wnd;
	wnd.pend = &pend;
	glfwSetWindowUserPointer(window, reinterpret_cast<void*>(&wnd));
	glfwSetKeyCallback(window, key_callback);

	GLUquadricObj* quadObj = gluNewQuadric();
	std::unique_ptr<GLUquadricObj, void (*)(GLUquadricObj*)> flusher(quadObj, gluDeleteQuadric);
	gluQuadricDrawStyle(quadObj, GLU_FILL);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glClearColor(0.3, 0.5, 0.7, 0);
	glEnable(GL_DEPTH_TEST);

	auto prev = std::chrono::system_clock::now();
	while (!glfwWindowShouldClose(window))
	{
		{
			auto now = std::chrono::system_clock::now();
			wnd.dt = std::chrono::duration<double>(now - prev).count();
			prev = std::move(now);
		}

		vec*posprev = nullptr;
		vec* savedV = nullptr;
		vec posprevsaved;
		if (wnd.edit)
			if (wnd.selected == 1)
				posprev = &pend.x1, posprevsaved = pend.x1, savedV = &pend.v1;
			else if (wnd.selected == 2)
				posprev = &pend.x2, posprevsaved = pend.x2, savedV = &pend.v2;
		pend.Update();
		if (posprev != nullptr)
		{
			*posprev = posprevsaved + wnd.GetEditorPos();
			*savedV = vec(0);
		}

		double ratio;
		int width, height;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = height / (float)width;

		// glFrustum(float left, float right, float bottom, float top, float near, float far);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLoadIdentity();
		glFrustum(-0.1, 0.1, -0.1 * ratio, 0.1 * ratio, 0.1, 1000);
		gluLookAt(wnd.camrad * sin(wnd.zang), wnd.camrad * cos(wnd.zang), wnd.posz + 1,
		          0, 0, -pend.r1 + wnd.posz,
		          0, 0, 1);

		// draw content
		glBegin(GL_LINE_STRIP);
			glLineWidth(5);
			glColor3f(0, 1, 1);
			glVertex3f(0, 0, 0);
			DrawVertAt(pend.x1);
			DrawVertAt(pend.x2);
		glEnd();

		glPushMatrix();
			if (wnd.selected == 1)
				glColor3f(1, !wnd.edit, 0);
			else
				glColor3f(1, 1, 1);
			glTranslated(pend.x1.X, pend.x1.Y, pend.x1.Z);
			gluSphere(quadObj, pend.m1 / 10, 10, 10);
		glPopMatrix();
		glPushMatrix();
			if (wnd.selected == 2)
				glColor3f(1, !wnd.edit, 0);
			else
				glColor3f(1, 1, 1);
			glTranslated(pend.x2.X, pend.x2.Y, pend.x2.Z);
			gluSphere(quadObj, pend.m2 / 10, 10, 10);
		glPopMatrix();

		DrawCS();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}
