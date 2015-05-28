# Image-resizing
by Tilen Matkovič

We want to crop images from NxM to XxY, but we don't want to remove important parts. Our assignment consists of two parts: manual (we mark the important-featured part) and automatic. Program was made with Visual Studio 2012 in C++ and OpenCV libraries (2.4.11).

Program options:

Resize.exe --input <"path to input file"> --output <"path to output - cropped file"> --featured <"first point">-<"second point"> --size <"output size">

# Manual
We start manual cropping with

Resize.exe --input in.jpg --output out.jpg --featured 50x50-150x100 --size 300x250

Main idea: we calculate the center of the featured part in the image and expand the featured width and height to our arbitrary dimensions. This is in file resizeManual.cpp.


#Automatic
We start automatic cropping with

Resize.exe --input in.jpg --output out.jpg --size 300x400

Idea: the algorithm consists of multiple algorithms. The main is seam carving but I use it only to bring important parts closer to center. Before I use seam carving, I calculate the gradient(25%) + "SALIENT REGION DETECTION WITH OPPONENT COLOR BOOSTING"(75%) which I put in seam carving energy calculation to calculate the seam with the lowest energy. When I am done with seam carving I calculate the center of carved image, see where that point is in the original image and crop the original image to our arbitrary size. The reason I did this is because sometimes the image after seam carving is really deformed and hard to look at it. With calculated center there is no deformation, but the main object is sometimes cut - it is not fully in the output image.

Also, if you use large images (more than 1500x1500) be very patient - it may take minutes to finish. (it does not work on all images)

Good example:
Input

![ExampleAutomaticIn](http://shrani.si/f/1h/XJ/33SPkIue/zeman1.png)

Output - size 200x400

![ExampleAutomaticOut](http://shrani.si/f/2W/5M/38fEQ71u/zeman1a.jpg)
