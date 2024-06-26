#ifndef MPART_TRIANGULARMAP_H
#define MPART_TRIANGULARMAP_H

#include "MParT/ConditionalMapBase.h"
#include "MParT/Utilities/Miscellaneous.h"

#include <Eigen/Core>

#include <Kokkos_Core.hpp>

#if defined(MPART_HAS_CEREAL)
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#endif 

namespace mpart{

/**
 @brief Provides a definition of block lower triangular transport maps.
 @details
This class defines a map \f$T:\mathbb{R}^N\rightarrow \mathbb{R}^M\f$ with the block triangular structure
\f[
T(x) = \left[\begin{array}{l} T_1(x_{1:N_1})\\ \vdots \\ T_k(x_{1:N_2})\\ \vdots T_K(x_{1:N}) \end{array} \right],
\f]
where each component \f$T_i(x_{1:N_i}):\mathbb{R}^{N_i}\rightarrow \mathbb{R}^{M_i}\f$ is a function depending on
the first \f$N_i\f$ inputs and returning \f$M_i\f$ outputs.  Note that this function must be invertible in the last
\f$M_i\f$ input arguments.  For example, it must be possible to solve \f$T_i(x_1:{N_i-M_i}, x_{N_i-M_i:N_i}) = r\f$
for \f$x_{N_i-M_i:N_i}\f$ given a vector \f$r\in\mathbb{R}^{M_i}\f$ and previous components \f$x_1:{N_i-M_i}\f$.

This block triangular form is analogous to a block triangular matrix where each \f$M_i\timesM_i\f$ diagonal block
is positive definite.

 */
template<typename MemorySpace>
class TriangularMap : public ConditionalMapBase<MemorySpace>{

public:

    /** @brief Construct a block triangular map from a collection of other ConditionalMapBase objects.

    @param components A vector of ConditionalMapBase objects defining each \f$T_k\f$ in the block triangular map.
                      To maintain the correct block structure, the dimensions of the components must satisfy \f$N_k = N_{k-1}+M_{k}\f$.
    @param moveCoeffs A boolean to choose whether to keep the coefficients from the maps in `components` or make new ones. If true,
                      the new object takes ownership of all the coefficients vectors within all the maps in `components` (changing the coefficients
                      in the new map will then change the coefficients in the original maps). If false, no coefficients are copied or created.
    */
    TriangularMap(std::vector<std::shared_ptr<ConditionalMapBase<MemorySpace>>> const& components, bool moveCoeffs=false);

    virtual ~TriangularMap() = default;

    /** @brief Sets the coefficients for all components of the map.

    @details This function will copy the provided coeffs vectors into the savedCoeffs object in the TriangularMap class.   To avoid
    duplicating the coefficients, the savedCoeffs member variable for each component will then be set to a subview of this vector.
    @param coeffs A vector containing coefficients for all components.  If component \f$k\f$ is defined by \f$C_k\f$ coefficients,
                  then this vector should have length \f$\sum_{k=1}^K C_i\f$ and the coefficients for component \f$k\f$ should
                  start at index \f$\sum_{j=1}^{k-1} C_j\f$.
    */
    using ParameterizedFunctionBase<MemorySpace>::SetCoeffs;
    void SetCoeffs(Kokkos::View<const double*, MemorySpace> coeffs) override;
    void WrapCoeffs(Kokkos::View<double*, MemorySpace> coeffs) override;

    virtual std::shared_ptr<ConditionalMapBase<MemorySpace>> GetComponent(unsigned int i){ return comps_.at(i);}

    /** @brief Computes the log determinant of the Jacobian matrix of this map.

    @details
    @param pts The points where the jacobian should be evaluated.  Should have \f$N\f$ rows.  Each column contains a single point.
    @param output A vector with length equal to the number of columns in pts containing the log determinant of the Jacobian.  This
                  vector should be correctly allocated and sized before calling this function.
    */
    void LogDeterminantImpl(StridedMatrix<const double, MemorySpace> const& pts,
                            StridedVector<double, MemorySpace>              output) override;


