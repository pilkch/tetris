// Standard libraries
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <set>
#include <bitset>

// writing on a text file
#include <iostream>
#include <fstream>

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_image.h>

// Breathe
#include <breathe/breathe.h>

#include <breathe/util/cString.h>
#include <breathe/util/log.h>
#include <breathe/util/cVar.h>

#include <breathe/storage/filesystem.h>
#include <breathe/storage/file.h>
#include <breathe/storage/xml.h>

#include <breathe/util/cTimer.h>

#include <breathe/math/math.h>
#include <breathe/math/cVec2.h>
#include <breathe/math/cVec3.h>
#include <breathe/math/cVec4.h>
#include <breathe/math/cMat4.h>
#include <breathe/math/cPlane.h>
#include <breathe/math/cQuaternion.h>
#include <breathe/math/cColour.h>
#include <breathe/math/cFrustum.h>
#include <breathe/math/cOctree.h>

#include <breathe/util/base.h>
#include <breathe/render/model/cMesh.h>
#include <breathe/render/model/cModel.h>
#include <breathe/render/model/cStatic.h>

#include <breathe/render/cTexture.h>
#include <breathe/render/cTextureAtlas.h>
#include <breathe/render/cMaterial.h>
#include <breathe/render/cRender.h>
#include <breathe/render/cFont.h>
#include <breathe/render/cParticleSystem.h>

#include <breathe/gui/cWidget.h>
#include <breathe/gui/cWindow.h>
#include <breathe/gui/cWindowManager.h>

#include <breathe/util/app.h>

#include "tetris.h"
#include "cGame.h"

#ifdef BUILD_DEBUG

#ifdef __WIN__

#define FILELISTMAXLEN 1024
#define FILELISTMAXFILES 40

#include <string.h>
#ifdef __WIN__
#include <windows.h>
#endif
#include <wchar.h>

class cFileEntry
{
public:
  breathe::string_t name;
	bool dir;

	void Set(const breathe::string_t& sFilename, bool _dir)
	{
    name = sFilename;
		dir=_dir;
	}
};

class cFileList
{
private:
	cFileEntry files[FILELISTMAXFILES];

	breathe::char_t lcase(breathe::char_t c)
	{
		if(c<TEXT('a')) return c+TEXT('a')-TEXT('A');

		return c;
	}

	int where(breathe::char_t c1, breathe::char_t c2)
	{
		if(lcase(c1)<lcase(c2)) return -1;
		else if(lcase(c1)>lcase(c2)) return 1;

		return 0;
	}

	bool preceeds(breathe::char_t* c1, breathe::char_t* c2)
	{
		int d1=wcslen(c1);
		int d2=wcslen(c2);
		int i=0;
		while(i<d1 && i<d2 && where(c1[i], c2[i])==0) i++;

		return (false == where(c1[i],c2[i])==1);
	}

	void Add(breathe::char_t * newname, bool dir)
	{
		cFileEntry temp;
		temp.Set(newname, dir);
		/*for (int i=0;i<n;i++)
			if (preceeds(temp.name, files[i].name))
			{
				for(int f=n;f>i;f--) files[f] = files[f-1];

				files[f]=temp;
				n++;
				return;
			}

		files[n]=temp;
		n++;*/
	}

public:

	std::vector<breathe::string_t> output_files;
	std::vector<breathe::string_t> output_directories;

	breathe::char_t * file(int i)
	{
		return &files[i].name[0];
	}

	bool dir(int i)
	{
		return files[i].dir;
	}

	void SearchDir(const breathe::string_t& nDir, breathe::string_t& File, bool recurse)
	{
		breathe::char_t Dir[FILELISTMAXLEN];
		if(!wcslen(nDir.c_str())) wsprintf(Dir, TEXT("."));
		else wcscpy(Dir, nDir.c_str());

		breathe::char_t Both[FILELISTMAXLEN] = TEXT("");

		// Find "nDir\File"
		wsprintf(Both, TEXT("%s\\%s"), Dir, File.c_str());
		WIN32_FIND_DATA fd;
		HANDLE hdl = FindFirstFile(Both, &fd);
		bool more=true;
		while((hdl != INVALID_HANDLE_VALUE) && more)
		{
			if (fd.cFileName[0] != TEXT('.')) {
				breathe::string_t path(fd.cFileName);
				output_files.push_back(nDir + TEXT("\\") + path);
			}

			more = FindNextFile(hdl, &fd) == TRUE;
		}
		FindClose(hdl);


		// Find "nDir\<folders>"
		wsprintf(Both, TEXT("%s\\*"), Dir);
		hdl = FindFirstFile(Both, &fd);
		more=true;
		while((hdl != INVALID_HANDLE_VALUE) && more)
		{
			if ((fd.cFileName[0] != TEXT('.')) && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				breathe::string_t path(fd.cFileName);
				output_directories.push_back(nDir + TEXT("\\") + path);
				if (recurse) {
					path = nDir;
					path += TEXT("\\") + breathe::string_t(fd.cFileName);
					SearchDir(path, File, recurse);
				}
			}

			more = FindNextFile(hdl, &fd) == TRUE;
		}
		FindClose(hdl);
	}
};

