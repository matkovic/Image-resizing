
#include<iostream>
#include <string>
#include "optionparser.h"
#include "resizeAM.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

struct Arg: public option::Arg
{
  static void printError(const char* msg1, const option::Option& opt, const char* msg2)
  {
    fprintf(stderr, "%s", msg1);
    fwrite(opt.name, opt.namelen, 1, stderr);
    fprintf(stderr, "%s", msg2);
  }

  static option::ArgStatus Unknown(const option::Option& option, bool msg)
  {
    if (msg) printError("Unknown option '", option, "'\n");
    return option::ARG_ILLEGAL;
  }

  static option::ArgStatus Required(const option::Option& option, bool msg)
  {
    if (option.arg != 0)
      return option::ARG_OK;

    if (msg) printError("Option '", option, "' requires an argument\n");
    return option::ARG_ILLEGAL;
  }

};





enum  optionIndex { UNKNOWN, HELP, SIZE, FEATURED, INPUT, OUTPUT };
const option::Descriptor usage[] =
{
 {UNKNOWN, 0, "", "",option::Arg::None, "USAGE: resize [options]\n\n"
                                        "Options:" },
 {HELP, 0,"", "help",option::Arg::None, "  --help  \tPrint usage and exit." },
 {SIZE, 0,"s","size",option::Arg::Required, "  --size, -s [<x>x<y>]  \tOutput image size" },
 {FEATURED, 0,"f","featured",option::Arg::Required, "  --featured, -f [<x>x<y>] \tCoordinates of the important part of the image (optional)" },
 {INPUT, 0,"i","input",option::Arg::Required, "  --input, -i [...]\tPath to the original image"},
 {OUTPUT, 0, "o","output",option::Arg::Required, "  --output, -o [...] \tPath to the output image"},
 {UNKNOWN, 0, "", "",option::Arg::None, "\nExamples:\n"
                               "  Resize.exe --size 320x320 --featured 100x100-150x150 --input /path/to/original/image.png --output output.png\n"
							   "  Resize.exe --size 320x320 --input /path/to/original/image.png --output output.png\n"
                               "  \n" },
 {0,0,0,0,0,0}
};


void delimit( string str, string delimiter, string& first, string& second )
{
	first = str.substr(0, str.find(delimiter));
	second = str.substr(str.find(delimiter)+1, str.size());
}


int main( int argc, char** argv )
{
	
	argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
	option::Stats  stats(usage, argc, argv);
	option::Option* options = new option::Option[stats.options_max];
	option::Option* buffer  = new option::Option[stats.buffer_max];
	option::Parser parse(usage, argc, argv, options, buffer);

	if (parse.error())
		return 1;

	if (options[HELP] || argc == 0) {
		option::printUsage(std::cout, usage);
		return 0;
	}

	string size = "";
	string featured = "";
	string input = "";
	string output = "";

	for (int i = 0; i < parse.optionsCount(); ++i)
		{
			option::Option& opt = buffer[i];
			switch (opt.index())
			{
				case SIZE: 
					size=opt.arg;
					break;
				case FEATURED: 
					featured=opt.arg;
					break;
				case INPUT:
					input=opt.arg;
					break;
				case OUTPUT:
					output=opt.arg;
					break;
				case UNKNOWN:
					// not possible
					break;
			}
	  }
	
	for (option::Option* opt = options[UNKNOWN]; opt; opt = opt->next())
	cout << "Unknown option: " << string(opt->name,opt->namelen) << "\n";

	for (int i = 0; i < parse.nonOptionsCount(); ++i)
	cout << "Non-option #" << i << ": " << parse.nonOption(i) << "\n";

	delete[] options;
	delete[] buffer;

	///
	///
	//break size and featured (if selected)
	string sizeX="";
	string sizeY="";
	string delimiter="x";

	if(size.length()!=0)
		delimit(size,delimiter,sizeX,sizeY);

	string featured1X="";
	string featured1Y="";
	string featured2X="";
	string featured2Y="";
	
	if(featured.length()!=0)
	{
		string featured1="";//first coordinate
		string featured2="";//second coordinate
		delimiter="-";
		delimit(featured, delimiter, featured1, featured2);
		delimiter="x";
		delimit(featured1,delimiter,featured1X,featured1Y);
		delimit(featured2,delimiter,featured2X,featured2Y);
	}

	if(size.length()!=0 && featured.length()!=0 && input.length()!=0 && output.length()!=0 && atoi(sizeX.c_str())!=0 && atoi(sizeY.c_str())!=0)
	{
		Point size = Point(atoi(sizeX.c_str()),atoi(sizeY.c_str()));
		Point featured1 = Point(atoi(featured1X.c_str()),atoi(featured1Y.c_str()));
		Point featured2 = Point(atoi(featured2X.c_str()),atoi(featured2Y.c_str()));
		resizeManual(input, output, size,featured1,featured2);
		//manual
	}
	else if(size.length()!=0 && input.length()!=0 && output.length()!=0 && atoi(sizeX.c_str())!=0 && atoi(sizeY.c_str())!=0)
	{
		Point size = Point(atoi(sizeX.c_str()),atoi(sizeY.c_str()));
		resizeAutomatic(input,output,size);
		//automatic
	}
	else
	{
		cout << "Error, check help (--help)" << endl;
		return -1;
		//error
	}



}



