#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include "resizeAM.h"
#include <limits>

# define M_PI           3.14159265358979323846  /* pi */

using namespace cv;
using namespace std;


Mat gDer(Mat f, int sigma, int iorder, int jorder)
{
	//translated from Matlab - use with calcSalientColor
	//calculates gaussian derivative

	int break_off_sigma=3;
	int filtersize = (int)floor(break_off_sigma*sigma+0.5);
	int minusFiltersize=-filtersize;

    Mat x;
	while(minusFiltersize <= filtersize) {
		x.push_back(minusFiltersize);
        minusFiltersize += 1;
    }
	x.convertTo(x,CV_32F);

	double sqrt1;
	sqrt1=sqrt(2*M_PI);
	Mat exp1;
	
	exp(x.mul(x)/(-2*sigma*sigma),exp1);

	Mat Gauss = 1/(sqrt1*sigma) * exp1;
	
	Mat Gx;
	switch(iorder)
	{
	case 0:
		Gx=Gauss/sum(Gauss)[0];
		break;
	case 1:
		Gx=(-x/(sigma*sigma)).mul(Gauss);
		Gx=Gx/sum(x.mul(Gx))[0];
		break;
	}
	Mat H;
	filter2D(f,H,CV_32FC1,Gx);

	Mat Gy;
	switch(jorder)
	{
	case 0:
		Gy=Gauss/sum(Gauss)[0];
		break;
	case 1:
		Gy=(-x/(sigma*sigma)).mul(Gauss);
		Gy=Gy/sum(x.mul(Gy))[0];
		break;
	}

	transpose(Gy,Gy);
	filter2D(H,H,CV_32FC1,Gy);
	
	return H;
}

Mat calcSalientColor(Mat image)
{
	//calculate color saliency from article "SALIENT REGION DETECTION WITH OPPONENT COLOR BOOSTING"
	//return gray image with important parts

	Mat OpponentMatrix = (Mat_<float>(3,3) << 0.3905, 0.5499, 0.0089, -0.1764, 0.4307, -0.1164, -0.1191, -0.1739, 0.8673);
	Mat O[3];
	
	Mat channel[3];//bgr
	split(image,channel);

	O[0]=OpponentMatrix.at<float>(0,0)*channel[2] + OpponentMatrix.at<float>(0,1)*channel[1] + OpponentMatrix.at<float>(0,2)*channel[0];
	O[1]=OpponentMatrix.at<float>(1,0)*channel[2] + OpponentMatrix.at<float>(1,1)*channel[1] + OpponentMatrix.at<float>(1,2)*channel[0];
	O[2]=OpponentMatrix.at<float>(2,0)*channel[2] + OpponentMatrix.at<float>(2,1)*channel[1] + OpponentMatrix.at<float>(2,2)*channel[0];

	Mat opponent = Mat(O[0].rows,O[0].cols,CV_32FC1);
	merge(O,3,opponent);
	
	Mat M = Mat::zeros(3,3,CV_32F);
	
	int sigma=1;
	Mat concat1,concat2;
	for(int i=0;i<M.rows;i++)
	{
		hconcat(gDer(O[i],sigma,1,0),gDer(O[i],sigma,0,1),concat1);
		for(int j=0;j<M.cols;j++)
		{			
			hconcat(gDer(O[j],sigma,1,0),gDer(O[j],sigma,0,1),concat2);
			
			Mat iSubtract, jSubtract;
			subtract(concat1,mean(concat1)[0],iSubtract);
			subtract(concat2,mean(concat2)[0],jSubtract);

			Mat OiOj;
			multiply(iSubtract,jSubtract,OiOj);
			
			M.at<float>(i,j)=(float)sum(OiOj)[0];			
		}
	}

	M=M/(image.rows*image.cols);

	Mat S,U,V;
	SVD::compute(M,S,U,V);

	Mat gFunction = Mat(3,3,CV_32F);
	//transpose(V,V);
	Mat squareRoot;
	sqrt(S,squareRoot);
	gFunction=U*(Mat::diag(1/squareRoot)*V);
	
	Mat arrayFromRgb = opponent.reshape(1, opponent.rows*opponent.cols);
	
	transpose(arrayFromRgb,arrayFromRgb);

	Mat floatArrayFromRgb;
	arrayFromRgb.convertTo(floatArrayFromRgb,CV_32F);

	Mat boostedColor = gFunction*floatArrayFromRgb;
	transpose(boostedColor,boostedColor);

	boostedColor = boostedColor.reshape(3, opponent.rows);

	GaussianBlur(boostedColor, boostedColor, Size(5,5),0,0);

	split(boostedColor,channel);
	
	Mat means = (Mat_<float>(1,3) << mean(channel[0])[0], mean(channel[1])[0], mean(channel[2])[0]);

	Mat Salient  = boostedColor.reshape(1, boostedColor.rows*boostedColor.cols);
	Salient.convertTo(Salient,CV_32F);
	Mat ss = Mat(1,Salient.rows,CV_32F);

	for(int i=0;i<Salient.rows;i++)
	{
		ss.at<float>(i) = (float)norm(means-Salient.row(i),NORM_L2);
	}
	ss = ss.reshape(1, boostedColor.rows);

	ss.convertTo(ss,CV_8UC1);

	normalize(ss,ss,0,255,NORM_MINMAX, CV_8UC1);

	double T = 2*sum(ss)[0] / (ss.cols*ss.rows);
	threshold( ss, ss, T, 255, CV_THRESH_BINARY );

	return ss;
}

