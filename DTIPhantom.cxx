#include "DTIPhantomCLP.h"
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkDiffusionTensor3D.h>
#include <random>

typedef itk::Image< itk::DiffusionTensor3D< float > , 3 > ImageType ;

void CreateGrid( std::string referenceVolume ,
                 std::vector<double> outputImageSize ,
                 ImageType::Pointer &image
               )
{
  if( !referenceVolume.empty() )
  {
    itk::ImageFileReader< ImageType >::Pointer reader = itk::ImageFileReader< ImageType >::New() ;
    reader->SetFileName( referenceVolume ) ;
    reader->Update() ;
    image->CopyInformation( reader->GetOutput() ) ;
    image->SetRegions( reader->GetOutput()->GetLargestPossibleRegion() ) ;
    image->Allocate() ;
  }
  else
  {
    ImageType::SizeType size ;
    for( int i = 0 ; i < 3 ; i++ )
    {
      size[ i ] = outputImageSize[ i ] ;
    }
    image->SetRegions( size ) ;
    image->Allocate() ;
  }
}


void tube( ImageType::Pointer &image , double shapeSize , double  std_noise )
{
  typedef itk::ContinuousIndex< double , 3 > ContinuousIndex ;
  ImageType::SizeType size = image->GetLargestPossibleRegion().GetSize() ;
  ImageType::SpacingType spacing = image->GetSpacing() ;
  ContinuousIndex centerIndex ;
  for( int i = 0 ; i < 3 ; i++ )
  {
    centerIndex[i] = (double)size[i]/2.0 ;
  }
  ImageType::PointType centerPoint ;
  image->TransformContinuousIndexToPhysicalPoint( centerIndex , centerPoint ) ;
  itk::DiffusionTensor3D< float > zeroTensor ;
  zeroTensor.Fill( 0.0 ) ;
  itk::DiffusionTensor3D< float > tensor ;
  double radius = 0 ;
  int position ;
  position = (size[ 0 ] < size[ 1 ] ? 0 : 1 ) ;
  radius = size[ position ]*spacing[ position ] - centerPoint[ position ] ;
  radius *= shapeSize ;
  //We compute the square of the radius to avoid computing the sqrt later
  radius = radius * radius ;
  ImageType::IndexType index ;
  ImageType::PointType point ;
  std::default_random_engine generator;
  std_noise = std_noise * 1e-6 ;
  std::normal_distribution<double> distribution_eigen1( 9e-6 , std_noise ) ;
  std::normal_distribution<double> distribution_eigen2( 1e-6 , std_noise ) ;
  std::normal_distribution<double> distribution_eigen3( 1e-6 , std_noise ) ;
  std::normal_distribution<double> distribution_rest( 0 , std_noise ) ;
  for( index[ 2 ] = 0 ; index[ 2 ] < size[ 2 ] ; index[ 2 ]++ )
  {
    for( index[ 0 ] = 0 ; index[ 0 ] < size[ 0 ] ; index[ 0 ]++ )
    {
      for( index[ 1 ] = 0 ; index[ 1 ] < size[ 1 ] ; index[ 1 ]++ )
      {
        ImageType::PointType point ;
        image->TransformIndexToPhysicalPoint( index , point ) ;
        double distance = 0 ;
        for( int i = 0 ; i < 2 ; i++ )
        {
          distance += (point[i]-centerPoint[i])*(point[i]-centerPoint[i]);
        }
        if( distance < radius )
        {
          for( int i = 0 ; i < 6 ; i++ )
          {
            tensor.Fill( distribution_rest( generator ) ) ;
          }
          tensor(0,0) = distribution_eigen2( generator ) ;
          tensor(1,1) = distribution_eigen3( generator ) ;
          tensor(2,2) = distribution_eigen1( generator ) ;
          image->SetPixel( index , tensor ) ;
        }
        else
        {
          image->SetPixel( index , zeroTensor ) ;
        }
      }
    }
  }
}

int main( int argc , char* argv[] )
{
  PARSE_ARGS ;
  if( outputImageSize.size() != 3 )
  {
    std::cout << "outputImageSize needs 3 values" << std::endl ;
    return EXIT_FAILURE ;
  }
  if( shapeSize <= 0.0 || shapeSize > 1.0 )
  {
    std::cout << "shapeSize has to be larger than 0.0 and smaller or equal to 1.0" << std::endl ;
    return EXIT_FAILURE ;
  }
  if( outputVolume.empty() )
  {
    std::cout << "The name of the output volume has to be specified" << std::endl ;
    return EXIT_FAILURE ;
  }
  ImageType::Pointer image = ImageType::New() ;
  CreateGrid( referenceVolume , outputImageSize , image ) ;
  if( shape == "tube" )
  {
    tube( image , shapeSize , std_noise ) ;
  }
  typedef itk::ImageFileWriter< ImageType > WriterType ;
  WriterType::Pointer writer = WriterType::New() ;
  writer->SetFileName( outputVolume )  ;
  writer->SetInput( image ) ;
  writer->UseCompressionOn() ;
  writer->Update() ;
  return EXIT_SUCCESS ;
}