void AddEmptyLastLine(const breathe::string_t& filename)
{
	size_t bytes;
	if (breathe::storage::BYTEORDER_UTF8 != breathe::storage::DetectByteOrderMark(filename, bytes)) return;

	/*std::string contents;
	breathe::storage::ReadText(filename, contents);

	size_t n = contents.size();
	if (n > 2) {
		std::string lastTwo = contents.substr(n - 2);

		if (lastTwo == TEXT("\r\n")) return;
	}

	breathe::storage::AppendText(filename, "\r\n");*/
}

//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Jan 20 11:13:16 GMT-0800 1998
// Last Modified: Sun Jan  9 10:34:22 PST 2000 (minor changes)
// Last Modified: Thu Aug  2 13:52:52 PDT 2001 (added stdlib.h for new gcc)
// Last Modified  Tue Apr  9 12:24:27 PST 2002 (added Mac options)
// Last Modified  Mon Jan 20 21:12:52 PST 2003 (enabled conversion from mac to other)
// Last Modified  Wed May 18 14:03:03 PDT 2005 (updated for newer C++ compilers)
// Last Modified  Sun Aug 21 21:41:47 PDT 2005 (fixed so running -d twice works)
// Web Address:   http://www-ccrma.stanford.edu/~craig/utility/flip/flip.cpp
// Syntax:        C++
// $Smake:        g++ -ansi -O3 -o %b %f -static && strip %b
// $Smake-osx:    g++2 -ansi -O3 -o %b %f && strip %b
//
// Description: Utility program to convert text files between
//              UNIX or Mac newlines and DOS linefeed + newlines.
//
//              Unix uses the character 0x0a (decimal 10) to end a line
//              while DOS uses two characters: 0x0d 0x0a, where
//              (hex number 0x0d is equal to 13 decimal).
//              Mac OSes (prior to OS X), use 0x0d to end a line.
//              "Can't we just all get along?"
//
//              Options are:
//                 -t guess the OS format type of a text file.
//                    If the file is in mixed format, determine
//                    that the file is in DOS format.
//                 -u make the file Unix/MAC conformant
//                 -d make the file DOS/Window conformant
//                 -m make the file Macintosh (<10) conformant
//              Multiple files can be processed at once.
//
//              I wrote this program so that I could have the same
//              program for both Unix and MS Windows programs, but
//              an easy way to accomplish the same thing in Unix is
//              with using the sed command.  To make Unix formatted
//              newlines, run the command:
//                 sed 's/^M//' filename > temp; mv temp filename
//              where ^M is the "control-M" key or the return key
//              which is usually created on the command line by pressing
//              control-v and then the enter key.  Be careful not to write
//              the output of sed to the file you are modifying or you will
//              lose the file.  Likewise, you can go from Unix to DOS
//              formats by running the command:
//                 sed 's/$/^M/' filename > temp; mv temp filename
//
//              You can also use the PERL language interpreter
//              to convert the file contents:
//
//              Mac to Unix:
//                 perl -i -pe 's/\015/\012/g' mac-file
//              Mac to DOS:
//                 perl -i -pe 's/\015/\015\012/g' mac-file
//              Unix to Mac:
//                 perl -i -pe 's/\012/\015/g' unix-file
//              Unix to DOS:
//                 perl -i -pe 's/\012/\015\012/g' unix-file
//              DOS to Unix:
//                 perl -i -pe 's/\015\012/\012/g' mac-file
//              DOS to Mac:
//                 perl -i -pe 's/\015\012/\012/g' mac-file
//
//              Note 0x0a (hex) = \012 (octal) = 10 (decimal)
//              Note 0x0d (hex) = \015 (octal) = 13 (decimal)
//
//              But why would you want to remember all of that when you
//              can do this with the flip program:
//
//              Anything to Unix:
//                 flip -u file
//              Anything to DOS:
//                 flip -d file
//              Anything to Macintosh:
//                 flip -m file
//              To determine what the current line-flavor is:
//                 flip -t file
//
//              Note that the flip program is destructive.  It
//              overwrites the contents of the old file with the new
//              newline converted file.  Multiple files can be
//              converted at the same time with a wild card:
//                  flip -u *
//              This will flip newlines to the Unix style in all files
//              in the current directory.  You should not run the flip
//              program on binary data, because the 0x0a and 0x0d do
//              not necessarily mean newlines in binary data.
//

