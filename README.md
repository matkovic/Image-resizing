# Image-resizing
by Tilen Matkovič

We want to crop images from NxM to XxY, but we don't want to remove important parts. Our assignment consists of two parts: manual (we mark the important-featured part) and automatic. Program was made with Visual Studio 2012 in C++ and OpenCV libraries (2.4.11). If you want to compile it, make sure you have installed OpenCV (PATH set). I used local method, OpenCV_Debug.props and OpenCV_Release.props are there if you want them and correct settings if needed. If you don't want to mess with this, everything you need is in 'program' folder. Please contact me if you have any comments.

Program options:

Resize.exe --input <"path to input file"> --output <"path to output - cropped file"> --featured <"first point">-<"second point"> --size <"output size">

# Manual
We start manual cropping with

Resize.exe --input in.jpg --output out.jpg --featured 50x50-150x100 --size 300x250

x - this is small letter 'x'

Main idea: we calculate the center of the featured part in the image and expand the featured width and height to our arbitrary dimensions. This is in file resizeManual.cpp.

Example:

Resize.exe --input 106005.jpg --output 106005n.jpg --size 250x250 --featured 120x20-360x250

![ExampleManualIn](http://shrani.si/f/2j/ro/15EKylE/106005.jpg)

![ExampleManualOut](http://shrani.si/f/3g/Ce/4PcKB9pT/106005n.jpg)

#Automatic
We start automatic cropping with

Resize.exe --input in.jpg --output out.jpg --size 300x400

Idea: the algorithm consists of multiple algorithms. The main is seam carving but I use it only to bring important parts closer to center. Before I use seam carving, I calculate the gradient(25%) + "SALIENT REGION DETECTION WITH OPPONENT COLOR BOOSTING"(75%) which I put in seam carving energy calculation to calculate the seam with the lowest energy. When I am done with seam carving I calculate the center of carved image, see where that point is in the original image and crop the original image to our arbitrary size. The reason I did this is because sometimes the image after seam carving is really deformed. With calculated center there is no deformation, but the main object is sometimes cut - it is not fully in the output image.

Sometimes it may take 10s or more. (it does not work perfectly on all images - works on images where object is prevalent)

Good example:

Resize.exe --input zeman1.jpg --output zeman1a.jpg --size 200x400

Input

![ExampleAutomaticIn](http://shrani.si/f/1h/XJ/33SPkIue/zeman1.png)

Output - size 200x400

![ExampleAutomaticOut](http://shrani.si/f/2W/5M/38fEQ71u/zeman1a.jpg)
