class KeyValues;

struct ClosedCaptionInfo_t
{
public:
	ClosedCaptionInfo_t() :
		icon( NULL ),
		attenuation( 5 ),
		flags( 0 )
	{
	}
	~ClosedCaptionInfo_t()
	{
		delete[] icon;
	}
	char *icon;
	Color clr;
	unsigned char attenuation;
	enum
	{
		CCI_COLOR, // color was specified, doesn't override <clr> in caption text
		CCI_IMPORTANT, // don't let this caption get pushed offscreen by other captions
		CCI_EXCLUSIVE, // don't show other captions while this one is onscreen
		CCI_NOSFX, // don't show <sfx> captions while this one is onscreen
	};
	unsigned char flags;
};

class CClosedCaptionsInfo
{
public:
	void Init( void );
	const ClosedCaptionInfo_t *GetCaptionInfo( const char *token );
	void Reload( void );
private:
	void ReadCaptionsInfoFile( void );
	void ReadColors( KeyValues *pkv );
	void ReadCaption( KeyValues *pkv );

	CUtlDict<ClosedCaptionInfo_t *, int> m_CaptionsInfo;
	CUtlDict<Color, int> m_Colors;
};

extern CClosedCaptionsInfo *closedcaptionsinfo;