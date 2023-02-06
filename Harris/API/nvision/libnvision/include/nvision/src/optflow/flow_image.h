/* flow_image.h
 *
 * Author: Fabian Meyer
 * Created On: 16 May 2019
 */

#ifndef NVISION_FLOW_IMAGE_H_
#define NVISION_FLOW_IMAGE_H_

#include "nvision/src/core/pixel.h"
#include "nvision/src/optflow/flow_field.h"

namespace nvision
{
    template<typename ColorSpace, typename Scalar>
    auto imflow(const FlowField<Scalar> &flow)
    {
        const auto norms =flow.unaryExpr([](const auto &vec) { return vec.squaredNorm(); });
        const Scalar maxMagnitude =  std::sqrt(norms.maxCoeff());
        return flow.unaryExpr([maxMagnitude](const FlowVector<Scalar>& vec)
        {
            const auto magnitude = vec.norm() / maxMagnitude;
            const auto angle = nvision::angle::normalize(std::atan2(vec.y(), vec.x())) / (2 * nvision::pi<Scalar>());

            Pixel<HSV> pixel(angle, magnitude, 0.9f);
            return pixel.template convert<ColorSpace>();
        });
    }
}

#endif
