// @HEADER
// *****************************************************************************
//                           Stokhos Package
//
// Copyright 2009 NTESS and the Stokhos contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#ifndef STOKHOS_MONOMIAL_PROJ_GRAM_SCHMIDT_PCE_BASIS2_HPP
#define STOKHOS_MONOMIAL_PROJ_GRAM_SCHMIDT_PCE_BASIS2_HPP

#include "Teuchos_RCP.hpp"
#include "Teuchos_Array.hpp"
#include "Teuchos_BLAS.hpp"
#include "Teuchos_SerialDenseMatrix.hpp"
#include "Teuchos_SerialDenseVector.hpp"
#include "Teuchos_ParameterList.hpp"

#include "Stokhos_ReducedPCEBasis.hpp"
#include "Stokhos_OrthogPolyApprox.hpp"
#include "Stokhos_Quadrature.hpp"
#include "Stokhos_ProductBasisUtils.hpp"

namespace Stokhos {

  /*! 
   * \brief Generate a basis from a given set of PCE expansions that is 
   * orthogonal with respect to the product measure induced by these expansions.
   */
  /*!
   * Given the PCE expansions, first build a non-orthogonal monomial basis.  
   * Orthogonalize this basis using Gram-Schmidt, then build a quadrature rule
   * using the simplex method.
   */
  template <typename ordinal_type, typename value_type>
  class MonomialProjGramSchmidtPCEBasis2 : 
    public ReducedPCEBasis<ordinal_type,value_type> {
  public:

    //! Constructor
    /*!
     * \param p order of the basis
     * \param pce polynomial chaos expansions defining new measure
     * \param quad quadrature data for basis defining pce
     * \param Cijk sparse triple product tensor for basis defining pce
     * \param sparse_tol tolerance for dropping terms in sparse tensors
     */
    MonomialProjGramSchmidtPCEBasis2(
     ordinal_type p,
     const Teuchos::Array< Stokhos::OrthogPolyApprox<ordinal_type, value_type> >& pce,
     const Teuchos::RCP<const Stokhos::Quadrature<ordinal_type, value_type> >& quad,
     const Teuchos::ParameterList& params = Teuchos::ParameterList());

    //! Destructor
    virtual ~MonomialProjGramSchmidtPCEBasis2();

    //! \name Implementation of Stokhos::OrthogPolyBasis methods
    //@{

    //! Return order of basis
    ordinal_type order() const;

    //! Return dimension of basis
    ordinal_type dimension() const;

    //! Return total size of basis
    virtual ordinal_type size() const;

    //! Return array storing norm-squared of each basis polynomial
    /*!
     * Entry \f$l\f$ of returned array is given by \f$\langle\Psi_l^2\rangle\f$
     * for \f$l=0,\dots,P\f$ where \f$P\f$ is size()-1.
     */
    virtual const Teuchos::Array<value_type>& norm_squared() const;

    //! Return norm squared of basis polynomial \c i.
    virtual const value_type& norm_squared(ordinal_type i) const;

    //! Compute triple product tensor
    /*!
     * The \f$(i,j,k)\f$ entry of the tensor \f$C_{ijk}\f$ is given by
     * \f$C_{ijk} = \langle\Psi_i\Psi_j\Psi_k\rangle\f$ where \f$\Psi_l\f$
     * represents basis polynomial \f$l\f$ and \f$i,j,k=0,\dots,P\f$ where
     * \f$P\f$ is size()-1.
     */
    virtual 
    Teuchos::RCP< Stokhos::Sparse3Tensor<ordinal_type, value_type> > 
    computeTripleProductTensor() const;

    //! Compute linear triple product tensor where k = 0,1,..,d
    virtual 
    Teuchos::RCP< Stokhos::Sparse3Tensor<ordinal_type, value_type> > 
    computeLinearTripleProductTensor() const;

    //! Evaluate basis polynomial \c i at zero
    virtual value_type evaluateZero(ordinal_type i) const;

    //! Evaluate basis polynomials at given point \c point
    /*!
     * Size of returned array is given by size(), and coefficients are
     * ordered from order 0 up to size size()-1.
     */
    virtual void evaluateBases(
      const Teuchos::ArrayView<const value_type>& point,
      Teuchos::Array<value_type>& basis_vals) const;

    //! Return string name of basis
    virtual const std::string& getName() const;

    //! Print basis to stream \c os
    virtual void print(std::ostream& os) const;

    //@}

    //! \name Implementation of Stokhos::ReducedPCEBasis methods
    //@{

    //! Transform coefficients to original basis from this basis
    virtual void 
    transformToOriginalBasis(const value_type *in, 
			     value_type *out,
			     ordinal_type ncol = 1, 
			     bool transpose = false) const;

    //! Transform coefficients from original basis to this basis
    virtual void 
    transformFromOriginalBasis(const value_type *in, 
			       value_type *out,
			       ordinal_type ncol = 1, 
			       bool transpose = false) const;

    //! Get reduced quadrature object
    virtual Teuchos::RCP<const Stokhos::Quadrature<ordinal_type, value_type> >
    getReducedQuadrature() const;

    //@}

  protected:

    //! Build the reduced basis, parameterized by total order \c max_p
    /*!
     * Returns resulting size of reduced basis
     */
    ordinal_type 
    buildQ(
      ordinal_type max_p, 
      value_type threshold,
      const Teuchos::Array< Stokhos::OrthogPolyApprox<ordinal_type, value_type> >& pce,
      const Teuchos::RCP<const Stokhos::Quadrature<ordinal_type, value_type> >& quad,
      Teuchos::Array< Stokhos::MultiIndex<ordinal_type> >& terms_,
      Teuchos::Array<ordinal_type>& num_terms_,
      Teuchos::SerialDenseMatrix<ordinal_type,value_type>& Qp_, 
      Teuchos::SerialDenseMatrix<ordinal_type,value_type>& A_,
      Teuchos::SerialDenseMatrix<ordinal_type,value_type>& F_);

  private:

    // Prohibit copying
    MonomialProjGramSchmidtPCEBasis2(const MonomialProjGramSchmidtPCEBasis2&);

    // Prohibit Assignment
    MonomialProjGramSchmidtPCEBasis2& operator=(const MonomialProjGramSchmidtPCEBasis2& b);
    
  protected:

    typedef Stokhos::CompletePolynomialBasisUtils<ordinal_type,value_type> CPBUtils;
    typedef Teuchos::SerialDenseVector<ordinal_type,value_type> SDV;
    typedef Teuchos::SerialDenseMatrix<ordinal_type,value_type> SDM;

    //! Name of basis
    std::string name;

    //! Algorithm parameters
    Teuchos::ParameterList params;

    //! Original pce basis
    Teuchos::RCP<const Stokhos::OrthogPolyBasis<ordinal_type,value_type> > pce_basis;

    //! Size of original pce basis
    ordinal_type pce_sz;
    
    //! Total order of basis
    ordinal_type p;

    //! Total dimension of basis
    ordinal_type d;

    //! Total size of basis
    ordinal_type sz;

    //! 2-D array of basis terms
    Teuchos::Array< Stokhos::MultiIndex<ordinal_type> > terms;

    //! Number of terms up to each order
    Teuchos::Array<ordinal_type> num_terms;

    //! Norms
    Teuchos::Array<value_type> norms;

    //! Values of transformed basis at quadrature points
    SDM Q;

    //! Coefficients of transformed basis in original basis
    SDM Qp;

    //! Reduced quadrature object
    Teuchos::RCP<const Stokhos::Quadrature<ordinal_type, value_type> > reduced_quad;

    //! Whether to print a bunch of stuff out
    bool verbose;

    //! Rank threshold
    value_type rank_threshold;

    //! Orthogonalization method
    std::string orthogonalization_method;

    Teuchos::BLAS<ordinal_type,value_type> blas;

  }; // class MonomialProjGramSchmidtPCEBasis2

} // Namespace Stokhos

// Include template definitions
#include "Stokhos_MonomialProjGramSchmidtPCEBasis2Imp.hpp"

#endif
