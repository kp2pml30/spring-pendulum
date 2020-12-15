#pragma once

#include "mth/vec.h"

#include <chrono>
#include <limits>
#include <type_traits>
#include <vector>
#include <valarray>

template<typename T, typename Y>
std::valarray<T>& operator*=(std::valarray<T>& l, Y const& r)
{
	using namespace mth;
	for (std::size_t i = 0; i < l.size(); i++)
		l[i] *= r;
	return l;
}
template<typename T, typename Y>
std::valarray<T> operator*(std::valarray<T> const& l, Y const& r)
{
	auto copy = l;
	copy *= r;
	return copy;
}
template<typename T, typename Y>
std::valarray<T>& operator/=(std::valarray<T>& l, Y const& r)
{
	using namespace mth;
	for (std::size_t i = 0; i < l.size(); i++)
		l[i] /= r;
	return l;
}

/*
 * Returns delta
 */
template<typename F, typename V, typename T>
V RungeKutta(F const& f, V const& x0, T h)
{
	using namespace mth;
	auto k1 = f(x0, 0);
	auto hd2 = h / 2;
	auto k2 = f(x0 + k1 * hd2, hd2);
	auto k3 = f(x0 + k2 * hd2, hd2);
	auto k4 = f(x0 + k3 * h  , h);
	k2 *= 2;
	k3 *= 2;
	k1 += k2;
	k1 += k3;
	k1 += k4;
	k1 *= h / 6;
	return k1;
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

	Pendulum& PopBall() noexcept
	{
		ballParams.pop_back();
		auto nv = std::valarray<vec>(ballCoords.size() - 2);
		for (std::size_t i = 0; i < ballCoords.size() - 2; i++)
			nv[i] = ballCoords[i];
		ballCoords = std::move(nv);
		return *this;
	}

	Pendulum& AddBall(BallData const& bd, vec x0 = {0, 0, -1e40}, vec v0 = {0, 0, 0})
	{
		// I wanted to use nan but fast-math kills everything
		if (x0.Z == -1e40)
			if (ballParams.empty())
				x0.Z = -bd.r;
			else
				x0 = ballCoords[ballCoords.size() - 2] + vec(0, 0, -bd.r);
		ballParams.emplace_back(bd);
		auto nv = std::valarray<vec>(ballCoords.size() + 2);
		for (std::size_t i = 0; i < ballCoords.size(); i++)
			nv[i] = ballCoords[i];
		ballCoords = std::move(nv);
		ballCoords[ballCoords.size() - 2] = x0;
		ballCoords[ballCoords.size() - 1] = v0;
		return *this;
	}

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
		auto delta = delta1.count();
#if 0
		if (delta < 0.05)
			return;
#endif

		prev = std::move(now);

		if (ballParams.empty())
			return;
		assert(ballParams.size() * 2 == ballCoords.size());

		// f > 0 <=> spring got longer => force is directed to collapse
		auto deltacalc = [&](auto const& p, double delta) {
					auto ret = std::valarray<vec>(p.size());
					auto fp = (1 - ballParams[0].r / ballCoords[0].Len()) * ballParams[0].k;
					vec xp= vec(0);
					for (std::size_t i = 0; i < ballParams.size() - 1; i++)
					{
						auto const& par = ballParams[i];
						auto const& parn = ballParams[i + 1];
						auto const& xm = p[i * 2];
						auto const& vm = p[i * 2 + 1];
						auto const& xn = p[i * 2 + 2];
						ret[i * 2] = vm * delta;
						auto fn = (1 - parn.r / (xn - xm).Len()) * par.k;
						ret[i * 2 + 1] = fp / par.m * (xp - xm) + (xn - xm) * fn / par.m + g;
						fp = fn;
						xp = xm;
					}
					ret[ret.size() - 2] = p[p.size() - 1] * delta;
					ret[ret.size() - 1] = fp * (xp - p[p.size() - 2]) / ballParams.back().m + g;
					return ret;
				};
		auto res = RungeKutta(
				deltacalc,
				ballCoords,
				delta
			);
		ballCoords += res;
	}
};