Mat calcGradient(Mat image)
{
	//calculate scharr gradient

	//gradient
	Mat grayImage;
	cvtColor(image,grayImage,CV_RGB2GRAY);
	int scale = 1;
	int delta = 0;
	int ddepth = CV_16S;
	/// Generate grad_x and grad_y
	Mat grad_x, grad_y, grad;
	Mat abs_grad_x, abs_grad_y;
	
	/// Gradient X
    Scharr( grayImage, grad_x, ddepth, 1, 0, scale, delta, cv::BORDER_DEFAULT );
    convertScaleAbs( grad_x, abs_grad_x );
    /// Gradient Y  
    Scharr( grayImage, grad_y, ddepth, 0, 1, scale, delta, cv::BORDER_DEFAULT );
    convertScaleAbs( grad_y, abs_grad_y );
	addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );
	
	return grad;
}

Mat calculateEnergy(Mat image)
{
	//calculate energy - gray image from image

	Mat energy;
	Mat salientColor = calcSalientColor(image);
	Mat gradient = calcGradient(image);
	addWeighted(salientColor,0.75,gradient,0.25,0,energy);

	return energy;
}

Mat findSeam(Mat listOfSeams)
{
	//list of seam = calculated energy of seam from calculateSeam()
	//seamEnergy = lowest energy (1xN vector with positions of which pixel to remove)
	//return seam with lowest energy

	Mat seamOptimal = Mat(listOfSeams.size().height, 1, CV_32F);
	
	Mat lastRow;
	listOfSeams.row(listOfSeams.rows-1).copyTo(lastRow);
	
	double maxVal;
	double minVal;
	Point minLoc;
	
	minMaxLoc(lastRow,&minVal,&maxVal,&minLoc);
	seamOptimal.at<float>(listOfSeams.rows-1,0)=(float)minLoc.x;
	
	int j = minLoc.x;

	float* pCurrent = (float*) listOfSeams.data;;
	size_t elem_step = listOfSeams.step / sizeof(float);
	for(int row=listOfSeams.rows-2; row>=0; row--)
	{
		Mat vector=Mat(1,3,CV_32F);

		if(seamOptimal.at<float>(row+1,0)==0)
		{
			vector.at<float>(0,0)=std::numeric_limits<float>::max();
			vector.at<float>(0,1)=pCurrent[row*elem_step +j];
			vector.at<float>(0,2)=pCurrent[row*elem_step +j+1];
		}
		else if(seamOptimal.at<float>(row+1,0)==listOfSeams.cols-1)
		{
			vector.at<float>(0,0)=pCurrent[row*elem_step +j-1];
			vector.at<float>(0,1)=pCurrent[row*elem_step +j];
			vector.at<float>(0,2)=std::numeric_limits<float>::max();;
		}
		else
		{
			vector.at<float>(0,0)=pCurrent[row*elem_step +j-1];
			vector.at<float>(0,1)=pCurrent[row*elem_step +j];
			vector.at<float>(0,2)=pCurrent[row*elem_step +j+1];
		}
		minMaxLoc(vector,&minVal,&maxVal,&minLoc);
		int inc=minLoc.x-1;
		j=(int)seamOptimal.at<float>(row+1,0)+inc;
		seamOptimal.at<float>(row,0)=(float)j;
	}
	return seamOptimal;
}

