#pragma once

#include <complex>

#include "camera.hpp"
#include "common.hpp"
#include "math.hpp"
#include "random.hpp"
#include "tensor.hpp"
#include "utilities.hpp"

namespace rendex::reflection {

template <typename Scalar, template <typename, auto> typename Vector = rendex::tensor::Vector>
class Metal {
   public:
    constexpr Metal() = default;

    constexpr Metal(const Vector<std::complex<Scalar>, 3> &refractive_index, auto fuzziness)
        : m_refractive_index(refractive_index), m_fuzziness(fuzziness) {}

    constexpr Metal(Vector<std::complex<Scalar>, 3> &&refractive_index, auto fuzziness)
        : m_refractive_index(std::move(refractive_index)), m_fuzziness(fuzziness) {}

    constexpr auto &refractive_index() { return m_refractive_index; }
    constexpr const auto &refractive_index() const { return m_refractive_index; }

    constexpr auto &fuzziness() { return m_fuzziness; }
    constexpr const auto &fuzziness() const { return m_fuzziness; }

    constexpr auto operator()(const auto &ray, const auto &normal, auto &generator) const {
        auto complex_reflectance = rendex::math::square((1.0 - m_refractive_index) / (1.0 + m_refractive_index));
        auto specular_reflectance = [&]<auto... Is>(std::index_sequence<Is...>) {
            return Vector<Scalar, 3>{rendex::math::abs(complex_reflectance[Is])...};
        }
        (std::make_index_sequence<rendex::tensor::dimension_v<Vector<std::complex<Scalar>, 3>, 0>>{});
        auto cosine = -rendex::tensor::dot(ray.direction(), normal);
        auto fresnel_reflectance = schlick_approx(specular_reflectance, cosine);
        auto reflected_position = ray.position() + 1e-6 * normal;
        auto reflected_direction = reflect(ray.direction(), normal);
        auto random_direction = rendex::random::uniform_in_unit_sphere<Scalar, Vector>(generator) * m_fuzziness;
        auto fuzzy_reflected_direction = rendex::tensor::normalized(reflected_direction + random_direction);
        rendex::camera::Ray<Scalar, Vector> reflected_ray(reflected_position, fuzzy_reflected_direction);
        return std::make_tuple(std::move(reflected_ray), std::move(fresnel_reflectance));
    }

   private:
    Vector<std::complex<Scalar>, 3> m_refractive_index;
    Scalar m_fuzziness;
};

}  // namespace rendex::reflection
