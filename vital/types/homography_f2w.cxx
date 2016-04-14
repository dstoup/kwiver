/*ckwg +29
 * Copyright 2015 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
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

/**
 * \file
 * \brief Frame to World Homography implementation
 */

#include "homography_f2w.h"

namespace kwiver {
namespace vital {


/// Construct an identity homography for the given frame
f2w_homography
::f2w_homography( frame_id_t const frame_id )
  : h_( homography_sptr( new homography_<double>() ) ),
    frame_id_( frame_id )
{
}

/// Construct given an existing homography
f2w_homography
::f2w_homography( homography_sptr const &h, frame_id_t const frame_id )
  : h_( h->clone() ),
    frame_id_( frame_id )
{
}

/// Copy Constructor
f2w_homography
::f2w_homography( f2w_homography const &h )
  : h_( h.h_->clone() ),
    frame_id_( h.frame_id_ )
{
}

/// Get the homography transformation
homography_sptr
f2w_homography
::homography() const
{
  return this->h_;
}

/// Get the frame identifier
frame_id_t
f2w_homography
::frame_id() const
{
  return this->frame_id_;
}


} } // end vital namespace
