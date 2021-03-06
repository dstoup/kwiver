#
# KWIVER processes
#
project( kwiver_processes )

set( sources
  register_processes.cxx

  compute_homography_process.cxx
  compute_stereo_depth_map_process.cxx
  detect_features_process.cxx
  detected_object_output_process.cxx
  detected_object_input_process.cxx
  detected_object_filter_process.cxx
  draw_detected_object_set_process.cxx
  draw_tracks_process.cxx
  extract_descriptors_process.cxx
  frame_list_process.cxx
  image_file_reader_process.cxx
  image_filter_process.cxx
  image_object_detector_process.cxx
  image_writer_process.cxx
  matcher_process.cxx
  read_descriptor_process.cxx
  refine_detections_process.cxx
  stabilize_image_process.cxx
  video_input_process.cxx
)

set( private_headers
  compute_homography_process.h
  compute_stereo_depth_map_process.h
  detect_features_process.h
  detected_object_output_process.h
  detected_object_input_process.h
  detected_object_filter_process.h
  draw_detected_object_set_process.h
  draw_tracks_process.h
  extract_descriptors_process.h
  frame_list_process.h
  image_file_reader_process.h
  image_filter_process.h
  image_object_detector_process.h
  image_writer_process.h
  matcher_process.h
  read_descriptor_process.h
  refine_detections_process.h
  stabilize_image_process.h
  video_input_process.h
)

kwiver_private_header_group( ${private_headers} )

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

kwiver_add_plugin( kwiver_processes
  SUBDIR           sprokit
  SOURCES        ${sources}
                 ${private_headers}
  PRIVATE          sprokit_pipeline
                   kwiver_algo_core
                   kwiversys
                   vital vital_vpm vital_logger vital_config
                 ${Boost_SYSTEM_LIBRARY}
                 ${Boost_FILESYSTEM_LIBRARY}
)
