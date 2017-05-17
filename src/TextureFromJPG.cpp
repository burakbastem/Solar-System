
#include <stdio.h>
#include <GL/glew.h>
#include "TextureFromJPG.h"

CTexture::CTexture()
{
	
}

CTexture::~CTexture()
{
	//glGenTextures(1, &mTexture);
	if (textureID!=NULL)
	{
		glDeleteTextures(1,textureID);
		delete textureID;
	}
}

bool CTexture::FileExist(char * mFileName)
{
	FILE *File = NULL;									// File Handle

	if (!mFileName)										// Make Sure A Filename Was Given
	{	return false; 	}								// No file specified
	
	//open the file to see if it does exist
	File = fopen(mFileName,"r");							// Check To See If The File Exists
	if (!File)											// If the file does not exist then exit
	{	return false;	}								
	
	fclose(File);										// Close The Handle

	//return success = file exists
	return true;
}

bool CTexture::loadTexture2D(char* mFileName , GLuint &mTexture,bool anisoVal)
{
	if (!FileExist(mFileName))							//check to see if the file exists
	{	
		printf("Failed!!! - File does not exist\n");   
		return false;	
	}	
	else
	{
		printf("Loading Texture :  %s......",mFileName);
	}

	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP* dib(0);

	fif = FreeImage_GetFileType(mFileName, 0); // Check the file signature and deduce its format

	if(fif == FIF_UNKNOWN) // If still unknown, try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilename(mFileName);
	
	if(fif == FIF_UNKNOWN) // If still unkown, return failure
		return false;

	if(FreeImage_FIFSupportsReading(fif)) // Check if the plugin has reading capabilities and load the file
		dib = FreeImage_Load(fif, mFileName);
	if(!dib)
		return false;

	BYTE* bDataPointer = FreeImage_GetBits(dib); // Retrieve the image data
	
	iWidth = FreeImage_GetWidth(dib); // Get the image width and height
	iHeight = FreeImage_GetHeight(dib);
	iBPP = FreeImage_GetBPP(dib);

	// If somehow one of these failed (they shouldn't), return failure
	if(bDataPointer == NULL || iWidth == 0 || iHeight == 0)
		return false;

	// Generate an OpenGL texture ID for this texture
	textureID=new GLuint;
	*textureID=mTexture;
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	if(anisoVal)
	{
		GLfloat fLargest;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,&fLargest);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
		printf("AnisoTropic extension supported from JPG Load File\n");
	}
	else
	{
		printf("NO ANISOTROPIC  EXTENSION SUPPORTED \n");
	}

	int iFormat = iBPP == 24 ? GL_BGR : iBPP == 8 ? GL_LUMINANCE : 0;
	int iInternalFormat = iBPP == 24 ? GL_RGB : GL_DEPTH_COMPONENT; 

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, iFormat, GL_UNSIGNED_BYTE, bDataPointer);
	glGenerateMipmap(GL_TEXTURE_2D);

	printf("File Loaded into Texture SuccessFully....\n");
	

	FreeImage_Unload(dib);
	return true;
}
