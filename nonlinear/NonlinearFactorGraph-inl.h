/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    NonlinearFactorGraph-inl.h
 * @brief   Factor Graph Consisting of non-linear factors
 * @author  Frank Dellaert
 * @author  Carlos Nieto
 * @author  Christian Potthast
 */

#pragma once

#include <boost/foreach.hpp>
#include <gtsam/linear/GaussianFactorGraph.h>
#include <gtsam/nonlinear/NonlinearFactorGraph.h>
#include <gtsam/inference/FactorGraph-inl.h>
#include <gtsam/inference/inference-inl.h>

#define INSTANTIATE_NONLINEAR_FACTOR_GRAPH(C) \
  INSTANTIATE_FACTOR_GRAPH(NonlinearFactor<C>); \
  template class NonlinearFactorGraph<C>;

using namespace std;

namespace gtsam {

/* ************************************************************************* */
template<class Values>
void NonlinearFactorGraph<Values>::print(const std::string& str) const {
  Base::print(str);
}

	/* ************************************************************************* */
	template<class Values>
	Vector NonlinearFactorGraph<Values>::unwhitenedError(const Values& c) const {
		list<Vector> errors;
		BOOST_FOREACH(const sharedFactor& factor, this->factors_)
			errors.push_back(factor->unwhitenedError(c));
		return concatVectors(errors);
	}

	/* ************************************************************************* */
	template<class Values>
	double NonlinearFactorGraph<Values>::error(const Values& c) const {
		double total_error = 0.;
		// iterate over all the factors_ to accumulate the log probabilities
		BOOST_FOREACH(const sharedFactor& factor, this->factors_)
			total_error += factor->error(c);
		return total_error;
	}

  /* ************************************************************************* */
  template<class Values>
	Ordering::shared_ptr NonlinearFactorGraph<Values>::orderingCOLAMD(const Values& config) const {

    // Create symbolic graph and initial (iterator) ordering
	  FactorGraph<Factor>::shared_ptr symbolic;
	  Ordering::shared_ptr ordering;
	  boost::tie(symbolic,ordering) = this->symbolic(config);

	  // Compute the VariableIndex (column-wise index)
	  VariableIndex<> variableIndex(*symbolic);

	  // Compute a fill-reducing ordering with COLAMD
	  Permutation::shared_ptr colamdPerm(Inference::PermutationCOLAMD(variableIndex));

	  // Permute the Ordering and VariableIndex with the COLAMD ordering
	  ordering->permuteWithInverse(*colamdPerm->inverse());
//	  variableIndex.permute(*colamdPerm);
	  // SL-FIX: fix permutation

	  // Return the Ordering and VariableIndex to be re-used during linearization
	  // and elimination
	  return ordering;
	}

  /* ************************************************************************* */
  template<class Values>
  SymbolicFactorGraph::shared_ptr NonlinearFactorGraph<Values>::symbolic(
      const Values& config, const Ordering& ordering) const {
    // Generate the symbolic factor graph
    SymbolicFactorGraph::shared_ptr symbolicfg(new FactorGraph<Factor>);
    symbolicfg->reserve(this->size());
    BOOST_FOREACH(const sharedFactor& factor, this->factors_) {
      symbolicfg->push_back(factor->symbolic(ordering));
    }
    return symbolicfg;
  }

  /* ************************************************************************* */
	template<class Values>
	pair<SymbolicFactorGraph::shared_ptr, Ordering::shared_ptr>
	NonlinearFactorGraph<Values>::symbolic(const Values& config) const {
	  // Generate an initial key ordering in iterator order
    Ordering::shared_ptr ordering(config.orderingArbitrary());
    return make_pair(symbolic(config, *ordering), ordering);
	}

	/* ************************************************************************* */
	template<class Values>
	boost::shared_ptr<GaussianFactorGraph> NonlinearFactorGraph<Values>::linearize(
			const Values& config, const Ordering& ordering) const {

		// create an empty linear FG
		GaussianFactorGraph::shared_ptr linearFG(new GaussianFactorGraph);
		linearFG->reserve(this->size());

		// linearize all factors
		BOOST_FOREACH(const sharedFactor& factor, this->factors_) {
			boost::shared_ptr<GaussianFactor> lf = factor->linearize(config, ordering);
			if (lf)	linearFG->push_back(lf);
		}

		return linearFG;
	}

	/* ************************************************************************* */

} // namespace gtsam
