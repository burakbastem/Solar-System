#ifndef _TEXTURE_FROM_JPG__
#define _TEXTURE_FROM_JPG__

#include "FreeImage.h"
//#include <GL/gl.h>

//#include "load_texture.h"

class CTexture
{
	public:
		CTexture();
		~CTexture();

		bool loadTexture2D(char* mFileName , GLuint &mTexture,bool anisoVal=false);
		
		/*void bindTexture(int iTextureUnit = 0);
		void setFiltering(int a_tfMagnification, int a_tfMinification);
		int getMinificationFilter();
		int getMagnificationFilter();
		void releaseTexture();*/

	
	private:
		int iWidth, iHeight, iBPP; // Texture width, height, and bytes per pixel
		int uiTexture;			   // Texture name
		GLuint *textureID;

	public:
		bool FileExist(char *mFileName);	// This checks to make sure the file exists

	
};


#endif