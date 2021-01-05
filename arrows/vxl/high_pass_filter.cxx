// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "high_pass_filter.h"

#include <arrows/vxl/image_container.h>

#include <vil/vil_image_view.h>
#include <vil/vil_plane.h>
#include <vil/vil_transpose.h>
#include <vil/vil_math.h>

namespace kwiver {
namespace arrows {
namespace vxl {

// --------------------------------------------------------------------------------------
/// Private implementation class
class high_pass_filter::priv
{
public:
  priv( high_pass_filter* parent )
  : p( parent )
  {
  }

  ~priv()
  {
  }

  // Internal parameters/settings
  std::string mode = "box";
  size_t kernel_width = 7;
  size_t kernel_height = 7;
  bool treat_as_interlaced = false;
  bool output_net_only = false;
  high_pass_filter* p;

  // Perform box filtering
  template <typename PixType>
  vil_image_view< PixType >
  box_high_pass_filter( const vil_image_view< PixType >& grey_img )
  {
    // recreate the output image and create views of each plane
    vil_image_view< PixType > output;
    output.set_size( grey_img.ni(), grey_img.nj(), 3 );
    vil_image_view<PixType> filter_x = vil_plane(output, 0);
    vil_image_view<PixType> filter_y = vil_plane(output, 1);
    vil_image_view<PixType> filter_xy = vil_plane(output, 2);

    // compute the vertically smoothed image
    filter_x.set_size(grey_img.ni(), grey_img.nj(), 1);
    if( treat_as_interlaced )
    {
      // if interlaced, split the image into odd and even views
      // transpose all input and ouput images so that the horizontal
      // smoothing function produces vertical smoothing
      vil_image_view<PixType> im_even_t = vil_transpose( even_rows( grey_img ) );
      vil_image_view<PixType> im_odd_t = vil_transpose( odd_rows( grey_img ) );

      vil_image_view<PixType> smooth_even_t = vil_transpose( even_rows( filter_x ) );
      vil_image_view<PixType> smooth_odd_t = vil_transpose( odd_rows( filter_x ) );

      // use a half size odd kernel since images are half height
      int half_kernel_height = kernel_height / 2;
      if (half_kernel_height % 2 == 0)
      {
        ++half_kernel_height;
      }
      box_average_1d(im_even_t, smooth_even_t, half_kernel_height);
      box_average_1d(im_odd_t, smooth_odd_t, half_kernel_height);
    }
    else
    {
      // if not interlaced, transpose inputs and outputs and apply horizontal smoothing.
      vil_image_view<PixType> grey_img_t = vil_transpose(grey_img);
      vil_image_view<PixType> smooth_t = vil_transpose(filter_x);
      box_average_1d(grey_img_t, smooth_t, kernel_height);
    }

    // apply horizontal smoothing to the vertical smoothed image to get a 2D box filter
    box_average_1d(filter_x, filter_xy, kernel_width);

    // smooth the image horizontally and detect the vertical response
    box_average_1d(grey_img, filter_y, kernel_width);

    // Report the difference between the pixel value, and all of the smoothed responses
    vil_math_image_abs_difference( grey_img, filter_x, filter_x );
    vil_math_image_abs_difference( grey_img, filter_y, filter_y );
    vil_math_image_abs_difference( grey_img, filter_xy, filter_xy );

    return output;
  }

