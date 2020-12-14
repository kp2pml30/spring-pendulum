#pragma once

#include "mth/vec.h"

#include <chrono>
#include <limits>
#include <valarray>
#include <vector>

/*
 * Returns delta
 */
template<typename F, typename V, typename T>
V RungeKutta(F const& f, V const& x0, T h)
{
	auto k1 = f(x0, 0);
	auto hd2 = h / 2;
	auto k2 = f(x0 + hd2 * k1, hd2);
	auto k3 = f(x0 + hd2 * k2, hd2);
	auto k4 = f(x0 + h   * k3, h);
	return h / 6 * (k1 + k2 / 2 + k3 / 2 + k4);
}

using vec = mth::vec<double>;

class Pendulum
{
private:
	std::chrono::time_point<std::chrono::system_clock> prev = std::chrono::system_clock::now();
public:
	struct BallData
	{
		double
			r = 5,
			m = 3,
			k = 10;
	};
	std::vector<BallData> ballParams;
	std::valarray<vec> ballCoords; // as {{x, v}, ...}
	Pendulum& AddBall(BallData const& bd, vec x0 = {0, 0, std::numeric_limits<double>::quiet_NaN()}, vec v0 = {0, 0, 0})
	{
		if (std::isnan(x0.Z))
			if (ballParams.empty())
				x0.Z = bd.r;
			else
				x0 = ballCoords[ballCoords.size() - 2];
		return *this;
	}

	double r1 = 5, r2 = 2;
	vec x1 = {0, 0, -r1}, x2 = {0, 0, -r1 - r2};
	vec v1 = {0, 0, 0}, v2 = {0, 0, 0};
	double m1 = 3, m2 = 4;
	double k1 = 30, k2 = 10;

	vec g = {0, 0, -9.8};

	bool frozen = false;

	void Update()
	{
		using namespace mth;
		using vec = ::vec;
		auto now = std::chrono::system_clock::now();
		if (frozen)
		{
			prev = std::move(now);
			return;
		}
		std::chrono::duration<double> delta1 = now - prev;
		auto delta = delta1.count() * 10;
		prev = std::move(now);

		using funny = mth::vec4<vec>;

		// f > 0 <=> spring got longer => force is directed to collapse
		auto accel1 = [&](vec x1, vec x2, double f2) {
			return -(1 - r1 / x1.Len()) * k1 / m1 * x1 + (x2 - x1) * f2 / m1 + g;
		};
		auto accel2 = [&](vec x1, vec x2, double f2) {
			return f2 * (x1 - x2) / m2 + g;
		};
		/*  y'   val
		 * x1' = v1
		 * x2' = v2
		 * v1' = a1
		 * v2' = a2
		 */
		auto deltacalc = [&](funny p, double delta) {
					auto f2 = (1 - r2 / (x2 - x1).Len()) * k2;
					return funny(
							p.Y * delta,
							accel1(p.X, p.Z, f2) * delta,
							p.W * delta,
							accel2(p.X, p.Z, f2) * delta
						);
				};
		auto res = RungeKutta(
				deltacalc,
				funny(x1, v1, x2,  v2),
				delta
			);
		x1 += res.X;
		v1 += res.Y;
		x2 += res.Z;
		v2 += res.W;
	}
};

