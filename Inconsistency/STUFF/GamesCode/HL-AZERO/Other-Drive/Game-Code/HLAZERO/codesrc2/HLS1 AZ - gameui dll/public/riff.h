//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================

#ifndef RIFF_H
#define RIFF_H
#pragma once

#include "commonmacros.h"


//-----------------------------------------------------------------------------
// Purpose: This is a simple abstraction that the RIFF classes use to read from
//			files/memory
//-----------------------------------------------------------------------------
class IFileReadBinary
{
public:
	virtual int open( const char *pFileName ) = 0;
	virtual int read( void *pOutput, int size, int file ) = 0;
	virtual void close( int file ) = 0;
	virtual void seek( int file, int pos ) = 0;
	virtual unsigned int tell( int file ) = 0;
	virtual unsigned int size( int file ) = 0;
};


//-----------------------------------------------------------------------------
// Purpose: Used to read/parse a RIFF format file
//-----------------------------------------------------------------------------
class InFileRIFF
{
public:
	InFileRIFF( const char *pFileName, IFileReadBinary &io );
	~InFileRIFF( void );

	unsigned int RIFFName( void ) { return m_riffName; }
	unsigned int RIFFSize( void ) { return m_riffSize; }

	int		ReadInt( void );
	int		ReadData( void *pOutput, int dataSize );
	int		PositionGet( void );
	void	PositionSet( int position );
	bool	IsValid( void ) { return m_file != 0; }

private:
	const InFileRIFF & operator=( const InFileRIFF & );

	IFileReadBinary		&m_io;
	int					m_file;
	unsigned int		m_riffName;
	unsigned int		m_riffSize;
};


//-----------------------------------------------------------------------------
// Purpose: Used to iterate over an InFileRIFF
//-----------------------------------------------------------------------------
class IterateRIFF
{
public:
	IterateRIFF( InFileRIFF &riff, int size );
	IterateRIFF( IterateRIFF &parent );

	bool			ChunkAvailable( void );
	bool			ChunkNext( void );

	unsigned int	ChunkName( void );
	unsigned int	ChunkSize( void );
	int				ChunkRead( void *pOutput );
	int				ChunkReadPartial( void *pOutput, int dataSize );
	int				ChunkReadInt( void );
	int				ChunkFilePosition( void ) { return m_chunkPosition; }

private:
	const IterateRIFF & operator=( const IterateRIFF & );

	void	ChunkSetup( void );
	void	ChunkClear( void );

	InFileRIFF			&m_riff;
	int					m_start;
	int					m_size;

	unsigned int		m_chunkName;
	int					m_chunkSize;
	int					m_chunkPosition;
};

#define RIFF_WAVE			MAKEID('W','A','V','E')
#define WAVE_FMT			MAKEID('f','m','t',' ')
#define WAVE_DATA			MAKEID('d','a','t','a')
#define WAVE_FACT			MAKEID('f','a','c','t')
#define WAVE_CUE			MAKEID('c','u','e',' ')
#define WAVE_SAMPLER		MAKEID('s','m','p','l')
#define WAVE_VALVEDATA		MAKEID('V','D','A','T')

#endif // RIFF_H