  // Given an input grayscale image, and a smoothed version of this greyscale
  // image, calculate the bidirectional filter response in either the vertical
  // or horizontal directions.
  template <typename PixType>
  void box_bidirectional_pass( const vil_image_view<PixType>& grey,
                               const vil_image_view<PixType>& avg,
                               vil_image_view<PixType>& output,
                               unsigned kernel_width,
                               bool is_horizontal )
  {
    const unsigned ni = grey.ni();
    const unsigned nj = grey.nj();

    output.fill( 0 );

    unsigned offset = ( kernel_width / 2 ) + 1;

    unsigned diff1 = 0, diff2 = 0;

    if( is_horizontal )
    {
      if( ni < 2 * offset + 1 )
      {
        return;
      }

      for( unsigned j = 0; j < nj; j++ )
      {
        for( unsigned i = offset; i < ni - offset; i++ )
        {
          const PixType& val = grey(i,j);
          const PixType& avg1 = avg(i-offset,j);
          const PixType& avg2 = avg(i+offset,j);

          diff1 = ( val > avg1 ? val - avg1 : avg1 - val );
          diff2 = ( val > avg2 ? val - avg2 : avg2 - val );

          output(i,j) = std::min( diff1, diff2 );
        }
      }
    }
    else
    {
      if( nj < 2 * offset + 1 )
      {
        return;
      }

      for( unsigned j = offset; j < nj - offset; j++ )
      {
        for( unsigned i = 0; i < ni; i++ )
        {
          const PixType& val = grey(i,j);
          const PixType& avg1 = avg(i,j+offset);
          const PixType& avg2 = avg(i,j-offset);

          diff1 = ( val > avg1 ? val - avg1 : avg1 - val );
          diff2 = ( val > avg2 ? val - avg2 : avg2 - val );

          output(i,j) = std::min( diff1, diff2 );
        }
      }
    }
  }

  // Perform bidirectional filtering
  template <typename PixType>
  vil_image_view<PixType> bidirection_box_filter( const vil_image_view<PixType>& grey_img )
  {
    // recreate the output image and create views of each plane
    vil_image_view< PixType > output;
    output.set_size( grey_img.ni(), grey_img.nj(), 3 );
    vil_image_view<PixType> filter_x = vil_plane(output, 1);
    vil_image_view<PixType> filter_y = vil_plane(output, 2);
    vil_image_view<PixType> filter_xy = vil_plane(output, 0);

    // comptue the vertically smoothed image
    filter_x.set_size(grey_img.ni(), grey_img.nj(), 1);
    if( treat_as_interlaced )
    {
      // if interlaced, split the image into odd and even views
      // transpose all input and ouput images so that the horizontal
      // smoothing function produces vertical smoothing
      vil_image_view<PixType> im_even_t = vil_transpose( even_rows( grey_img ) );
      vil_image_view<PixType> im_odd_t = vil_transpose( odd_rows( grey_img ) );

      vil_image_view<PixType> smooth_even_t = vil_transpose( even_rows( filter_x ) );
      vil_image_view<PixType> smooth_odd_t = vil_transpose( odd_rows( filter_x ) );

      // use a half size odd kernel since images are half height
      int half_kernel_height = kernel_height / 2;
      if (half_kernel_height % 2 == 0)
      {
        ++half_kernel_height;
      }
      box_average_1d(im_even_t, smooth_even_t, half_kernel_height);
      box_average_1d(im_odd_t, smooth_odd_t, half_kernel_height);
    }
    else
    {
      // if not interlaced, transpose inputs and outputs and apply horizontal smoothing.
      vil_image_view<PixType> grey_img_t = vil_transpose(grey_img);
      vil_image_view<PixType> smooth_t = vil_transpose(filter_x);
      box_average_1d(grey_img_t, smooth_t, kernel_height);
    }

    // smooth the image horizontally and detect the vertical response
    box_average_1d(grey_img, filter_y, kernel_width);

    // Report the difference between the pixel value, and all of the smoothed responses
    box_bidirectional_pass( grey_img, filter_x, filter_xy, kernel_width, true );
    box_bidirectional_pass( grey_img, filter_y, filter_x, kernel_height, false );
    vil_math_image_max( filter_xy, filter_x, filter_y );

    return output;
  }

  // Function which returns an image view of the even rows of an image
  template <typename PixType>
  inline vil_image_view<PixType> even_rows(const vil_image_view<PixType>& im)
  {
    return vil_image_view<PixType>( im.memory_chunk(), im.top_left_ptr(),
                                    im.ni(), (im.nj()+1)/2, im.nplanes(),
                                    im.istep(), im.jstep()*2, im.planestep() );
  }


