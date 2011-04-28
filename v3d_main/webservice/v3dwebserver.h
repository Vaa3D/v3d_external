/**
 *	v3dwebserver.h
 *	developed by Yang Yu, Feb 16, 2011
 */

/*the service definitions*/

//gsoap ns service name:	v3dwebserver
//gsoap ns service namespace:	http://localhost/v3dwebserver.wsdl
//gsoap ns service location:	http://localhost:9125
//gsoap ns service style:	rpc
//gsoap ns service encoding:	encoded

//gsoap ns schema namespace: urn:v3dwebserver

class ns__LOAD_MSG
{
public:
	std::string image_path	1;	///< Required element.
	
	int x	1;
	int y	1;
	int z	1;
	int c	1;
	int t	1;
	
	float intensity	0;	///< Optional element
	
	int dt	1;

    struct soap *soap;	///< A handle to the soap struct that manages this instance
};

class ns__LOAD_RES
{
public:
	float loadtime	0;
	long memorysize	0;
	
	struct soap *soap;	///< soap handle
};

class ns__V3DMSG 
{
public:
	char*	imageName	1;	///< Required element.
	
	int xrot	1;
	int yrot	1;
	int zrot	1;
	
	struct soap *soap;	///< soap handle
	
};

class ns__V3DMSG_ROTATION 
{
public:
	char*	imageName	1;	///< Required element.
	
	int xrot	1;
	int yrot	1;
	int zrot	1;
	
	struct soap *soap;	///< soap handle
	
};

class ns__V3DMSG_ZOOM 
{
public:
	char*	imageName	1;	///< Required element.
	
	float zoom	1;
	
	struct soap *soap;	///< soap handle
	
};

class ns__V3DMSG_SHIFT 
{
public:
	char*	imageName	1;	///< Required element.
	
	float xshift	1;
	float yshift	1;
	float zshift	1;
	
	struct soap *soap;	///< soap handle
	
};

/* func hello world */
int ns__helloworld(
				   char *name,		///< Request parameter
				   char **response	///< Response parameter
);

/* func loading message */
int ns__msghandler(
				   ns__LOAD_MSG *lm,	///< Request parameter
				   ns__LOAD_RES *lr		///< Response parameter
);

/* func open a file in V3D */
int ns__v3dopenfile(char *fn, char **v3dfn);

/* func open a file in V3D with 3d viewer operations example func */
int ns__v3dopenfile3d(ns__V3DMSG *input, ns__V3DMSG *output);

/* func open a file in V3D and set 3dview rotation position */
int ns__v3dopenfile3dwrot(ns__V3DMSG_ROTATION *input, ns__V3DMSG_ROTATION *output);

/* func open a file in V3D and set 3dview zoom */
int ns__v3dopenfile3dwzoom(ns__V3DMSG_ZOOM *input, ns__V3DMSG_ZOOM *output);

/* func open a file in V3D and set 3dview shift position */
int ns__v3dopenfile3dwshift(ns__V3DMSG_SHIFT *input, ns__V3DMSG_SHIFT *output);

