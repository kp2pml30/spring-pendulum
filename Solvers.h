#pragma once

#include "mth/vec.h"

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

template<typename F, typename V, typename T>
V RungeKutta(F const& f, V const& x0, T h)
{
	using namespace mth;
	// optimized: inplace operations are always faster
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
	// h / 6
	hd2 /= 3;
	k1 *= hd2;
	return k1;
}

template<typename F, typename V, typename T>
V Euler(F const& f, V const& x0, T h)
{
	using namespace mth;
	auto res = f(x0, h);
	res *= h;
	// res /= 1.4405;
	return res;
}

template<typename F, typename V, typename T>
V Midpoint(F const& f, V const& x0, T h)
{
	using namespace mth;
	auto k1 = f(x0, 0);
	auto hd2 = h / 2;
	k1 *= hd2;
	k1 += x0;
	auto ret = f(k1, hd2);
	ret *= h;
	return ret;
}

struct RungeKuttaSolver
{
	template<typename ...A>
	auto operator()(A&& ...a) const { return RungeKutta(std::forward<A>(a)...); }
};
struct EulerSolver
{
	template<typename ...A>
	auto operator()(A&& ...a) const { return Euler(std::forward<A>(a)...); }
};
struct MidpointSolver
{
	template<typename ...A>
	auto operator()(A&& ...a) const { return Midpoint(std::forward<A>(a)...); }
};


