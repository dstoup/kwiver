/*ckwg +29
 * Copyright 2016 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef KWIVER_VITAL_TYPES_BOUNDING_BOX_H
#define KWIVER_VITAL_TYPES_BOUNDING_BOX_H

#include <vital/types/vector.h>
#include <Eigen/Geometry>

namespace kwiver {
namespace vital {

// ----------------------------------------------------------------
/**
 * @brief Coordinate aligned bounding box.
 *
 * This class represents a coordinate aligned box. The coordinate
 * system places the origin in the upper left.
 */
template < typename T >
class bounding_box
{
public:
  typedef Eigen::Matrix< T, 2, 1 > vector_type;

  bounding_box()
  { }

  bounding_box( vector_type const& upper_left,
                vector_type const& lower_right )
    : m_bbox( upper_left, lower_right )
  { }

  bounding_box( vector_type const& upper_left,
                T const& width, T const& height )
  {
    vector_type lr( upper_left );
    lr.x() += width;
    lr.y() += height;
    m_bbox =  Eigen::AlignedBox< T, 2 >( upper_left, lr );
  }

  vector_type center() const { return m_bbox.center(); }
  vector_type upper_left() const { return m_bbox.min(); }
  vector_type lower_right() const { return m_bbox.max(); }

  T width() const { return m_bbox.sizes()[0]; }
  T height() const { return m_bbox.sizes()[1]; }

  double area() const { return m_bbox.volume(); }

  Eigen::AlignedBox< T, 2 >& get_eabb()  { return m_bbox; }
  Eigen::AlignedBox< T, 2 > get_eabb() const  { return m_bbox; }

protected:
  // Note that this class is implemented using Eigen types.
  // There is no guarantee of this in the future.
  bounding_box( Eigen::AlignedBox< T, 2 > const& b )
    : m_bbox( b )
  { }

  Eigen::AlignedBox< T, 2 > m_bbox;
};

// Define for common types.
typedef bounding_box< int > bounding_box_i;
typedef bounding_box< double > bounding_box_d;


/**
 * @brief Translate a box by (x,y) offset.
 *
 * This operator translates a bounding_box by the specified amount.
 *
 * @param[in,out] bbox Box to translate
 * @param[in] pt X and Y offsets to use for translation
 *
 * @return The specified box is updated with the new coordinates.
 */
  template < typename T >
  bounding_box<T> & translate( bounding_box<T>& bbox,
                               Eigen::Matrix< T, 2, 1 > const& pt )
{
  bbox.get_eabb().translate( pt );
  return bbox;
}


/**
 * @brief Determine intersection of two boxes.
 *
 * This operator calculates the intersection of two
 * bounding_boxes. The rectangular intersection is returned as a new
 * bounding box.
 *
 * @param one The first bounding box for finding intersection
 * @param other The other bounding box for finding intersection.
 *
 * @return A new bounding_box specifying the intersection if the two
 * parameters.
 */
template<typename T>
bounding_box<T> intersection( bounding_box<T> const& one,
                              bounding_box<T> const& other )
{
  return bounding_box<T>( one.get_eabb().intersection( other.get_eabb() ) );
}

} }   // end namespace kwiver

#endif /* KWIVER_VITAL_TYPES_BOUNDING_BOX_H */