  // Function which returns an image view of the odd rows of an image
  template <typename PixType>
  inline vil_image_view<PixType> odd_rows(const vil_image_view<PixType>& im)
  {
    return vil_image_view<PixType>( im.memory_chunk(), im.top_left_ptr()+im.jstep(),
                                    im.ni(), (im.nj())/2, im.nplanes(),
                                    im.istep(), im.jstep()*2, im.planestep() );
  }

  // Fast 1D (horizontal) box filter smoothing
  template <typename PixType>
  void box_average_1d( const vil_image_view<PixType>& src,
                       vil_image_view<PixType>& dst,
                       unsigned width )
  {
    if( src.ni() <= 0 )
    {
      LOG_ERROR( p->logger(), "Image width must be non-zero" );
    }
    if( width % 2 == 0 )
    {
      LOG_ERROR( p->logger(), "Kernel width must be odd" );
    }

    if( width >= src.ni() )
    {
      width = ( src.ni() == 1 ? 1 : (src.ni()-2) | 0x01 );
    }

    unsigned ni = src.ni(), nj = src.nj(), np = src.nplanes();
    dst.set_size(ni,nj,np);
    unsigned half_width = width / 2;

    std::ptrdiff_t istepS=src.istep(),  jstepS=src.jstep(),  pstepS=src.planestep();
    std::ptrdiff_t istepD=dst.istep(),  jstepD=dst.jstep(),  pstepD=dst.planestep();

    const PixType*   planeS = src.top_left_ptr();
    PixType*         planeD = dst.top_left_ptr();

    for (unsigned p=0; p<np; ++p, planeS+=pstepS, planeD+=pstepD)
    {
      const PixType* rowS = planeS;
      PixType*       rowD = planeD;
      for (unsigned j=0; j<nj; ++j, rowS += jstepS, rowD += jstepD)
      {
        const PixType* pixelS1 = rowS;
        const PixType* pixelS2 = rowS;
        PixType*       pixelD = rowD;

        // fast box filter smoothing by adding one pixel to the sum and
        // subtracting another pixel at each step
        unsigned sum = 0;
        unsigned i=0;

        // initialize the sum for half the kernel width
        for (; i<=half_width; ++i, pixelS2+=istepS)
        {
          sum += *pixelS2;
        }

        // starting boundary case: the kernel width is expanding
        for (; i<width; ++i, pixelS2+=istepS, pixelD+=istepD)
        {
          *pixelD = static_cast<PixType>(sum / i);
          sum += *pixelS2;
        }

        // general case: add the leading edge and remove the trailing edge.
        for (; i<ni; ++i, pixelS1+=istepS, pixelS2+=istepS, pixelD+=istepD)
        {
          *pixelD = static_cast<PixType>(sum / width);
          sum -= *pixelS1;
          sum += *pixelS2;
        }

        // ending boundary case: the kernel is shrinking
        for (i=width; i>half_width; --i, pixelS1+=istepS, pixelD+=istepD)
        {
          *pixelD = static_cast<PixType>(sum / i);
          sum -= *pixelS1;
        }
      }
    }
  }

