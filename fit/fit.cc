#include <iostream>
#include <cmath>
#include <numbers>
#include <limits>
#include <type_traits>
#include <stdexcept>
#include <fmt/core.h>

using std::cout;
using std::endl;
using std::numbers::pi;
using fmt::format;

template <typename... T>
[[ gnu::always_inline ]]
constexpr auto sq(const T&... x) noexcept { return (... + (x*x)); }

double bezx(double t, double a) noexcept {
  const double u = 1-t, t2 = t*t;
  return 3*a*u*u*t + 3*(1-a)*u*t2 + t*t2;
}
double bezy(double t) noexcept {
  return 4*t*(1-t); // b = 4/3
}

double fth(double g) noexcept {
  return std::atan(pi*(1/g - 0.5))+pi/2;
}
double fx(double t, double th) noexcept {
  return std::cos(t)*std::cos(th) + t*std::sin(th);
}
double fy(double t) noexcept {
  return std::sin(2*pi*t);
}

struct point {
  double x, y;

  point operator-(const point& o) const noexcept {
    return { x-o.x, y-o.y };
  }
  double operator*(const point& o) const noexcept {
    return sq(x-o.x,y-o.y);
  }
};

template <typename F>
struct brent {
  F f;
  double x0, x1, x2, f0, f1, f2;

  template <typename T>
  brent(T&& f, double x0, double x1, double x2)
  : f(std::forward<T>(f)), x0(x0), x1(x1), x2(x2), f1(f(x1)), f2(f(x2))
  { }
  void operator()() {
    f0 = f(x0);
    const double
      dx1 = x0 - x1,
      dx2 = x0 - x2,
      df1 = f0 - f1,
      df2 = f0 - f2,
      m = x0 - 0.5*( dx1*dx1*df2 - dx2*dx2*df1 )/( dx1*df2 - dx2*df1 );
    if (!( x1 <= m && m <= x2 )) throw std::range_error(format(
      "minimum abscissa {:.8e} out of range [{:.8e}, {:.8e}]", m, x1, x2));
    if (m < x0) {
      x2 = x0;
      f2 = f0;
    } else {
      x1 = x0;
      f1 = f0;
    }
    x0 = m;
  }
};

template <typename F>
brent(F&&,...) -> brent<std::decay_t<F>>;

int main(int argc, char* argv[]) {
  const double
    g = 2,
    th = fth(g),
    x0 = fx(0,th),
    xbot = fx(pi,th)-x0;

  const unsigned nt = 100;
  point *const xy1 = new point[nt+1],
        *const xy2 = new point[nt+1];

  { const double tmax = 0.25, dt = tmax/nt;
    for (unsigned i=0; i<=nt; ++i) {
      auto& xy = xy1[i];
      const double t = dt*i;
      xy.x = (fx(2*pi*t,th)-x0)/xbot;
      xy.y = fy(t);
    }
  }

  brent m([=](double a){
    const double tmax = 0.5, dt = tmax/nt;
    for (unsigned i=0; i<=nt; ++i) {
      auto& xy = xy2[i];
      const double t = dt*i;
      xy.x = bezx(t,a);
      xy.y = bezy(t);
    }

    double chi2 = 0;
    for (unsigned i=0, j=0, k; i<=nt; ++i) {
      double d = xy1[i]*xy2[j], d2;
      for (k=j; k<nt; ) {
        d2 = xy1[i]*xy2[++k];
        if (d2 > d) { --k; break; }
        else d = d2;
      }
      if (k==j) for (; k>0; ) {
        d2 = xy1[i]*xy2[--k];
        if (d2 > d) { ++k; break; }
        else d = d2;
      }
      chi2 += d;
    }

    return chi2;
  }, 0.2, 0., 0.5);

  cout << format("{:>5} [{:14}, {:14}] {:14} {}\n",
          "iter", "lower", "upper", "min", "l - u");

  bool stop = false;
  for (unsigned iter=0; ; ++iter) {
    cout << format("{:>5} [{:.8e}, {:.8e}] {:.8e} {:.8e}\n",
            iter, m.x1, m.x2, m.x0, m.x2-m.x1);

    if (stop || iter >= 1000) break;

    const double a1 = m.x0;
    m();

    if (m.x2-m.x1 < 1e-6 || std::abs(a1-m.x0) < 1e-8)
      stop = true;
  }

  delete[] xy1;
  delete[] xy2;
}