Mat calculateSeam(Mat energy)
{
	//energy=gradient+colorSaliency
	//calculate seam energy map from Mat energy
	//seamEnergy=lowest energy
	//return seam with lowest energy

	//made with pointers for efficiency

	Mat seam;
	Mat energyCopy;
	energy.convertTo(energyCopy,CV_32F);

    CV_Assert(energyCopy.depth() != sizeof(uchar));

    int channels = energyCopy.channels();

    int nRows = energyCopy.rows;
    int nCols = energyCopy.cols;

    int i,j;
	float* pCurrent = (float*) energyCopy.data;;
	size_t elem_step = energyCopy.step / sizeof(float);
	for( i = 1; i < nRows; i++)
    {
		for ( j = 0; j < nCols; j++)
        {
			if(j==0)
			{
				pCurrent[i*elem_step + j]+=min(pCurrent[(i-1)*elem_step + (j)],pCurrent[(i-1)*elem_step + (j+1)]);
			}
			else if(j==nCols-1)
			{
				pCurrent[i*elem_step + j]+=min(pCurrent[(i-1)*elem_step + (j-1)],pCurrent[(i-1)*elem_step + (j)]);
			}
			else
			{
				float minim = min(pCurrent[(i-1)*elem_step + (j)],pCurrent[(i-1)*elem_step + (j+1)]);
				pCurrent[i*elem_step + j]+=min(pCurrent[(i-1)*elem_step + (j-1)],minim);
			}
        }
    }

	seam = findSeam(energyCopy);

	return seam;
}

Mat removeSeam(Mat image, Mat seam)
{
	//remove column vector(Mat seam) - seam(0)=x-location... - from image(NxM)
	//return image without seam

	Mat tempImage;
	
	for(int i=0;i<seam.rows;i++)
	{
		float seamValue = seam.at<float>(i,0);
		Mat row = image.row(i);
		if(seamValue==0)
		{
			tempImage.push_back(row.colRange(1,image.size().width));
		}
		else if(seamValue==image.size().width-1)
		{
			tempImage.push_back(row.colRange(0,image.size().width-1));
		}
		else
		{
			Mat first=row.colRange(0,(int)seamValue);
			Mat second=row.colRange((int)seamValue+1,image.size().width);
			Mat concat;
			hconcat(first,second,concat);
			tempImage.push_back(concat);
		}
	}

	return tempImage;	
}

Mat removeCol(Mat seam, Mat rc)
{
	//check which value is in the middle of seam and
	//remove middle value from rc
	Mat rowCol;

	if(seam.at<float>(seam.rows/2) ==0 )
	{
		rowCol=rc.colRange(1,rc.cols);
	}
	else if(seam.at<float>(seam.rows/2)==rc.cols-1)
	{
		rowCol=rc.colRange(0,rc.cols-1);
	}
	else
	{
		Mat rc1=rc.colRange(0,(int)seam.at<float>(seam.rows/2));
		Mat rc2=rc.colRange((int)seam.at<float>(seam.rows/2)+1,rc.cols);

		hconcat(rc1,rc2,rowCol);				
	}	
	return rowCol;
}