    /** @brief Evaluates the map.

    @details
    @param pts The points where the map should be evaluated.  Should have \f$N\f$ rows.  Each column contains a single point.
    @param output A matrix with \f$M\f$ rows to store the map output.  Should have the same number of columns as pts and be
                  allocated before calling this function.
    */
    void EvaluateImpl(StridedMatrix<const double, MemorySpace> const& pts,
                      StridedMatrix<double, MemorySpace>              output) override;

    void GradientImpl(StridedMatrix<const double, MemorySpace> const& pts,
                      StridedMatrix<const double, MemorySpace> const& sens,
                      StridedMatrix<double, MemorySpace>              output) override;

    /** @brief Evaluates the map inverse.

    @details To understand this function, consider splitting the map input \f$x_{1:N}\f$ into two parts so that \f$x_{1:N} = [x_{1:N-M},x_{N-M+1:M}]\f$.  Note that the
    second part of this split \f$x_{N-M+1:M}\f$ has the same dimension as the map output.   With this split, the map becomes
    \f$T(x_{1:N-M},x_{N-M+1:M})=r_{1:M}\f$.  Given \f$x_{1:N-M}\f$ and \f$r_{1:M}\f$, the `InverseImpl` function solves for the value
    of \f$x_{N-M+1:M}\f$ that satisfies \f$T(x_{1:N-M},x_{N-M+1:M})=r_{1:M}\f$.   Our shorthand for this will be
    \f$x_{N-M+1:M} = T^{-1}(r_{1:M}; x_{1:N-M})\f$.

    @param x1 The points \f$x_{1:N-M}\f$ where the map inverse should be evaluated.  Each column contains a single point.
    @param r The map output \f$r_{1:M}\f$ to invert.
    @param output A matrix with \f$M\f$ rows to store the computed map inverse \f$x_{N-M+1:M}\f$.
    */
    void InverseImpl(StridedMatrix<const double, MemorySpace> const& x1,
                     StridedMatrix<const double, MemorySpace> const& r,
                     StridedMatrix<double, MemorySpace>              output) override;


    virtual void InverseInplace(StridedMatrix<double, MemorySpace>              x1,
                                StridedMatrix<const double, MemorySpace> const& r);


    void CoeffGradImpl(StridedMatrix<const double, MemorySpace> const& pts,
                       StridedMatrix<const double, MemorySpace> const& sens,
                       StridedMatrix<double, MemorySpace>              output) override;


    void LogDeterminantCoeffGradImpl(StridedMatrix<const double, MemorySpace> const& pts,
                                             StridedMatrix<double, MemorySpace>              output) override;

    void LogDeterminantInputGradImpl(StridedMatrix<const double, MemorySpace> const& pts,
                                             StridedMatrix<double, MemorySpace>              output) override;

    std::vector<unsigned int> DiagonalCoeffIndices() const;
#if defined(MPART_HAS_CEREAL)
    template<class Archive>
    void save(Archive & ar) const
    {
        ar(cereal::base_class<ConditionalMapBase<MemorySpace>>(this), comps_);

        bool moveCoeffs = (this->savedCoeffs.is_allocated() && (this->savedCoeffs.size()>0));
        ar(comps_, moveCoeffs);
    }

    template <class Archive>
    static void load_and_construct( Archive & ar, cereal::construct<TriangularMap<MemorySpace>> & construct )
    {      
        std::vector<std::shared_ptr<ConditionalMapBase<MemorySpace>>> comps;
        ar(comps);
        
        bool moveCoeffs;
        ar(moveCoeffs);

        construct( comps , moveCoeffs);
    }

#endif // MPART_HAS_CEREAL

private:

    std::vector<std::shared_ptr<ConditionalMapBase<MemorySpace>>> comps_;


}; // class TriangularMap

}


#if defined(MPART_HAS_CEREAL)
CEREAL_FORCE_DYNAMIC_INIT(mpart_triangularmap)
#endif

#endif
