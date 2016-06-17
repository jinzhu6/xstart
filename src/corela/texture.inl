
TEXTURE* TextureCreate(coDword width, coDword height, coDword flags)
{
	BEGIN_CHECK_GL_ERROR();

	// Create texture object
	TEXTURE* tx = (TEXTURE*)malloc(sizeof(TEXTURE));
	memset(tx, 0, sizeof(TEXTURE));

	// ...
	tx->target = GL_TEXTURE_2D;
	tx->flags = flags;
	tx->vwidth = width;
	tx->vheight = height;
	tx->width = width = _GLCaps_CompatTexSize(width);
	tx->height = height = _GLCaps_CompatTexSize(height);

	// ...
	tx->drawRect.left = 0.0;
	tx->drawRect.right = 1.0;
	tx->drawRect.top = 0.0;
	tx->drawRect.bottom = 1.0;

	// Use texture compression if requested and supported
	if(flags & TEX_COMPRESS && _GLCaps_HasExtension("ARB_texture_compression"))
	{
		if(TEX_ALPHA)
			tx->internalFormat = GL_COMPRESSED_RGBA;
		else
			tx->internalFormat = GL_COMPRESSED_RGB;
	}
	else
	{
		if(TEX_ALPHA)
			tx->internalFormat = GL_RGBA;
		else
			tx->internalFormat = GL_RGB;
	}

	// OpenGL: Generate texture and set parameters
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &tx->handle);
	glBindTexture(tx->target, tx->handle);

	// Set texture filter/mipmap filter
	if(!(flags & TEX_NOMIPMAP))
	{
		if(flags & TEX_NOFILTER)
		{
			glTexParameteri(tx->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(tx->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else
		{
			glTexParameteri(tx->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(tx->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			if(anisotropy) glTexParameteri(tx->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
		}
	}
	else
	{
		if(flags & TEX_NOFILTER)
		{
			glTexParameteri(tx->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(tx->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else
		{
			glTexParameteri(tx->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(tx->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			if(anisotropy) glTexParameteri(tx->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
		}
	}

	// Set texture lod bias
	if(flags & TEX_SHARP && _GLCaps_HasExtension("EXT_texture_lod_bias"))
		glTexParameterf(tx->target, GL_TEXTURE_LOD_BIAS, -0.5f);

	// Set texture clamping
	if(flags & TEX_CLAMP)
	{
		glTexParameteri(tx->target, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(tx->target, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}

	//
	if(flags & TEX_CLEAR)
	{
		TextureClear(tx);
	}

	END_CHECK_GL_ERROR("TextureCreate");

	return tx;
}

void TextureDestroy(TEXTURE* tex)
{
	if(tex->pLockMem) free(tex->pLockMem);
	glDeleteTextures(1, &tex->handle);
	free(tex);
}

void TextureClear(TEXTURE* tex, coDword color)
{
	// TODO: Optimize
	IMAGE* im = ImageCreate(tex->width, tex->height);
	TextureGetImage(tex, im);
	ImageFill(im, color);
	TextureSetImage(tex, im);
	ImageDestroy(im);
}

TEXTURE* TextureLoad(cstring file, coDword flags)
{
	BEGIN_CHECK_GL_ERROR();

	IMAGE* image = ImageLoad(file);
	TEXTURE* tx = TextureCreate(image->width, image->height, flags);

	glTexImage2D(GL_TEXTURE_2D, 0, tx->internalFormat, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);

	if( !(flags & TEX_NOMIPMAP) && glGenerateMipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	END_CHECK_GL_ERROR("TextureLoad");

	return tx;
}

coByte* TextureLock(TEXTURE* tex, coBool read, int format)
{
	BEGIN_CHECK_GL_ERROR();

	if(tex->pLockMem) free(tex->pLockMem);
	tex->pLockMem = (coByte*)malloc(tex->width * tex->height * 4);
	
	if(read)
	{
		glBindTexture(tex->target, tex->handle);
		glGetTexImage(tex->target, 0, format, GL_UNSIGNED_BYTE, tex->pLockMem);
	}
	
	END_CHECK_GL_ERROR("TextureLock");

	return tex->pLockMem;
}

void TextureUnlock(TEXTURE* tex, coBool write, int format)
{
	if(write)
		TextureUpload(tex, tex->pLockMem, format);
	free(tex->pLockMem);
	tex->pLockMem = 0;
}

void TextureUpload(TEXTURE* tex, coByte* data, int format)
{
	// TODO: Optimize uploads via PBOs.

	BEGIN_CHECK_GL_ERROR();

	glEnable(tex->target);
	glBindTexture(tex->target, tex->handle);
	glTexImage2D(tex->target, 0, tex->internalFormat, tex->width, tex->height, 0, format, GL_UNSIGNED_BYTE, data);
	if(!(tex->flags & TEX_NOMIPMAP) && glGenerateMipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	END_CHECK_GL_ERROR("TextureUpload");
}

void TextureGetImage(TEXTURE* tex, IMAGE* im, int format)
{
	// resize image if neccessary
	if(im->width != tex->width || im->height != tex->height)
		ImageResize(im, tex->width, tex->height);
	
	// download texture to image
	glEnable(tex->target);
	glBindTexture(tex->target, tex->handle);
	glGetTexImage(tex->target, 0, format, GL_UNSIGNED_BYTE, im->data);
}

void TextureSetImage(TEXTURE* tex, IMAGE* im, int format)
{
	// get compatible texture size
	int w = _GLCaps_CompatTexSize(im->width);
	int h = _GLCaps_CompatTexSize(im->height);

	tex->width = w;
	tex->height = h;
	//tex->vwidth = im->width;
	//tex->vheight = im->height;

	// upload image to texture
	TextureUpload(tex, (coByte*)im->data, format);
}

