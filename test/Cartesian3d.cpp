
#include <catch2/catch.hpp>

#define EQ(a,b) REQUIRE_THAT((a), Catch::WithinAbs((b), 1.e-12));

#include <GMatElastic/Cartesian3d.h>

namespace GM = GMatElastic::Cartesian3d;

TEST_CASE("GMatElastic::Cartesian3d", "Cartesian3d.h")
{

double K = 12.3;
double G = 45.6;

GM::Tensor2 Eps;
GM::Tensor2 Sig;

double gamma = 0.02;
double epsm = 0.12;

Eps.fill(0.0);
Eps(0,0) = Eps(1,1) = Eps(2,2) = epsm;
Eps(0,1) = Eps(1,0) = gamma;

SECTION("Elastic")
{
    GM::Elastic mat(K, G);
    Sig = mat.Stress(Eps);

    EQ(Sig(0,0), 3.0 * K * epsm);
    EQ(Sig(1,1), 3.0 * K * epsm);
    EQ(Sig(2,2), 3.0 * K * epsm);
    EQ(Sig(0,1), 2.0 * G * gamma);
    EQ(Sig(1,0), 2.0 * G * gamma);
    EQ(Sig(0,2), 0.0);
    EQ(Sig(1,2), 0.0);
    EQ(Sig(2,0), 0.0);
    EQ(Sig(2,1), 0.0);
}

SECTION("Matrix")
{
    size_t nelem = 3;
    size_t nip = 2;

    GM::Matrix mat(nelem, nip);

    {
        xt::xtensor<size_t,2> I = xt::ones<size_t>({nelem, nip});
        mat.setElastic(I, K, G);
    }

    xt::xtensor<double,4> eps = xt::empty<double>({nelem, nip, 3ul, 3ul});

    for (size_t e = 0; e < nelem; ++e) {
        for (size_t q = 0; q < nip; ++q) {
            xt::view(eps, e, q) = Eps;
        }
    }

    auto sig = mat.Stress(eps);

    for (size_t e = 0; e < nelem; ++e) {
        for (size_t q = 0; q < nip; ++q) {
            EQ(sig(e,q,0,0), 3.0 * K * epsm);
            EQ(sig(e,q,1,1), 3.0 * K * epsm);
            EQ(sig(e,q,2,2), 3.0 * K * epsm);
            EQ(sig(e,q,0,1), 2.0 * G * gamma);
            EQ(sig(e,q,1,0), 2.0 * G * gamma);
        }
    }

    REQUIRE(xt::allclose(xt::view(sig, xt::all(), xt::all(), xt::keep(0), xt::keep(2)), 0.0));
    REQUIRE(xt::allclose(xt::view(sig, xt::all(), xt::all(), xt::keep(1), xt::keep(2)), 0.0));
    REQUIRE(xt::allclose(xt::view(sig, xt::all(), xt::all(), xt::keep(2), xt::keep(0)), 0.0));
    REQUIRE(xt::allclose(xt::view(sig, xt::all(), xt::all(), xt::keep(2), xt::keep(1)), 0.0));
}

}
