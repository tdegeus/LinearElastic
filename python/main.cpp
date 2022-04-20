/**
\file
\copyright Copyright. Tom de Geus. All rights reserved.
\license This project is released under the MIT License.
*/

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#define FORCE_IMPORT_ARRAY
#include <xtensor-python/pytensor.hpp>

#define GMATELASTIC_USE_XTENSOR_PYTHON
#define GMATTENSOR_USE_XTENSOR_PYTHON
#include <GMatElastic/Cartesian3d.h>
#include <GMatElastic/version.h>
#include <GMatTensor/Cartesian3d.h>

namespace py = pybind11;

namespace my3d {

template <class S, class T>
auto Elastic(T& cls)
{
    cls.def(
        py::init<const xt::pytensor<double, S::rank>&, const xt::pytensor<double, S::rank>&>(),
        "Heterogeneous system.",
        py::arg("K"),
        py::arg("G"));

    cls.def_property_readonly("shape", &S::shape, "Shape of array.");
    cls.def_property_readonly("shape_tensor2", &S::shape_tensor2, "Array of rank 2 tensors.");
    cls.def_property_readonly("shape_tensor4", &S::shape_tensor4, "Array of rank 4 tensors.");
    cls.def_property_readonly("K", &S::K, "Bulk modulus.");
    cls.def_property_readonly("G", &S::G, "Shear modulus.");
    cls.def_property_readonly("Sig", &S::Sig, "Stress tensor.");
    cls.def_property_readonly("C", &S::C, "Tangent tensor.");
    cls.def_property_readonly("energy", &S::energy, "Potential energy.");

    cls.def_property(
        "Eps",
        static_cast<xt::pytensor<double, S::rank + 2>& (S::*)()>(&S::Eps),
        &S::template set_Eps<xt::pytensor<double, S::rank + 2>>,
        "Strain tensor");

    cls.def("refresh", &S::refresh, "Recompute stress from strain.");
    cls.def("__repr__", [](const S&) { return "<GMatElastic.Cartesian3d.Elastic>"; });
}

template <class R, class T, class M>
void Epseq(M& mod)
{
    mod.def(
        "Epseq",
        static_cast<R (*)(const T&)>(&GMatElastic::Cartesian3d::Epseq),
        "Equivalent strain of a(n) (array of) tensor(s).",
        py::arg("A"));
}

template <class R, class T, class M>
void epseq(M& mod)
{
    mod.def(
        "epseq",
        static_cast<void (*)(const T&, R&)>(&GMatElastic::Cartesian3d::epseq),
        "Equivalent strain of a(n) (array of) tensor(s).",
        py::arg("A"),
        py::arg("ret"));
}

template <class R, class T, class M>
void Sigeq(M& mod)
{
    mod.def(
        "Sigeq",
        static_cast<R (*)(const T&)>(&GMatElastic::Cartesian3d::Sigeq),
        "Equivalent stress of a(n) (array of) tensor(s).",
        py::arg("A"));
}

template <class R, class T, class M>
void sigeq(M& mod)
{
    mod.def(
        "sigeq",
        static_cast<void (*)(const T&, R&)>(&GMatElastic::Cartesian3d::sigeq),
        "Equivalent stress of a(n) (array of) tensor(s).",
        py::arg("A"),
        py::arg("ret"));
}

} // namespace my3d

/**
Overrides the `__name__` of a module.
Classes defined by pybind11 use the `__name__` of the module as of the time they are defined,
which affects the `__repr__` of the class type objects.
*/
class ScopedModuleNameOverride {
public:
    explicit ScopedModuleNameOverride(py::module m, std::string name) : module_(std::move(m))
    {
        original_name_ = module_.attr("__name__");
        module_.attr("__name__") = name;
    }
    ~ScopedModuleNameOverride()
    {
        module_.attr("__name__") = original_name_;
    }

private:
    py::module module_;
    py::object original_name_;
};

PYBIND11_MODULE(_GMatElastic, m)
{
    ScopedModuleNameOverride name_override(m, "GMatElastic");

    xt::import_numpy();

    m.doc() = "Linear elastic material model";

    m.def("version", &GMatElastic::version, "Version string.");

    m.def(
        "version_dependencies",
        &GMatElastic::version_dependencies,
        "List of version strings, include dependencies.");

    m.def(
        "version_compiler",
        &GMatElastic::version_compiler,
        "Information on the compiler, the platform, the C++ standard, and the compilation data.");

    // -----------------------
    // GMatElastic.Cartesian3d
    // -----------------------

    py::module sm = m.def_submodule("Cartesian3d", "3d Cartesian coordinates");

    namespace SM = GMatElastic::Cartesian3d;

    // Tensor algebra

    my3d::Epseq<xt::pytensor<double, 2>, xt::pytensor<double, 4>>(sm);
    my3d::Epseq<xt::pytensor<double, 1>, xt::pytensor<double, 3>>(sm);
    my3d::Epseq<xt::pytensor<double, 0>, xt::pytensor<double, 2>>(sm);

    my3d::epseq<xt::pytensor<double, 2>, xt::pytensor<double, 4>>(sm);
    my3d::epseq<xt::pytensor<double, 1>, xt::pytensor<double, 3>>(sm);
    my3d::epseq<xt::pytensor<double, 0>, xt::pytensor<double, 2>>(sm);

    my3d::Sigeq<xt::pytensor<double, 2>, xt::pytensor<double, 4>>(sm);
    my3d::Sigeq<xt::pytensor<double, 1>, xt::pytensor<double, 3>>(sm);
    my3d::Sigeq<xt::pytensor<double, 0>, xt::pytensor<double, 2>>(sm);

    my3d::sigeq<xt::pytensor<double, 2>, xt::pytensor<double, 4>>(sm);
    my3d::sigeq<xt::pytensor<double, 1>, xt::pytensor<double, 3>>(sm);
    my3d::sigeq<xt::pytensor<double, 0>, xt::pytensor<double, 2>>(sm);

    // Elastic

    py::class_<SM::Elastic<0>, GMatTensor::Cartesian3d::Array<0>> array0d(sm, "Elastic0d");
    py::class_<SM::Elastic<1>, GMatTensor::Cartesian3d::Array<1>> array1d(sm, "Elastic1d");
    py::class_<SM::Elastic<2>, GMatTensor::Cartesian3d::Array<2>> array2d(sm, "Elastic2d");
    py::class_<SM::Elastic<3>, GMatTensor::Cartesian3d::Array<3>> array3d(sm, "Elastic3d");

    my3d::Elastic<SM::Elastic<0>>(array0d);
    my3d::Elastic<SM::Elastic<1>>(array1d);
    my3d::Elastic<SM::Elastic<2>>(array2d);
    my3d::Elastic<SM::Elastic<3>>(array3d);
}
