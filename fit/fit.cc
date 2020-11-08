#include <iostream>
#include <cmath>
#include <cstring>
#include <numbers>
#include <limits>
#include <type_traits>
#include <stdexcept>
#include <fmt/core.h>
#include "wls.hh"

using std::cout;
using std::cerr;
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
      dx1df2 = dx1*df2,
      dx2df1 = dx2*df1,
      m = x0 - 0.5*( dx1*dx1df2 - dx2*dx2df1 )/( dx1df2 - dx2df1 );
    if (m < x1 || x2 < m) throw std::range_error(format(
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
brent(F&&,...) -> brent<std::remove_reference_t<F>>;

int main(int argc, char* argv[]) {
  if (argc < 3) {
    cout << "usage: " << argv[0] << " nt ng [verbose (0,1,2)]\n";
    return 1;
  }
  const unsigned
    nt = atoi(argv[1]),
    ng = atoi(argv[2]),
    verbose = (argc>3 ? atoi(argv[3]) : 0);

  point *const xy1 = new point[nt+1],
        *const xy2 = new point[nt+1];

  double *const gs = new double[(ng+1)*2],
         *const as = gs + (ng+1);

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
        const auto p = xy1[i];
        // find nearest point
        double d = p*xy2[j], d2;
        for (k=j; k<nt; ) {
          d2 = p*xy2[++k];
          if (d2 > d) { --k; break; }
          else d = d2;
        }
        if (k==j) for (; k>0; ) {
          d2 = p*xy2[--k];
          if (d2 > d) { ++k; break; }
          else d = d2;
        }
        j = k;

        // linear interpolation
        for (int k=-1; k<2; k+=2) {
          if (j+k > nt) continue;
          point p0;
          auto p1 = xy2[j];
          auto p2 = xy2[j+k];
          auto p3 = p;
          double a = (p1.y-p2.y)/(p1.x-p2.x), b;
#define SWAP(x,y) b = x; x = y; y = b;
          if (a > 1) { // flip axes to keep slope below 1 for precision
            SWAP(p1.x,p1.y);
            SWAP(p2.x,p2.y);
            SWAP(p3.x,p3.y);
            a = 1/a;
          }
          b = p1.y - a*p1.x;
          p0.x = (p3.x + a*(p3.y-b))/(1+a*a);
          double xmin = p1.x, xmax = p2.x;
          if (xmin > xmax) std::swap(xmin,xmax);
          if (xmin <= p0.x && p0.x <= xmax) {
            p0.y = a*p0.x + b;
            d2 = p0*p3;
            if (d2 < d) d = d2;
          }
        }

        chi2 += d;
      }

      return chi2;
    }, 0.2, 0., 0.5);

    if (verbose > 1)
      cout << format("{:>5} [{:14}, {:14}] {:14} {}\n",
              "iter", "lower", "upper", "min", "l - u");

    double a = std::numeric_limits<double>::quiet_NaN();
    const unsigned max_iter = 100;
    for (unsigned iter=0; ; ++iter) {
      if (verbose > 1)
        cout << format("{:>5} [{:.8e}, {:.8e}] {:.8e} {:.8e}\n",
                iter, m.x1, m.x2, m.x0, m.x2-m.x1);

      if (std::abs(a-m.x0) < 1e-8 || iter > max_iter) {
        a = m.x0;
        if (iter > max_iter)
          cerr << "exceeded maximum number of iterations" << endl;
        break;
      }

      a = m.x0;
      try {
        m();
      } catch (const std::range_error& e) {
        if (verbose > 1)
          cout << e.what() << endl;
        if (m.x0 < m.x1) {
          m.x1 = 2*m.x0-m.x2;
        } else {
          m.x2 = 2*m.x0-m.x1;
        }
      }
    }

    if (verbose > 1)
      cout << format("\ng = {:.8e}, a = {:.8e}\n\n",g,a);

    gs[gi] = g;
    as[gi] = a;
  }

  if (verbose > 0) {
    cout << '{';
    for (unsigned gi=0; gi<=ng; ++gi) {
      if (gi) cout << ',';
      cout << format("{{{:.2f},{:.8f}}}",gs[gi],as[gi]);
    }
    cout << "}\n\n";
  }

  // fit f: g -> a
  double cs[2];
  const unsigned nc = std::size(cs);
  double * const A = new double[(ng+1)*nc];
  { double* a = A;
    for (unsigned i=0; i<=ng; ++i)
      *a++ = 1;
    for (unsigned i=0; i<=ng; ++i)
      *a++ = gs[i];
  }

  ivanp::wls(A, as, nullptr, ng+1, nc, cs);

  cout << format("a0 = {:.12f}\na' = {:.12f}\n",cs[0],cs[1]);

  delete[] xy1;
  delete[] xy2;
  delete[] gs;
  delete[] A;
}
