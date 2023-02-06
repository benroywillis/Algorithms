/* camera_calibration.h
 *
 * Author: Fabian Meyer
 * Created On: 10 Sep 2019
 */

#ifndef NVISION_CAMERA_CALIBRATION_H_
#define NVISION_CAMERA_CALIBRATION_H_

#include "nvision/projective/direct_linear_transformation.h"
#include "nvision/projective/homography_decomposition.h"

namespace nvision
{
    template<typename Scalar>
    class CameraCalibration
    {
    public:
        typedef Eigen::Matrix<Scalar, 3, 3> Matrix3;
        typedef Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> Matrix;
    private:
        Index ransac_;
    public:

        CameraCalibration()
        {}

        Matrix3 operator()(const std::vector<Matrix> &points) const
        {
            DirectLinearTransformation<Scalar> dlt;
            HomographyDecomposition<Scalar> decomp;

            std::vector<Matrix3> homographies(points.size() - 1);

            for(size_t i = 0; i < homographies.size(); ++i)
                homographies[i] = dlt(points[0], points[i+1]);

            return decomp(homographies);
        }
    };
}

#endif