// define the following if you are compiling in MS-DOS/Windows 95/NT/98/2000
// #define MSDOS

#include <stdlib.h>

#ifdef OLDCPP
   #include <iostream.h>
   #include <fstream.h>
   #define SSTREAM strstream
   #define CSTRING str()
   #ifdef MSDOS
      #include <strstrea.h>
   #else
      #include <strstream.h>
   #endif
#else
   #include <iostream>
   #include <fstream>
   #define SSTREAM stringstream
   #define CSTRING str().c_str()
   using namespace std;
   #ifdef MSDOS
      #include <strstrea.h>
   #else
      #include <sstream>
   #endif
#endif


// function declarations
void exitUsage            (const char* command);
void translateToUnix      (const char* filename);
void translateToDos       (const char* filename);
void translateToMacintosh (const char* filename);
void determineType        (const char* filename);


///////////////////////////////////////////////////////////////////////////

/*int main(int argc, char* argv[]) {
   if (argc < 3) exitUsage(argv[0]);
   if (argv[1][0] != '-') exitUsage(argv[0]);
   char option = argv[1][1];
   if (!(option == 'u' || option == 'd' || option == 't'|| option == 'm')) {
      exitUsage(argv[0]);
   }

   for (int i=0; i<argc-2; i++) {
      if      (option == 'd') translateToDos(argv[i+2]);
      else if (option == 'u') translateToUnix(argv[i+2]);
      else if (option == 'm') translateToMacintosh(argv[i+2]);
      else determineType(argv[i+2]);
   }

   return 0;
}*/

///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// determineType
//

void determineType(const char* filename) {
   #ifdef OLDCPP
      #ifdef MSDOS
         fstream infile(filename, ios::in | ios::binary | ios::nocreate);
      #else
         fstream infile(filename, ios::in | ios::nocreate);
      #endif
   #else
      #ifdef MSDOS
         fstream infile(filename, ios::in | ios::binary | ios::nocreate);
      #else
         fstream infile(filename, ios::in);
      #endif
   #endif

   if (!infile) {
      cout << "Error: cannot find file " << filename << endl;
      return;
   }

   char ch;
   infile.get(ch);
   int d0count = 0;
   int a0count = 0;
   while (!infile.eof()) {
      if (ch == 0x0d) {
         d0count++;
      } else if (ch == 0x0a) {
         a0count++;
      }
      infile.get(ch);
   }

   infile.close();
   if ((a0count == d0count) && (d0count != 0)) {
      cout << filename << ": DOS" << endl;
   } else if ((a0count > 0) && (d0count == 0)) {
      cout << filename << ": UNIX" << endl;
   } else if ((a0count == 0) && (d0count > 0)) {
      cout << filename << ": MAC" << endl;
   } else if ((a0count > 0) && (d0count > 0)) {
      cout << filename << ": MIXED" << endl;
   } else {
      cout << filename << ": UNKNOWN" << endl;
   }
}



//////////////////////////////
//
// translateToDos
//

