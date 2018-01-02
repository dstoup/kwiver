/*ckwg +29
 * Copyright 2015-2017 by Kitware, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS [yas] elisp error!AS IS''
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

#include "detect_loops_process.h"

#include <vital/vital_types.h>
#include <vital/types/timestamp.h>
#include <vital/types/timestamp_config.h>
#include <vital/types/image_container.h>
#include <vital/types/feature_set.h>

#include <vital/algo/detect_loops.h>

#include <kwiver_type_traits.h>

#include <sprokit/pipeline/process_exception.h>

namespace algo = kwiver::vital::algo;

namespace kwiver
{

  create_config_trait( detect_loops, std::string, "", "Algorithm configuration subblock." )

/**
 * \class track_features_process
 *
 * \brief track feature points in supplied images.
 *
 * \process This process generates a list of tracked features that
 * can be used to determine coordinate transforms between images. The
 * actual tracking is done by the selected \b track_features
 * algorithm implementation
 *
 * \iports
 *
 * \iport{timestamp} time stamp for incoming images.
 *
 * \iport{image} Input image to be processed.
 *
 *\iport{feature_set} Set of detected features from previous image.
 *
 * \oports
 *
 * \oport{feature_set} Set of detected features for input image.
 *
 * \configs
 *
 * \config{track_features} Name of the configuration subblock that selects
 * and configures the feature detector algorithm
 */

//----------------------------------------------------------------
// Private implementation class
class detect_loops_process::priv
{
public:
  priv();
  ~priv();

  // Configuration values

  // There are many config items for the tracking and stabilization that go directly to
  // the algo.

  algo::detect_loops_sptr m_loop_detector;

  //static port_t const port_input;
  //static port_type_t const type_custom_feedback;

  bool first;

}; // end priv class

//sprokit::process::port_t const track_features_process::priv::port_input = port_t("prev_tracks_port");
//sprokit::process::port_type_t const track_features_process::priv::type_custom_feedback = port_type_t("kwiver:feature_track_set");

// ================================================================

  detect_loops_process
::detect_loops_process( kwiver::vital::config_block_sptr const& config )
  : process( config ),
    d( new detect_loops_process::priv )
{
  // Attach our logger name to process logger
  attach_logger( kwiver::vital::get_logger( name() ) ); // could use a better approach

  make_ports();
  make_config();
}


  detect_loops_process
::~detect_loops_process()
{
}


// ----------------------------------------------------------------
void detect_loops_process
::_configure()
{
  scoped_configure_instrumentation();


  // Get our process config
  kwiver::vital::config_block_sptr algo_config = get_config();

  // Instantiate the configured algorithm
  algo::detect_loops::set_nested_algo_configuration( "detect_loops", algo_config, d->m_loop_detector );
  if ( ! d->m_loop_detector )
  {
    throw sprokit::invalid_configuration_exception( name(), "Unable to create track_features" );
  }

  algo::detect_loops::get_nested_algo_configuration("detect_loops", algo_config, d->m_loop_detector);

  //// Check config so it will give run-time diagnostic if any config problems are found
  if ( ! algo::detect_loops::check_nested_algo_configuration( "detect_loops", algo_config ) )
  {
    throw sprokit::invalid_configuration_exception( name(), "Configuration check failed." );
  }

}


// ----------------------------------------------------------------
void
detect_loops_process
::_step()
{
  // timestamp
  kwiver::vital::timestamp frame_time = grab_from_port_using_trait( timestamp );

  vital::feature_track_set_sptr prev_tracks = grab_from_port_using_trait(feature_track_set); //NEED TO FIX THIS    

  kwiver::vital::feature_track_set_sptr curr_feat;

  {
    scoped_step_instrumentation();

    LOG_DEBUG( logger(), "Processing frame " << frame_time );

    // detect features on the current frame
    curr_feat = d->m_loop_detector->detect(prev_tracks, frame_time.get_frame());
  }

  // return by value
  push_to_port_using_trait(feature_track_set, curr_feat );
}


// ----------------------------------------------------------------
void detect_loops_process
::make_ports()
{
  // Set up for required ports
  sprokit::process::port_flags_t optional;
  sprokit::process::port_flags_t required;
  sprokit::process::port_flags_t input_nodep;
  required.insert( flag_required );
  input_nodep.insert(flag_input_nodep );  

  // -- input --
  declare_input_port_using_trait( timestamp, required );

  declare_input_port_using_trait(feature_track_set, input_nodep);

  // -- output --
  declare_output_port_using_trait(feature_track_set, optional );
}


// ----------------------------------------------------------------
void detect_loops_process
::make_config()
{
  declare_config_using_trait( detect_loops );
}


// ================================================================
detect_loops_process::priv
::priv():first(true)
{
}


detect_loops_process::priv
::~priv()
{
}

} // end namespace
