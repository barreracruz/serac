// Copyright (c) 2019-2022, Lawrence Livermore National Security, LLC and
// other Serac Project Developers. See the top-level LICENSE file for
// details.
//
// SPDX-License-Identifier: (BSD-3-Clause)

/**
 * @file segment_H1.inl
 *
 * @brief Specialization of finite_element for H1 on segment geometry
 */

// this specialization defines shape functions (and their gradients) that
// interpolate at Gauss-Lobatto nodes for the appropriate polynomial order
//
// note: mfem assumes the parent element domain is [0,1]
// for additional information on the finite_element concept requirements, see finite_element.hpp
/// @cond
template <int p, int c>
struct finite_element<Geometry::Segment, H1<p, c> > {
  static constexpr auto geometry   = Geometry::Segment;
  static constexpr auto family     = Family::H1;
  static constexpr int  components = c;
  static constexpr int  dim        = 1;
  static constexpr int  ndof       = (p + 1);

  using residual_type =
      typename std::conditional<components == 1, tensor<double, ndof>, tensor<double, ndof, components> >::type;

  SERAC_HOST_DEVICE static constexpr tensor<double, ndof> shape_functions(double xi)
  {
    return GaussLobattoInterpolation<ndof>(xi);
  }

  SERAC_HOST_DEVICE static constexpr tensor<double, ndof> shape_function_gradients(double xi)
  {
    return GaussLobattoInterpolationDerivative<ndof>(xi);
  }
};
/// @endcond