void translateToDos(const char* filename) {
   #ifdef OLDCPP
      #ifdef MSDOS
         fstream infile(filename, ios::in | ios::binary | ios::nocreate);
      #else
         fstream infile(filename, ios::in | ios::nocreate);
      #endif
   #else
      #ifdef MSDOS
         fstream infile(filename, ios::in | ios::binary | ios::nocreate);
      #else
         fstream infile(filename, ios::in);
      #endif
   #endif
   if (!infile) {
      cout << "Error: cannot find file: " << filename << endl;
      return;
   }

   SSTREAM outstring;
   char ch, lastch;
   infile.get(ch);
   lastch = ch;
   int peekch;
   while (!infile.eof()) {
      if (ch == 0x0a && lastch != 0x0d) {  // convert newline from Unix to MS-DOS
         outstring << (char)0x0d;
         outstring << ch;
         lastch = ch;
      } else if (ch == 0x0d) {             // convert newline from Mac to MS-DOS
         peekch = infile.peek();
         if (peekch != 0x0a) {
            outstring << ch;
            outstring << (char)0x0a;
            lastch = 0x0a;
         } else {
            lastch = 0x0d;
            // Bug fix here reported by Shelley Adams: running -d
            // twice in a row was generating Unix style newlines
            // without the following statement:
            outstring << (char)0x0d;
         }
      } else {
         outstring << ch;
         lastch = ch;
      }
      infile.get(ch);
   }

   infile.close();
   #ifdef MSDOS
      fstream outfile(filename, ios::out | ios::binary);
   #else
      fstream outfile(filename, ios::out);
   #endif
   if (!outfile) {
      cout << "Error: cannot write to file: " << filename << endl;
      return;
   }
   outstring << ends;
   outfile << outstring.CSTRING;
   outfile.close();
}



//////////////////////////////
//
// translateToMacintosh
//

void translateToMacintosh(const char* filename) {
   #ifdef OLDCPP
      #ifdef MSDOS
         fstream infile(filename, ios::in | ios::binary | ios::nocreate);
      #else
         fstream infile(filename, ios::in | ios::nocreate);
      #endif
   #else
      #ifdef MSDOS
         fstream infile(filename, ios::in | ios::binary | ios::nocreate);
      #else
         fstream infile(filename, ios::in);
      #endif
   #endif
   if (!infile) {
      cout << "Error: cannot find file: " << filename << endl;
      return;
   }

   SSTREAM outstring;
   char ch;
   infile.get(ch);
   int lastchar = '\0';
   while (!infile.eof()) {
      if (ch == 0x0a) {                        // convert newline from MSDOS to Mac
         if (lastchar == 0x0d) {
           // do nothing: already the newline was written
         } else {
            outstring << (char)0x0d;           // convert newline from Unix to Mac
         }
      } else {
         outstring << ch;
      }
      lastchar = ch;
      infile.get(ch);
   }

   infile.close();
   #ifdef MSDOS
      fstream outfile(filename, ios::out | ios::binary);
   #else
      fstream outfile(filename, ios::out);
   #endif
   if (!outfile.is_open()) {
      cout << "Error: cannot write to file: " << filename << endl;
      return;
   }
   outstring << ends;
   outfile << outstring.CSTRING;
   outfile.close();
}



//////////////////////////////
//
// translateToUnix
//

void translateToUnix(const char* filename) {
   #ifdef OLDCPP
      #ifdef MSDOS
         fstream infile(filename, ios::in | ios::binary | ios::nocreate);
      #else
         fstream infile(filename, ios::in | ios::nocreate);
      #endif
   #else
      #ifdef MSDOS
         fstream infile(filename, ios::in | ios::binary | ios::nocreate);
      #else
         fstream infile(filename, ios::in);
      #endif
   #endif
   if (!infile) {
      cout << "Error: cannot find file: " << filename << endl;
      return;
   }

   SSTREAM outstring;
   char ch, lastch;
   infile.get(ch);
   while (!infile.eof()) {
      if (ch == 0x0d) {
         outstring << (char)0x0a;
      } else if (ch == 0x0a) {
         if (lastch == 0x0d) {
            // do nothing: already converted MSDOS newline to Unix form
         } else {
            outstring << (char)0x0a;   // convert newline from Unix to Unix
         }
      } else {
         outstring << ch;
      }
      lastch = ch;
      infile.get(ch);
   }

   infile.close();
   #ifdef MSDOS
      fstream outfile(filename, ios::out | ios::binary);
   #else
      fstream outfile(filename, ios::out);
   #endif
   if (!outfile.is_open()) {
      cout << "Error: cannot write to file: " << filename << endl;
      return;
   }
   outstring << ends;
   outfile << outstring.CSTRING;
   outfile.close();
}

