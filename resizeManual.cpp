#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "resizeAM.h"

using namespace cv;
using namespace std;


int clip(int n, int lower, int upper) {
  return max(lower, min(n, upper));
}

int resizeManual(string input,string output, Point size, Point featured1, Point featured2)
{
	int widthFeatured = featured2.x-featured1.x;
	int heightFeatured = featured2.y-featured1.y;
    Mat image;
	Mat featuredRectangle;
	Point centerPoint;
	Point newPoint1;
	Point newPoint2;
	int newWidth;
	int newHeight;
	Mat outputImage;


    image = imread(input, IMREAD_COLOR); // Read the file
    if(! image.data ) // Check for invalid input
    {
        cout << "Could not open or find the image" << std::endl ;
        return -1;
    }
	/*ERRORS*/
	if( clip(size.x,0,image.size().width)!=size.x || clip(size.y,0,image.size().height)!=size.y || 
		clip(featured1.x,0,image.size().width)!=featured1.x || clip(featured1.y,0,image.size().width)!=featured1.y ||
		clip(featured2.x,0,image.size().width)!=featured2.x || clip(featured2.y,0,image.size().width)!=featured2.y ||
		featured1.x>=featured2.x || featured1.y>=featured2.y)
    {
		cout << "Wrong input" << endl;
		return -1;
    }

	if( size.x<widthFeatured )
	{
		cout << "Warning - featured part must not be cut" << endl;
		size.x=widthFeatured;
	}
	if( size.y<heightFeatured )
	{
		cout << "Warning - featured part must not be cut" << endl;
		size.y=heightFeatured;
	}

	featuredRectangle = image(Rect(featured1.x, featured1.y, widthFeatured, heightFeatured));

	centerPoint = Point(featured1.x + widthFeatured/2, featured1.y + heightFeatured/2);

	if( (image.cols-centerPoint.x)<size.x/2 ) centerPoint.x=image.cols-size.x/2;
	else if( centerPoint.x<size.x/2 ) centerPoint.x=size.x/2;

	if( (image.rows-centerPoint.y)<size.y/2 ) centerPoint.y=image.rows-size.y/2;
	else if( centerPoint.y<size.y/2 ) centerPoint.y=size.y/2;

	newPoint1 = Point(clip(centerPoint.x-size.x/2,0,image.size().width), clip(centerPoint.y-size.y/2,0,image.size().height));
	newPoint2 = Point(clip(centerPoint.x+size.x/2,0,image.size().width), clip(centerPoint.y+size.y/2,0,image.size().height));
	newWidth = newPoint2.x-newPoint1.x;
	newHeight = newPoint2.y-newPoint1.y;
	outputImage = image(Rect(newPoint1.x, newPoint1.y, newWidth, newHeight));
	
    //namedWindow( "Display window", WINDOW_AUTOSIZE ); // Create a window for display.
    //imshow( "Display window", image ); // Show our image inside it.
	//imshow( "part", featuredRectangle);
	//imshow( "New image", outputImage);

	imwrite(output,outputImage);

    return 0;
}