  // Apply a high pass filter to one frame and return the output
  template <typename PixType>
  kwiver::vital::image_container_sptr
  filter( vil_image_view< PixType >& input )
  {
    vil_image_view< PixType > output;
    if ( mode == "box" )
    {
      output = box_high_pass_filter(input);
    }
    else if ( mode == "bidir" )
    {
      output = bidirection_box_filter(input);
    }
    else
    {
      return nullptr;
    }

    // Only report the summation of directional filterings, contained in the
    // last plane
    if( output_net_only && output.nplanes() > 1)
    {
      output = vil_plane( output, output.nplanes()-1 );
    }

    return std::make_shared< vxl::image_container >( output );
  }
};

// ----------------------------------------------------------------------------
high_pass_filter
::high_pass_filter()
: d( new priv( this ) )
{
  attach_logger( "arrows.vxl.high_pass_filter" );
}

// ----------------------------------------------------------------------------
high_pass_filter
::~high_pass_filter()
{
}

// ----------------------------------------------------------------------------
vital::config_block_sptr
high_pass_filter
::get_configuration() const
{
  // get base config from base class
  vital::config_block_sptr config = algorithm::get_configuration();

  config->set_value( "mode", d->mode,
    "Operating mode of this filter, possible values: box, bidir" );
  config->set_value( "kernel_width", d->kernel_width,
    "Pixel width of smoothing kernel" );
  config->set_value( "kernel_height", d->kernel_height,
    "Pixel height of smoothing kernel" );
  config->set_value( "treat_as_interlaced", d->treat_as_interlaced,
    "Process alternating rows independently" );
  config->set_value( "output_net_only", d->output_net_only,
    "If set to false, the output image will contain multiple planes, each representing "
    "the modal filter applied at different orientations, as opposed to a single plane "
    "image representing the sum of filters applied in all directions." );

  return config;
}

// ----------------------------------------------------------------------------
void
high_pass_filter
::set_configuration( vital::config_block_sptr in_config )
{
  // Starting with our generated vital::config_block to ensure that assumed values
  // are present. An alternative is to check for key presence before performing a
  // get_value() call.
  vital::config_block_sptr config = this->get_configuration();
  config->merge_config( in_config );

  // Settings for filtering
  d->mode = config->get_value< std::string >( "mode" );
  d->kernel_width = config->get_value< size_t >( "kernel_width" );
  d->kernel_height = config->get_value< size_t >( "kernel_height" );
  d->treat_as_interlaced = config->get_value< bool >( "treat_as_interlaced" );
  d->output_net_only = config->get_value< bool >( "output_net_only" );
}

// ----------------------------------------------------------------------------
bool
high_pass_filter
::check_configuration( vital::config_block_sptr config ) const
{
  return ( config->get_value< std::string >( "mode" ) == "box" ||
           config->get_value< std::string >( "mode" ) == "bidir" );
}

// ----------------------------------------------------------------------------
kwiver::vital::image_container_sptr
high_pass_filter
::filter( kwiver::vital::image_container_sptr image_data )
{
  vil_image_view_base_sptr view =
    vxl::image_container::vital_to_vxl( image_data->get_image() );

  #define HANDLE_CASE(T)                                                 \
  case T:                                                                \
  {                                                                      \
    typedef vil_pixel_format_type_of< T >::component_type pix_t;         \
    auto input = static_cast< vil_image_view< pix_t > >( *view );        \
    kwiver::vital::image_container_sptr output = d->filter( input );     \
    return output;                                                       \
  }                                                                      \

  switch( view->pixel_format() )
  {
    HANDLE_CASE(VIL_PIXEL_FORMAT_BOOL)
    HANDLE_CASE(VIL_PIXEL_FORMAT_SBYTE)
    HANDLE_CASE(VIL_PIXEL_FORMAT_UINT_16)
    HANDLE_CASE(VIL_PIXEL_FORMAT_INT_16)
    HANDLE_CASE(VIL_PIXEL_FORMAT_UINT_32)
    HANDLE_CASE(VIL_PIXEL_FORMAT_INT_32)
    HANDLE_CASE(VIL_PIXEL_FORMAT_UINT_64)
    HANDLE_CASE(VIL_PIXEL_FORMAT_INT_64)
    HANDLE_CASE(VIL_PIXEL_FORMAT_FLOAT)
    HANDLE_CASE(VIL_PIXEL_FORMAT_DOUBLE)
  #undef HANDLE_CASE

  default:
    LOG_ERROR( logger(), "Invalid input format " << view->pixel_format()
                         << " type received" );
    return kwiver::vital::image_container_sptr();
  }

  return kwiver::vital::image_container_sptr();
}

} // end namespace vxl
} // end namespace arrows
} // end namespace kwiver
