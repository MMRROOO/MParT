#include "CommonPybindUtilities.h"
#include "MParT/TriangularMap.h"
#include "MParT/ConditionalMapBase.h"
#include <pybind11/stl.h>
#include <pybind11/eigen.h>

#include <Kokkos_Core.hpp>
#include <pybind11/pybind11.h>

namespace py = pybind11;
using namespace mpart::binding;

void mpart::binding::TriangularMapWrapper(py::module &m)
{

    // TriangularMap
     py::class_<TriangularMap, ConditionalMapBase, KokkosCustomPointer<TriangularMap>>(m, "TriangularMap")

        .def(py::init( [](std::vector<KokkosCustomPointer<ConditionalMapBase>> const& components)
        {
            std::vector<std::shared_ptr<ConditionalMapBase>> newComponents;
            for(auto& ptr : components)
                newComponents.push_back(ptr.impl);
            return new TriangularMap(newComponents);

        }))
        ;


}