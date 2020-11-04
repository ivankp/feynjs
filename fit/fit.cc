#include <iostream>
#include <iomanip>
#include <cmath>
#include <numbers>

using std::cout;
using std::endl;
using std::numbers::pi;

template <typename... T>
[[ gnu::always_inline ]]
constexpr auto sq(const T&... x) noexcept { return (... + (x*x)); }

double cbezx(double t, double a) noexcept {
  const double u = 1-t, t2 = t*t;
  return 3*a*u*u*t + 3*(1-a)*u*t2 + t*t2;
}
double cbezy(double t) noexcept {
  static constexpr double b = 4./3;
  return 3*b*t*(1-t);
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

  void rotate(double c, double s) noexcept {
    std::tie(x,y) = std::make_tuple(
      x*c - y*s,
      y*c + x*s
    );
  }
};

int main(int argc, char* argv[]) {
  cout << std::setprecision(9) << std::fixed;

  const double
    g = 2,
    th = fth(g),
    x0 = fx(0,th),
    xbot = fx(pi,th)-x0;

  constexpr unsigned nt = 10;
  point xy1[nt+1], xy2[nt+1];

  { const double tmax = 0.25, dt = tmax/nt;
    for (unsigned i=0; i<=nt; ++i) {
      auto& xy = xy1[i];
      const double t = dt*i;
      xy.x = (fx(2*pi*t,th)-x0)/xbot;
      xy.y = fy(t);
    }
  }

  { const double tmax = 0.5, dt = tmax/nt;
    const double a = atof(argv[1]);
    for (unsigned i=0; i<=nt; ++i) {
      auto& xy = xy2[i];
      const double t = dt*i;
      xy.x = cbezx(t,a);
      xy.y = cbezy(t);
    }
  }

  double chi2 = 0;
  for (unsigned i=0, j=0; i<=nt; ++i) {
    for (;;) {
      auto p1 = xy1[j];
      auto p3 = xy1[j+1] - p1;
      auto p2 = xy2[i] - p1;

      const double
        ang = pi-std::atan2(p3.y,p3.x),
        s = std::sin(ang),
        c = std::cos(ang);

      p2.rotate(c,s);
      p3.rotate(c,s);

      if (!( (p3.x <= p2.x && p2.x <= 0) || (0 >= p2.x && p2.x >= p3.x) )) {
        if ((p2.x<0)==(p3.x<0)) {
          ++j;
          if (j<nt) continue;
          else --j;
        } else {
          --j;
          if (j<nt) continue;
          else ++j;
        }
      }
      chi2 += sq(p2.y);
      break;
    }
  }

  cout << chi2 << endl;
}
