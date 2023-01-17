#include <map>
#include "MParT/TrainMap.h"

using namespace mpart;

// Vectors for keeping track of NLopt success/failure codes.
const std::vector<std::string> MPART_NLOPT_SUCCESS_CODES {
    "UNDEFINED OPTIMIZATION RESULT",
    "Generic success",
    "stopval reached",
    "xtol reached",
    "xtol reached",
    "maxeval reached",
    "maxtime reached"
};

const std::vector<std::string> MPART_NLOPT_FAILURE_CODES {
    "UNDEFINED OPTIMIZATION RESULT",
    "generic failure",
    "invalid arguments",
    "out of memory",
    "roundoff error limited progress",
    "forced termination"
};

nlopt::opt SetupOptimization(unsigned int dim, TrainOptions options) {
    nlopt::opt opt(options.opt_alg.c_str(), dim);

    // Set all the optimization options for nlopt here
    opt.set_stopval(options.opt_stopval);
    opt.set_xtol_rel(options.opt_xtol_rel);
    opt.set_ftol_rel(options.opt_ftol_rel);
    opt.set_ftol_abs(options.opt_ftol_abs);
    opt.set_maxeval(options.opt_maxeval);
    opt.set_maxtime(options.opt_maxtime);

    // Print all the optimization options, if verbose
    if(options.verbose){
        std::cout << "Optimization Settings:\n";
        std::cout << "Algorithm: " << opt.get_algorithm_name() << "\n";
        std::cout << "Optimization dimension: " << opt.get_dimension() << "\n";
        std::cout << "Optimization stopval: " << opt.get_stopval() << "\n";
        std::cout << "Max f evaluations: " << opt.get_maxeval() << "\n";
        std::cout << "Maximum time: " << opt.get_maxtime() << "\n";
        std::cout << "Relative x Tolerance: " << opt.get_xtol_rel() << "\n";
        std::cout << "Relative f Tolerance: " << opt.get_ftol_rel() << "\n";
        std::cout << "Absolute f Tolerance: " << opt.get_ftol_abs() << "\n";
    }
    return opt;
}

template<typename ObjectiveType>
void mpart::TrainMap(std::shared_ptr<ConditionalMapBase<Kokkos::HostSpace>> map, ObjectiveType &objective, TrainOptions options) {
    if(map->Coeffs().extent(0) == 0) {
        std::cout << "TrainMap: Initializing map coeffs to 1." << std::endl;
        Kokkos::View<double*, Kokkos::HostSpace> coeffs ("Default coeffs", map->numCoeffs);
        Kokkos::parallel_for("Setting default coeff val", map->numCoeffs, KOKKOS_LAMBDA(const unsigned int i){
            coeffs(i) = 1.;
        });
        map->SetCoeffs(coeffs);
    }
    nlopt::opt opt = SetupOptimization(map->numCoeffs, options);

    // Since objective is (rightfully) separate from the map, we use std::bind to create a functor
    // from objective::operator() that keeps the map argument held.
    std::function<double(unsigned, const double*, double*)> functor = std::bind(objective, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, map);
    opt.set_min_objective(functor);

    // Get the initial guess at the coefficients
    std::vector<double> mapCoeffsStd = KokkosToStd(map->Coeffs());

    // Optimize the map coefficients using NLopt
    double error;
    nlopt::result res = opt.optimize(mapCoeffsStd, error);

    // Set the coefficients using SetCoeffs
    Kokkos::View<double*, Kokkos::HostSpace> mapCoeffsView = VecToKokkos<double,Kokkos::HostSpace>(mapCoeffsStd);
    map->SetCoeffs(mapCoeffsView);

    // Print a warning if something goes wrong with NLOpt
    if(res < 0) {
        std::cerr << "WARNING: Optimization failed: " << MPART_NLOPT_FAILURE_CODES[-res] << std::endl;
    }

    // Print results of optimization if verbose
    if(options.verbose){
        if(res >= 0){
            std::cout << "Optimization result: " << MPART_NLOPT_SUCCESS_CODES[res] << "\n";
        }
        std::cout << "Optimization error: " << error << "\n";
        std::cout << "Optimization evaluations: " << opt.get_numevals() << std::endl;
    }
}

// Explicitly instantiate the function for HostSpace-- it doesn't work for device, since NLopt is CPU-based.
template void mpart::TrainMap(std::shared_ptr<ConditionalMapBase<Kokkos::HostSpace>> map, KLObjective<Kokkos::HostSpace> &objective, TrainOptions options);

