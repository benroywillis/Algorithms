/* image_file_format.h
 *
 * Author: Fabian Meyer
 * Created On: 04 Mar 2019
 */

#ifndef NVISION_IMAGE_FILE_FORMAT_H_
#define NVISION_IMAGE_FILE_FORMAT_H_

#include <istream>
#include <ostream>

namespace nvision
{
    template<typename FileFormat>
    class ImageReader { };

    template<typename FileFormat>
    class ImageWriter { };
}

#endif