Point centerInOriginalImage;
Mat calculateBySeamOrder (Mat image, int reduceRows, int reduceCols)
{
	//reduce reduceRows rows and reduceCols columns from image
	//reducing with seam carving
	//also calculate center in image with removed seams in original image
	
	Mat rows=Mat::zeros(1,image.rows,CV_32S); // for calculating the center of image with removed seams, in original image
	Mat cols=Mat::zeros(1,image.cols,CV_32S); //
	for(int i=0;i<image.rows;i++) rows.at<int>(i)=i;
	for(int i=0;i<image.cols;i++) cols.at<int>(i)=i;
	
	Mat imageClone = image.clone();

	Mat energy = calculateEnergy(imageClone);

	if(reduceCols>reduceRows) //columns or rows first - which is higher
	{
		for(int i=0;i<reduceCols;i++)
		{
				Mat seamVertical=calculateSeam(energy);
				imageClone=removeSeam(imageClone,seamVertical);
				energy=removeSeam(energy,seamVertical);	

				cols=removeCol(seamVertical,cols);

				if(i%20==0) {
					energy = calculateEnergy(imageClone);
				}
		}
		transpose(energy,energy);
		transpose(imageClone,imageClone);
		for(int j=0;j<reduceRows;j++)
		{
				Mat seamHorizontal=calculateSeam(energy);
				imageClone=removeSeam(imageClone,seamHorizontal);
				energy=removeSeam(energy,seamHorizontal);

				rows=removeCol(seamHorizontal,rows);
				if(j%20==0) {
					energy = calculateEnergy(imageClone);
				}
		}
		transpose(imageClone,imageClone);
		transpose(energy,energy);
	}
	else
	{
		transpose(energy,energy);
		transpose(imageClone,imageClone);
		for(int j=0;j<reduceRows;j++)
		{
				Mat seamHorizontal=calculateSeam(energy);
				imageClone=removeSeam(imageClone,seamHorizontal);
				energy=removeSeam(energy,seamHorizontal);

				rows=removeCol(seamHorizontal,rows);

				if(j%20==0) {
					energy = calculateEnergy(imageClone);
				}
		}
		transpose(imageClone,imageClone);
		transpose(energy,energy);
		for(int i=0;i<reduceCols;i++)
		{
				Mat seamVertical=calculateSeam(energy);
				imageClone=removeSeam(imageClone,seamVertical);
				energy=removeSeam(energy,seamVertical);	
				cols=removeCol(seamVertical,cols);

				if(i%20==0) {
					energy = calculateEnergy(imageClone);
				}
		}
	}

	centerInOriginalImage=Point(cols.at<int>(cols.cols/2), rows.at<int>(rows.cols/2));
	
	return imageClone;
}

Mat cutImage(Mat image, string output, Point size)
{
	//prepare settings for cutting from image

	Mat newImage;
	image.copyTo(newImage);
	

	//
	double t = (double)getTickCount();  //timer
	//


	int count=1;
	while(newImage.cols>600 && newImage.rows>600) //resize image to smaller dimensions for faster processing
	{
		count++;
		resize(newImage,newImage,Size(newImage.cols/2,newImage.rows/2));
	}

	newImage=calculateBySeamOrder(newImage,newImage.rows-size.y/count,newImage.cols-size.x/count); //remove seams
	centerInOriginalImage=centerInOriginalImage*count;	//multiply to apply to original image
	newImage = image(Rect(centerInOriginalImage.x-size.x/2, centerInOriginalImage.y-size.y/2, size.x, size.y)); //cut from original image


	//
	t = ((double)getTickCount() - t)/getTickFrequency(); 
	std::cout << "Times passed in seconds: " << t << std::endl;
	//


	return newImage;
}

int resizeAutomatic( string input, string output, Point size)
{
    Mat image;
    image = imread(input, IMREAD_COLOR); // Read the file
    if(! image.data ) // Check for invalid input
    {
        cout << "Could not open or find the image" << std::endl ;
        return -1;
    }
	/*ERRORS*/
	
	if( clip(size.x,0,image.size().width)!=size.x || clip(size.y,0,image.size().height)!=size.y)
    {
		cout << "Wrong size" << endl;
		return -1;
    }

	Mat outputImage = cutImage(image,output, size);
	imwrite(output,outputImage);

    waitKey(0); // Wait for a keystroke in the window
    return 0;
}

