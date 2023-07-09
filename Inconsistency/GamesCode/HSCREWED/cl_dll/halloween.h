//
//-----------------------------------------------------
//
class CHudHalloween: public CHudBase
{
public:
	virtual int Init( void );
	virtual int VidInit( void );
	virtual int Draw(float fTime);
	virtual void Reset( void );
	int MsgFunc_Halloween(const char *pszName,  int iSize, void *pbuf);
	int insanity;
	char * billymc [32];
private:
	char * m_cChatter;
};	
