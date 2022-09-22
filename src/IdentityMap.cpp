#include "MParT/IdentityMap.h"

#include <numeric>

using namespace mpart;

template<typename MemorySpace>
IdentityMap<MemorySpace>::IdentityMap(unsigned int inDim, unsigned int outDim) : ConditionalMapBase<MemorySpace>(inDim, outDim, 0)
{

}



template<typename MemorySpace>
void IdentityMap<MemorySpace>::LogDeterminantImpl(StridedMatrix<const double, MemorySpace> const& pts,
                                                    StridedVector<double, MemorySpace>              output)
{

    // Add to logdet of full map
    for(unsigned int j=0; j<output.size(); ++j)
        output(j) = 0.0;


}

template<typename MemorySpace>
void IdentityMap<MemorySpace>::EvaluateImpl(StridedMatrix<const double, MemorySpace> const& pts,
                                              StridedMatrix<double, MemorySpace>              output)
{
    // Copy x_{N-M+1:N}
    StridedMatrix<const double, MemorySpace> tailPts = Kokkos::subview(
        pts, std::make_pair(int(this->inputDim - this->outputDim), int(this->inputDim)), Kokkos::ALL());
    Kokkos::deep_copy(output, tailPts);

}

template<typename MemorySpace>
void IdentityMap<MemorySpace>::InverseImpl(StridedMatrix<const double, MemorySpace> const& x1,
                                             StridedMatrix<const double, MemorySpace> const& r,
                                             StridedMatrix<double, MemorySpace>              output)
{

    Kokkos::deep_copy(output, r);

}


template<typename MemorySpace>
void IdentityMap<MemorySpace>::CoeffGradImpl(StridedMatrix<const double, MemorySpace> const& pts,  
                                               StridedMatrix<const double, MemorySpace> const& sens,
                                               StridedMatrix<double, MemorySpace>              output)
{
    assert(false);
}


void IdentityMap<MemorySpace>::GradientImpl(StridedMatrix<const double, MemorySpace> const& pts,  
                            StridedMatrix<const double, MemorySpace> const& sens,
                            StridedMatrix<double, MemorySpace>              output) override
{


    // zero until inputDim-outDim
    for(unsigned int j=0; j<int(this->inputDim - this->outputDim); ++j)
        output(j) = 0.0;

    StridedMatrix<const double, MemorySpace> tailOut = Kokkos::subview(
        output, std::make_pair(int(this->inputDim - this->outputDim), int(this->inputDim)), Kokkos::ALL());

    Kokkos::deep_copy(tailOutput, sens);

}

template<typename MemorySpace>
void IdentityMap<MemorySpace>::LogDeterminantCoeffGradImpl(StridedMatrix<const double, MemorySpace> const& pts, 
                                                             StridedMatrix<double, MemorySpace>              output)
{
    assert(false);
}

void IdentityMap<MemorySpace>::LogDeterminantInputGradImpl(StridedMatrix<const double, MemorySpace> const& pts, 
                                            StridedMatrix<double, MemorySpace>              output) override
{   
    // Add to logdet of full map
    for(unsigned int j=0; j<output.size(); ++j)
        output(j) = 0.0;
}

// Explicit template instantiation
template class mpart::IdentityMap<Kokkos::HostSpace>;
#if defined(KOKKOS_ENABLE_CUDA ) || defined(KOKKOS_ENABLE_SYCL)
    template class mpart::IdentityMap<Kokkos::DefaultExecutionSpace::memory_space>;
#endif