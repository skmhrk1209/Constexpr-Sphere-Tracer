#pragma once

#include <tuple>
#include <type_traits>

#include "tensor.hpp"

namespace coex::tensor {

// ================================================================
// lazy evaluation

template <typename Matrix1, typename Matrix2>
struct LazyMatMul : std::tuple<Matrix1, Matrix2> {
    template <MatrixShaped Matrix>
    constexpr auto operator%(const Matrix &matrix) const {
        return LazyMatMul<LazyMatMul<Matrix1, Matrix2>, Matrix>(*this, matrix);
    }

    template <VectorShaped Vector>
    constexpr auto operator%(const Vector &vector) const {
        return std::get<0>(*this) % (std::get<1>(*this) % vector);
    }
};

// ================================================================
// multiplication

template <MatrixShaped Matrix1, MatrixShaped Matrix2>
constexpr auto operator%(const Matrix1 &matrix_1, const Matrix2 &matrix_2)
    requires(dimension_v<Matrix1, 1> == dimension_v<Matrix2, 0>)
{
    return LazyMatMul<const Matrix1&, const Matrix2&>(matrix_1, matrix_2);
}

template <MatrixShaped Matrix, VectorShaped Vector>
constexpr auto operator%(const Matrix &matrix, const Vector &vector)
    requires(dimension_v<Matrix, 0> == dimension_v<Vector, 0>) && (dimension_v<Matrix, 1> == dimension_v<Vector, 0>)
{
    return [&]<auto... Is>(std::index_sequence<Is...>)->Vector { return {dot(matrix[Is], vector)...}; }
    (std::make_index_sequence<dimension_v<std::decay_t<decltype(matrix)>, 0>>{});
}

}  // namespace coex::tensor
