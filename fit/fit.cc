#include <iostream>
#include <cmath>
#include <cstring>
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
  if (argc < 3) {
    cout << "usage: " << argv[0] << " nt ng [-v]\n";
    return 1;
  }
  const unsigned
    nt = atoi(argv[1]),
    ng = atoi(argv[2]);

  const bool verbose = argc > 3 && !strcmp(argv[3],"-v");

  point *const xy1 = new point[nt+1],
        *const xy2 = new point[nt+1],
        *const ga  = new point[ng+1];

  const double dg = 2./ng;
  for (unsigned gi=0; gi<=ng; ++gi) {
    const double
      g = dg*gi,
      th = fth(g),
      x0 = fx(0,th),
      xbot = fx(pi,th)-x0;

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

    if (verbose)
      cout << format("{:>5} [{:14}, {:14}] {:14} {}\n",
              "iter", "lower", "upper", "min", "l - u");

    double a = std::numeric_limits<double>::quiet_NaN();
    for (unsigned iter=0; ; ++iter) {
      if (verbose)
        cout << format("{:>5} [{:.8e}, {:.8e}] {:.8e} {:.8e}\n",
                iter, m.x1, m.x2, m.x0, m.x2-m.x1);

      if (m.x2-m.x1 < 1e-8 || std::abs(a-m.x0) < 1e-9 || iter >= 1000) {
        a = m.x0;
        break;
      }

      a = m.x0;
      try {
        m();
      } catch (const std::range_error& e) {
        if (verbose)
          cout << e.what() << endl;
        // break;
        if (m.x0 < m.x1) {
          m.x1 = 2*m.x0-m.x2;
        } else {
          m.x2 = 2*m.x0-m.x1;
        }
      }
    }

    if (verbose)
      cout << format("\ng = {:.8e}, a = {:.8e}\n\n",g,a);

    ga[gi] = { g, a };
  }

  cout << '{';
  for (unsigned gi=0; gi<=ng; ++gi) {
    if (gi) cout << ',';
    cout << format("{{{:.2f},{:.8f}}}",ga[gi].x,ga[gi].y);
  }
  cout << "}\n";

  delete[] xy1;
  delete[] xy2;
  delete[] ga;
}