/*
#include <Box2D/Box2D.h>

// This is a simple example of building and running a simulation
// using Box2D. Here we create a large ground box and a small dynamic
// box.
int box2dtest()
{
	// Define the size of the world. Simulation will still work
	// if bodies reach the end of the world, but it will be slower.
	b2AABB worldAABB;
	worldAABB.minVertex.Set(-100.0f, -100.0f);
	worldAABB.maxVertex.Set(100.0f, 100.0f);

	// Define the gravity vector.
	b2Vec2 gravity(0.0f, -10.0f);

	// Do we want to let bodies sleep?
	bool doSleep = true;

	// Construct a world object, which will hold and simulate the rigid bodies.
	b2World world(worldAABB, gravity, doSleep);

	// Define the ground box shape.
	b2BoxDef groundBoxDef;

	// The extents are the half-widths of the box.
	groundBoxDef.extents.Set(50.0f, 10.0f);

	// Set the density of the ground box to zero. This will
	// make the ground body static (fixed).
	groundBoxDef.density = 0.0f;

	// Define the ground body.
	b2BodyDef groundBodyDef;
	groundBodyDef.position.Set(0.0f, -10.0f);

	// Part of a body's def is its list of shapes.
	groundBodyDef.AddShape(&groundBoxDef);

	// Call the body factory which allocates memory for the ground body
	// from a pool and creates the ground box shape (also from a pool).
	// The body is also added to the world.
	world.CreateBody(&groundBodyDef);

	// Define another box shape for our dynamic body.
	b2BoxDef boxDef;
	boxDef.extents.Set(1.0f, 1.0f);

	// Set the box density to be non-zero, so it will be dynamic.
	boxDef.density = 1.0f;

	// Override the default friction.
	boxDef.friction = 0.3f;

	// Define the dynamic body. We set its position,
	// add the box shape, and call the body factory.
	b2BodyDef bodyDef;
	bodyDef.position.Set(0.0f, 4.0f);
	bodyDef.AddShape(&boxDef);
	b2Body* body = world.CreateBody(&bodyDef);

	// Prepare for simulation. Typically we use a time step of 1/60 of a
	// second (60Hz) and 10 iterations. This provides a high quality simulation
	// in most game scenarios.
	float32 timeStep = 1.0f / 60.0f;
	int32 iterations = 10;

	// This is our little game loop.
	for (int32 i = 0; i < 60; ++i)
	{
		// Instruct the world to perform a single step of simulation. It is
		// generally best to keep the time step and iterations fixed.
		world.Step(timeStep, iterations);

		// Now print the position and rotation of the body.
		b2Vec2 position = body->GetOriginPosition();
		float32 rotation = body->GetRotation();

		printf("%4.2f %4.2f %4.2f\n", position.x, position.y, rotation);
	}

	// When the world destructor is called, all memory is freed. This can
	// create orphaned pointers, so be careful about your world management.

	return 0;
}*/
#endif //__WIN__

#endif //BUILD_DEBUG

int main(int argc, char** argv)
{
	/*cFileList filelist;
	breathe::string_t path(TEXT("H:\\dev\\chris\\library\\include\\breathe"));
	breathe::string_t filename(TEXT("*.h"));
	filelist.SearchDir(path, filename, true);

	std::vector<breathe::string_t>::iterator iter = filelist.output_directories.begin();
	std::vector<breathe::string_t>::iterator iterEnd = filelist.output_directories.end();

	while (iter != iterEnd) {
		std::cout<<"File:"<<breathe::string::ToUTF8(*iter)<<std::endl;
		iter++;
	}

	iter = filelist.output_files.begin();
	iterEnd = filelist.output_files.end();
	while (iter != iterEnd) {
		std::cout<<"File:"<<breathe::string::ToUTF8(*iter)<<std::endl;
		AddEmptyLastLine(*iter);
		iter++;
	}


	return 0;*/

  CONSOLE<<"main"<<std::endl;
	breathe::cApp* pApp = new cGame(argc, argv);

	LOG.Newline("Run");
  CONSOLE<<"Run"<<std::endl;
	bool bReturn = pApp->Run();

  CONSOLE<<"Delete App"<<std::endl;
	LOG.Newline("Delete pApp");
	breathe::SAFE_DELETE(pApp);

  CONSOLE<<"main returning "<<((breathe::GOOD == bReturn) ? "Success" : "Failed")<<std::endl;
	//Returns 1 if something went wrong, 0 if everything is good
	return (breathe::GOOD == bReturn) ? EXIT_SUCCESS : EXIT_FAILURE;
}
