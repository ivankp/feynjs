#include <iostream>
#include <cmath>
#include <numbers>
#include <type_traits>
#include <fmt/core.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_min.h>

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
class minimizer {
  F f;
  gsl_min_fminimizer * const s;
  gsl_function g;
  static double wrap(double x, void* f) {
    return (*reinterpret_cast<F*>(f))(x);
  }
public:
  template <typename T>
  minimizer(
    T&& f, const gsl_min_fminimizer_type* t,
    double x0, double x_lower, double x_upper
  ): f(std::forward<T>(f)), s(gsl_min_fminimizer_alloc(t)) {
    g.function = &wrap;
    g.params = &f;
    gsl_min_fminimizer_set(s, &g, x0, x_lower, x_upper);
  }
  ~minimizer() {
    gsl_min_fminimizer_free(s);
  }

  auto name   () { return gsl_min_fminimizer_name(s); }
  auto iterate() { return gsl_min_fminimizer_iterate(s); }
  auto minimum() { return gsl_min_fminimizer_x_minimum(s); }
  auto lower  () { return gsl_min_fminimizer_x_lower(s); }
  auto upper  () { return gsl_min_fminimizer_x_upper(s); }
};

template <typename F>
minimizer(F&&,...) -> minimizer<std::decay_t<F>>;

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

  double a1=0.3, a2=0.5, a=(a1+a2)/2;

  minimizer m(
    [=](double a){
    /*
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
    */
      return sq(a-0.45);
    },
    gsl_min_fminimizer_brent,
    // gsl_min_fminimizer_goldensection,
    // gsl_min_fminimizer_quad_golden,
    a, a1, a2
  );

  cout << "minimization algorithm: " << m.name() << '\n';

  cout << format("{:5} [{:9}, {:9}] {:9} {:12}\n",
          "iter", "lower", "upper", "min", "l - u");

  bool stop = false;
  for (unsigned iter=0; ; ++iter) {
    cout << format("{:5} [{:.7f}, {:.7f}] {:.7f} {:.7e}\n",
            iter, a1, a2, a, a2-a1);

    if (stop || iter >= 1000) break;

    m.iterate();

    a  = m.minimum();
    a1 = m.lower();
    a2 = m.upper();

    if (gsl_min_test_interval(a1, a2, 1e-4, 0) != GSL_CONTINUE)
      stop = true;
  }

  delete[] xy1;
  delete[] xy2;
}
