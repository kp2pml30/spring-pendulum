#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "pendulum.h"
#include "shaders.h"
#include "Shader.h"

#ifndef __FAST_MATH__
# warning fast math is not enabled
#endif

namespace
{
	struct WindowData
	{
	private:
		static void EmpF() noexcept {}
	public:
		double dt = 0;
		double posz = -1;
		double zang = 30 * mth::PI / 180;
		double camrad = 3;
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

		std::function<void()> editedCallback = EmpF;
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
		if (!data.edit)
			data.GetEditorPos();
		// pause
		if (key == GLFW_KEY_P && action == GLFW_PRESS)
			data.pend->frozen ^= 1;
		// editor toggle
		else if (key == GLFW_KEY_T && action == GLFW_PRESS)
			data.selected = (data.selected + 1) % (data.pend->ballParams.size() + 1);
		else if (key == GLFW_KEY_F && action == GLFW_PRESS)
			data.edit ^= 1;
		// cam
		else if (key == GLFW_KEY_A && action != GLFW_RELEASE)
			data.zang -= data.dt * 6;
		else if (key == GLFW_KEY_D && action != GLFW_RELEASE)
			data.zang += data.dt * 6;
		else if (key == GLFW_KEY_W && action != GLFW_RELEASE)
			data.posz += 6 * data.dt;
		else if (key == GLFW_KEY_S && action != GLFW_RELEASE)
			data.posz -= 6 * data.dt;
		else if (key == GLFW_KEY_Q && action != GLFW_RELEASE)
			data.camrad += 10 * data.dt;
		else if (key == GLFW_KEY_E && action != GLFW_RELEASE && data.camrad > 0.1)
			data.camrad -= 10 * data.dt;
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
		else if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS)
		{
			data.pend->PopBall();
			data.selected %= data.pend->ballParams.size() + 1;
			data.editedCallback();
		}
		else if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS)
		{
			data.pend->frozen = true;
			double r;
			std::cout << "r : ";
			std::cin >> r;
			double k;
			std::cout << "k : ";
			std::cin >> k;
			double m;
			std::cout << "m : ";
			std::cin >> m;
			data.pend->AddBall({r, m, k});
			data.editedCallback();
		}
	}

	void ShowHelp() noexcept
	{
		std::cout
			<< "WASDQE -- controls\n"
			<< "T -- select ball\n"
			<< "F -- toggle edit mode\n"
			<< "arrows, page up/down -- move ball in edit mode\n"
			<< "[ -- pop ball\n"
			<< "] -- add ball (freezes)\n"
			<< "P -- pause\n"
			<< "\n"
			<< "Note that any editor operation sets all to RungeKutta current\n"
			<< R"delim(
+------------+-------+----------+
|     red    | green |   blue   |
+------------+-------+----------+
| RungeKutta | Euler | midpoint |
+------------+-------+----------+
)delim"
			<< std::endl
			;
	}

	void DrawPend(Pendulum const& pend, WindowData const& wnd, GLUquadricObj* quadObj)
	{
		glBegin(GL_LINE_STRIP);
			glLineWidth(5);
			glColor3f(0, 1, 1);
			glVertex3f(0, 0, 0);
			for (std::size_t i = 0; i < pend.ballParams.size(); i++)
				DrawVertAt(pend.ballCoords[i * 2]);
		glEnd();
		for (std::size_t i = 0; i < pend.ballParams.size(); i++)
		{
			glPushMatrix();
				if (wnd.selected == i + 1)
					glColor3f(1, !wnd.edit, 0);
				else
					glColor3f(1, 1, 1);
				auto tr = pend.ballCoords[i * 2];
				glTranslated(tr.X, tr.Y, tr.Z);
				gluSphere(quadObj, std::cbrt(pend.ballParams[i].m) / 10, 10, 10);
			glPopMatrix();
		}
	}
}

int main()
{
	ShowHelp();

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
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (auto err = glewInit(); err != GLEW_OK)
	{
		std::cout << "glew init error " << glewGetErrorString(err) << std::endl;
		return 1;
	}
	auto shd = Shader(vertexShader, pixelShader);

	Pendulum pend;
	pend.AddBall({0.5, 0.3, 50});
	pend.AddBall({0.2, 0.4, 25});
	pend.AddBall({0.2, 0.1, 20});
	pend.AddBall({0.1, 0.2, 30});
	std::array<Pendulum, 2> other_pends;
	other_pends.fill(pend);

	WindowData wnd;
	wnd.pend = &pend;
	wnd.editedCallback = [&]() {
		other_pends.fill(pend);
	};
	glfwSetWindowUserPointer(window, reinterpret_cast<void*>(&wnd));
	glfwSetKeyCallback(window, key_callback);

	GLUquadricObj* quadObj = gluNewQuadric();
	std::unique_ptr<GLUquadricObj, void (*)(GLUquadricObj*)> flusher(quadObj, gluDeleteQuadric);
	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricNormals(quadObj, GLU_SMOOTH);

	glClearColor(0.3, 0.5, 0.7, 0);
	glEnable(GL_DEPTH_TEST);

	auto prev = std::chrono::system_clock::now();
	while (!glfwWindowShouldClose(window))
	{
		auto now = std::chrono::system_clock::now();
		{
			wnd.dt = std::chrono::duration<double>(now - prev).count();
			prev = std::move(now);
		}

		vec* posprev = nullptr;
		vec* savedV = nullptr;
		vec posprevsaved;
		if (wnd.edit && wnd.selected > 0)
		{
			auto m1 = wnd.selected - 1;
			posprev = &pend.ballCoords[m1 * 2];
			posprevsaved = *posprev;
			savedV = &pend.ballCoords[m1 * 2 + 1];
			other_pends.fill(pend);
		}
		pend.Update(now);
		other_pends[0].Update(now, EulerSolver());
		other_pends[1].Update(now, MidpointSolver());
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
		          0, 0, wnd.posz,
		          0, 0, 1);

		// draw content
		{
			shd.Apply();
			auto ind = shd.GetUniformLocation("idcol");

			glUniform3f(ind, 1, 0.7, 0.7);
			DrawPend(pend, wnd, quadObj);
			if (!wnd.edit || wnd.selected == 0)
			{
				glUniform3f(ind, 0.7, 1, 0.7);
				DrawPend(other_pends[0], wnd, quadObj);
				glUniform3f(ind, 0.7, 0.7, 1);
				DrawPend(other_pends[1], wnd, quadObj);
			}

			Shader::ApplyDflt();
		}

		DrawCS();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}
