#include "CommonPybindUtilities.h"
#include "MParT/MapOptions.h"
#include <pybind11/stl.h>
#include <pybind11/eigen.h>

#include <Kokkos_Core.hpp>
#include <pybind11/pybind11.h>

#if defined(MPART_HAS_CEREAL)
#include "MParT/Utilities/Serialization.h"
#include <fstream>
#endif // MPART_HAS_CEREAL

namespace py = pybind11;
using namespace mpart::binding;

void mpart::binding::MapOptionsWrapper(py::module &m)
{

    // BasisTypes
    py::enum_<BasisTypes>(m, "BasisTypes")
    .value("ProbabilistHermite",BasisTypes::ProbabilistHermite)
    .value("PhysicistHermite",BasisTypes::PhysicistHermite)
    .value("HermiteFunctions",BasisTypes::HermiteFunctions);

    // PosFuncTypes
    py::enum_<PosFuncTypes>(m, "PosFuncTypes")
    .value("Exp",PosFuncTypes::Exp)
    .value("SoftPlus",PosFuncTypes::SoftPlus);

    // QuadTypes
    py::enum_<QuadTypes>(m, "QuadTypes")
    .value("ClenshawCurtis",QuadTypes::ClenshawCurtis)
    .value("AdaptiveSimpson",QuadTypes::AdaptiveSimpson)
    .value("AdaptiveClenshawCurtis",QuadTypes::AdaptiveClenshawCurtis);

    // SigmoidTypes
    py::enum_<SigmoidTypes>(m, "SigmoidTypes")
    .value("Logistic",SigmoidTypes::Logistic);

    // EdgeTypes
    py::enum_<EdgeTypes>(m, "EdgeTypes")
    .value("SoftPlus",EdgeTypes::SoftPlus);

    // MapOptions
    py::class_<MapOptions, std::shared_ptr<MapOptions>>(m, "MapOptions")
    .def(py::init<>())
    .def("__eq__", &MapOptions::operator==)
    .def("__str__", &MapOptions::String)
    .def("__repr__", [](MapOptions opts){return "<MapOptions with fields\n" + opts.String() + ">";})
    .def_readwrite("basisType", &MapOptions::basisType)
    .def_readwrite("basisLB", &MapOptions::basisLB)
    .def_readwrite("basisUB", &MapOptions::basisUB)
    .def_readwrite("basisNorm", &MapOptions::basisNorm)
    .def_readwrite("posFuncType", &MapOptions::posFuncType)
    .def_readwrite("edgeType", &MapOptions::edgeType)
    .def_readwrite("edgeShape", &MapOptions::edgeShape)
    .def_readwrite("sigmoidType", &MapOptions::sigmoidType)
    .def_readwrite("quadType", &MapOptions::quadType)
    .def_readwrite("quadAbsTol", &MapOptions::quadAbsTol)
    .def_readwrite("quadRelTol", &MapOptions::quadRelTol)
    .def_readwrite("quadMaxSub", &MapOptions::quadMaxSub)
    .def_readwrite("quadMinSub", &MapOptions::quadMinSub)
    .def_readwrite("quadPts", &MapOptions::quadPts)
    .def_readwrite("contDeriv", &MapOptions::contDeriv)
    .def_readwrite("nugget", &MapOptions::nugget)
    #if defined(MPART_HAS_CEREAL)
    .def("Serialize", [](MapOptions const &opts, std::string const &filename){
        std::ofstream os (filename);
        cereal::BinaryOutputArchive oarchive(os);
        oarchive(opts);
    })
    .def("Deserialize", [](MapOptions &opts, std::string const &filename){
        std::ifstream is(filename);
        cereal::BinaryInputArchive iarchive(is);
        iarchive(opts);
        return opts;
    })
    #endif // MPART_HAS_CEREAL
    ;

}