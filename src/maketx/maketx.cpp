/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008 Larry Gritz
// 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 
// (this is the MIT license)
/////////////////////////////////////////////////////////////////////////////


#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>
#include <iterator>

#include "argparse.h"
#include "imageio.h"
using namespace OpenImageIO;


static std::string dataformatname = "";
static float gammaval = 1.0f;
static bool depth = false;
static bool verbose = false;
static std::vector<std::string> filenames;
static int tile[3] = { 0, 0, 1 };
static bool scanline = false;
static bool zfile = false;
static std::string channellist;



static int
parse_files (int argc, const char *argv[])
{
    for (int i = 0;  i < argc;  i++)
        filenames.push_back (argv[i]);
    return 0;
}





#if 0
              << "       -o %s             Output directory or filename\n"
              << "       -shadow           Make shadow map\n"
              << "       -shadcube         Make cubical shadow map (order: px,nx
,py,ny,pz,nz)\n"
              << "       -volshad          Make volume shadow map\n"
              << "       -opaquewidth %f   For -volshad, set z fudge factor"
                                           " [default " << volshadeps << "]\n"
#ifndef RELEASE
              << "       -shadowmip        Make shadow mip-map\n"
              << "       -shadowmipcube    Make cubical shadow mipmap (px,nx,py,
ny,pz,nz)\n"
              << "       -volshadcube      Make cubical volume shadow (px,nx,py,
ny,pz,nz)\n"
              << "       -volshadlevel %d  Set the starting mip level for volume
 shadow maps\n"
#endif
              << "       -envlatl          Make lat/long environment map\n"
              << "       -envcube          Make cubical environment map\n"
              << "                             (file order: px,nx,py,ny,pz,nz)\n
"
              << "       -fov %f           envcube/twofish fov (90 - 180)\n"
//              << "       -envsphere        Not implemented\n"
              << "       -lightprobe       Convert a lightprobe to cube env\n"
              << "       -vertcross        Convert vertical cross to env map\n"
              << "       -latl2envcube     Convert a lat/long map to a cube map 
env\n"
              << "       -dump             Dump the file as text\n"
//              << "       -twofish          Not implemented\n"
              << "       -mode %s          black, clamp, periodic, or mirror\n"
              << "       -smode %s         Horizontal edge mode\n"
              << "       -tmode %s         Vertical edge mode\n"
              << "       -noresize         Do not resize texture to pow2\n"
              << "       -Mcamera %f x 16  Set the camera matrix\n"
              << "       -Mscreen %f x 16  Set the screen/viewing matrix\n"
              << "       -separate         Use planarconfig separate (default is

 contig)\n"
              << "       -tilesize %d      Specify a particular tile size\n"
              << "       -tiled            Process image in tiles (use less memo
ry)\n"
              << "       -u                Update mode\n"
              << "       -p %s             Specify search path for input images\
n"
              << "       -ingamma %f       Specify gamma of input files (default
: 1)\n"
              << "       -outgamma %f      Gamma correct output (default: 1)\n"
              << "       -format %s        Output format (default: tiff)\n"
              << "       -uint8 | -uint16  Force output integer bit depth\n"
              << "       -half | -float    Force output float bit depth\n"
              << "       -ch list          channel list\n"
              << "       -note %s          Append to image description/note\n"
              << "       -verbosity %d     Error message reporting level\n"
              << "       -debugdso         Extra messages for dsos\n"
              << "       -errorfile %s     Error file output (default stderr)\n"
#endif


static void
getargs (int argc, char *argv[])
{
    bool help = false;
    ArgParse ap (argc, (const char **)argv);
    if (ap.parse ("Usage:  maketx [options] inputfile outputfile",
                  "%*", parse_files, "",
                  "--help", &help, "Print help message",
                  "-v", &verbose, "Verbose status messages",
                  "-d %s", &dataformatname, "Set the output data format to one of:\n"
                          "\t\t\tuint8, sint8, uint16, sint16, half, float",
                  "-g %f", &gammaval, "Set gamma correction (default = 1)",
                  "--tile %d %d", &tile[0], &tile[1], "Output as a tiled image",
                  "--scanline", &scanline, "Output as a scanline image",
//FIXME           "-z", &zfile, "Treat input as a depth file",
//FIXME           "-c %s", &channellist, "Restrict/shuffle channels",
                  NULL) < 0) {
        std::cerr << ap.error_message() << std::endl;
        ap.usage ();
        exit (EXIT_FAILURE);
    }
    if (help) {
        ap.usage ();
        exit (EXIT_FAILURE);
    }

    if (filenames.size() != 2) {
        std::cerr << "maketx: Must have both an input and output filename specified.\n";
        ap.usage();
        exit (EXIT_FAILURE);
    }
    std::cout << "Converting " << filenames[0] << " to " << filenames[1] << "\n";
}



int
main (int argc, char *argv[])
{
    getargs (argc, argv);

    // Find an ImageIO plugin that can open the input file, and open it.
    ImageInput *in = ImageInput::create (filenames[0].c_str(), "" /* searchpath */);
    if (! in) {
        std::cerr 
            << "maketx ERROR: Could not find an ImageIO plugin to read \"" 
            << filenames[0] << "\" : " << OpenImageIO::error_message() << "\n";
        exit (0);
    }
    ImageIOFormatSpec inspec;
    if (! in->open (filenames[0].c_str(), inspec)) {
        std::cerr << "maketx ERROR: Could not open \"" << filenames[0]
                  << "\" : " << in->error_message() << "\n";
        delete in;
        exit (0);
    }

    // Copy the spec, with possible change in format
    ImageIOFormatSpec outspec = inspec;
    outspec.set_format (inspec.format);
    if (! dataformatname.empty()) {
        if (dataformatname == "uint8")
            outspec.set_format (PT_UINT8);
        else if (dataformatname == "int8")
            outspec.set_format (PT_INT8);
        else if (dataformatname == "uint16")
            outspec.set_format (PT_UINT16);
        else if (dataformatname == "int16")
            outspec.set_format (PT_INT16);
        else if (dataformatname == "half")
            outspec.set_format (PT_HALF);
        else if (dataformatname == "float")
            outspec.set_format (PT_FLOAT);
        else if (dataformatname == "double")
            outspec.set_format (PT_DOUBLE);
    }
    outspec.gamma = gammaval;

    if (tile[0]) {
        outspec.tile_width = tile[0];
        outspec.tile_height = tile[1];
        outspec.tile_depth = tile[2];
    }
    if (scanline) {
        outspec.tile_width = 0;
        outspec.tile_height = 0;
        outspec.tile_depth = 0;
    }

    // Find an ImageIO plugin that can open the output file, and open it
    ImageOutput *out = ImageOutput::create (filenames[1].c_str());
    if (! out) {
        std::cerr 
            << "maketx ERROR: Could not find an ImageIO plugin to write \"" 
            << filenames[1] << "\" :" << OpenImageIO::error_message() << "\n";
        exit (0);
    }
    if (! out->open (filenames[1].c_str(), outspec)) {
        std::cerr << "maketx ERROR: Could not open \"" << filenames[1]
                  << "\" : " << out->error_message() << "\n";
        exit (0);
    }

    char *pixels = new char [outspec.image_bytes()];
    in->read_image (outspec.format, pixels);
    in->close ();
    delete in;
    in = NULL;
    out->write_image (outspec.format, pixels);
    out->close ();
    delete out;
    delete [] pixels;

    return 0;
}