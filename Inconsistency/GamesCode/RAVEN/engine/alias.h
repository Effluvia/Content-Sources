/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#ifndef ALIAS_H
#define ALIAS_H

// header
#define ALIAS_MODEL_VERSION	0x006
#define IDPOLYHEADER		MAKEID('I', 'D', 'P', 'O') // little-endian "IDPO"

#define	MAX_SKINS				32
#define MAX_LBM_HEIGHT			480
#define MAX_ALIAS_MODEL_VERTS	2000

// model flags
#define	AF_ROCKET	1			// leave a trail
#define	AF_GRENADE	2			// leave a trail
#define	AF_GIB		4			// leave a trail
#define	AF_ROTATE	8			// rotate (bonus items)
#define	AF_TRACER	16			// green split trail
#define	AF_ZOMGIB	32			// small blood trail
#define	AF_TRACER2	64			// orange split trail + rotate
#define	AF_TRACER3	128			// purple trail
#define AF_RELOADED	256			// texture was fixed and reloaded

typedef enum { ALIAS_SINGLE=0, ALIAS_GROUP } aliasframetype_t;

typedef enum { ALIAS_SKIN_SINGLE=0, ALIAS_SKIN_GROUP } aliasskintype_t;

typedef struct {
	byte	v[3];
	byte	lightnormalindex;
} trivertx_t;

typedef struct maliasframedesc_s
{
	int firstpose;
	int numposes;
	float interval;
	trivertx_t bboxmin;
	trivertx_t bboxmax;
	int frame;
	char name[16];
} maliasframedesc_t;

typedef struct maliasskindesc_s
{
	aliasskintype_t type;
	void			*pcachespot;
	int				skin;
} maliasskindesc_t;

typedef struct maliasgroupframedesc_s
{
	trivertx_t		bboxmin, bboxmax;
	int				frame;
} maliasgroupframedesc_t;

typedef struct maliasgroup_s
{
	int				numframes;
	int				intervals;
	maliasgroupframedesc_t frames[1];
} maliasgroup_t;

typedef struct maliasskingroup_s
{
	int				numskins;
	int				intervals;
	maliasskindesc_t skindescs[1];
} maliasskingroup_t;

typedef struct mtriangle_s
{
	int				facesfront;
	int				vertindex[3];
} mtriangle_t;

typedef struct {
	aliasskintype_t	type;
} daliasskintype_t;

typedef struct {
	int			numskins;
} daliasskingroup_t;

typedef struct {
	float	interval;
} daliasskininterval_t;

typedef struct {
	int					ident;
	int					version;
	vec3_t				scale;
	vec3_t				scale_origin;
	float				boundingradius;
	vec3_t				eyeposition;
	int					numskins;
	int					skinwidth;
	int					skinheight;
	int					numverts;
	int					numtris;
	int					numframes;
	synctype_t			synctype;
	int					flags;
	float				size;

	int					numposes;
	int					poseverts;
	int					posedata;	// numposes*poseverts trivert_t
	int					commands;	// gl command list with embedded s/t
	int					gl_texturenum[MAX_SKINS];
	maliasframedesc_t	frames[1];	// variable sized
} aliashdr_t;

typedef struct {
	int			ident;
	int			version;
	vec3_t		scale;
	vec3_t		scale_origin;
	float		boundingradius;
	vec3_t		eyeposition;
	int			numskins;
	int			skinwidth;
	int			skinheight;
	int			numverts;
	int			numtris;
	int			numframes;
	synctype_t	synctype;
	int			flags;
	float		size;
} mdl_t;

#endif