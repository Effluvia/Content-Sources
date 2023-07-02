/*++

   Copyright    (c)    1995-1996    Microsoft Corporation

   Module  Name:

      ilogobj.hxx

   Abstract:

      Logging interfaces

   Author:

       Terence Kwan    ( terryk )    18-June-1996


--*/

# ifndef _ILOGOBJ_HXX_
# define _ILOGOBJ_HXX_

//
// strings
//

#define NCSALOG_CLSID          TEXT("{FF16065F-DE82-11CF-BC0A-00AA006111E0}")
#define ODBCLOG_CLSID          TEXT("{FF16065B-DE82-11CF-BC0A-00AA006111E0}")
#define ASCLOG_CLSID           TEXT("{FF160657-DE82-11CF-BC0A-00AA006111E0}")
#define EXTLOG_CLSID           TEXT("{FF160663-DE82-11CF-BC0A-00AA006111E0}")

#define NCSALOGUI_CLSID        TEXT("{31DCAB85-BB3E-11d0-9299-00C04FB6678B}")
#define ODBCLOGUI_CLSID        TEXT("{31DCAB86-BB3E-11d0-9299-00C04FB6678B}")
#define ASCLOGUI_CLSID         TEXT("{31DCAB87-BB3E-11d0-9299-00C04FB6678B}")
#define EXTLOGUI_CLSID         TEXT("{31DCAB88-BB3E-11d0-9299-00C04FB6678B}")

//
// GUIDS
//

DEFINE_GUID(CLSID_InetLogInformation,
    0xa1f89741, 0xf619, 0x11cf, 0xbc, 0xf, 0x0, 0xaa, 0x0, 0x61, 0x11, 0xe0);

DEFINE_GUID(IID_IINETLOG_INFORMATION, /* cc557a71-f61a-11cf-bc0f-00aa006111e0 */
    0xcc557a71,0xf61a,0x11cf,0xbc, 0x0f, 0x00, 0xaa, 0x00, 0x61, 0x11, 0xe0);

DEFINE_GUID(IID_ICLAPI_CLIENT, /* 08fd99d1-cfb6-11cf-bc03-00aa006111e0 */
    0x08fd99d1,0xcfb6,0x11cf,0xbc, 0x03, 0x00, 0xaa, 0x00, 0x61, 0x11, 0xe0);

DEFINE_GUID(IID_ILogPlugin, /* 08fd99d1-cfb6-11cf-bc03-00aa006111e0 */
    0x08fd99d1,0xcfb6,0x11cf,0xbc, 0x03, 0x00, 0xaa, 0x00, 0x61, 0x11, 0xe0);

DEFINE_GUID(CLSID_NCSALOG, /* ff16065F-DE82-11cf-bc0a-00aa006111e0 */
    0xff16065F,0xde82,0x11cf,0xbc, 0x0a, 0x00, 0xaa, 0x00, 0x61, 0x11, 0xe0);

DEFINE_GUID(CLSID_ODBCLOG, /* ff16065B-DE82-11cf-bc0a-00aa006111e0 */
    0xff16065B,0xde82,0x11cf,0xbc, 0x0a, 0x00, 0xaa, 0x00, 0x61, 0x11, 0xe0);

DEFINE_GUID(CLSID_ASCLOG, /* ff160657-DE82-11cf-bc0a-00aa006111e0 */
    0xff160657,0xde82,0x11cf,0xbc, 0x0a, 0x00, 0xaa, 0x00, 0x61, 0x11, 0xe0);

DEFINE_GUID(CLSID_EXTLOG, /* ff160663-DE82-11cf-bc0a-00aa006111e0 */
    0xff160663,0xde82,0x11cf,0xbc, 0x0a, 0x00, 0xaa, 0x00, 0x61, 0x11, 0xe0);

// logging ui guids
//IID_LOGGINGUI
DEFINE_GUID(IID_LOGGINGUI, /* 31DCAB89-BB3E-11d0-9299-00C04FB6678B */
    0x31dcab89, 0xbb3e, 0x11d0, 0x92, 0x99, 0x0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b);

//NCSALOGUI_CLSID
DEFINE_GUID(CLSID_NCSALOGUI, /* 31DCAB85-BB3E-11d0-9299-00C04FB6678B */
    0x31dcab85, 0xbb3e, 0x11d0, 0x92, 0x99, 0x0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b);

//ODBCLOGUI_CLSID
DEFINE_GUID(CLSID_ODBCLOGUI, /* 31DCAB86-BB3E-11d0-9299-00C04FB6678B */
    0x31dcab86, 0xbb3e, 0x11d0, 0x92, 0x99, 0x0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b);

//ASCLOGUI_CLSID
DEFINE_GUID(CLSID_ASCLOGUI, /* 31DCAB87-BB3E-11d0-9299-00C04FB6678B */
    0x31dcab87, 0xbb3e, 0x11d0, 0x92, 0x99, 0x0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b);

//EXTLOGUI_CLSID
DEFINE_GUID(CLSID_EXTLOGUI, /* 31DCAB88-BB3E-11d0-9299-00C04FB6678B */
    0x31dcab88, 0xbb3e, 0x11d0, 0x92, 0x99, 0x0, 0xc0, 0x4f, 0xb6, 0x67, 0x8b);


//
// Interface class for CINETLOG
//

class IInetLogInformation : public IUnknown {

    public:
        virtual LPSTR STDMETHODCALLTYPE
        GetSiteName(
            IN PCHAR    pszSiteName,
            IN PDWORD   pcbSize
            ) = 0;

        virtual LPSTR STDMETHODCALLTYPE
        GetComputerName(
            IN PCHAR    pszComputerName,
            IN PDWORD   pcbSize
            ) = 0;

        virtual LPSTR STDMETHODCALLTYPE
        GetClientHostName(
            IN PCHAR    pszClientHostName,
            IN PDWORD   pcbSize
            ) = 0;

        virtual LPSTR STDMETHODCALLTYPE
        GetClientUserName(
            IN PCHAR    pszClientUserName,
            IN PDWORD   pcbSize
            ) = 0;

        virtual LPSTR STDMETHODCALLTYPE
        GetServerAddress(
            IN PCHAR    pszServerIPAddress,
            IN PDWORD   pcbSize
            ) = 0;

        virtual LPSTR STDMETHODCALLTYPE
        GetOperation(
            IN PCHAR    pszOperation,
            IN PDWORD   pcbSize
            ) = 0;

        virtual LPSTR STDMETHODCALLTYPE
        GetTarget(
            IN PCHAR    pszTarget,
            IN PDWORD   pcbSize
            ) = 0;

        virtual LPSTR STDMETHODCALLTYPE
        GetParameters(
            IN PCHAR    pszParameters,
            IN PDWORD   pcbSize
            ) = 0;

        virtual LPSTR STDMETHODCALLTYPE
        GetExtraHTTPHeaders(
            IN PCHAR    pszHTTPHeaders,
            IN PDWORD   pcbSize
            ) = 0;

        virtual DWORD STDMETHODCALLTYPE
        GetTimeForProcessing(
            VOID
            ) = 0;

        virtual DWORD STDMETHODCALLTYPE
        GetBytesSent(
            VOID
            ) = 0;

        virtual DWORD STDMETHODCALLTYPE
        GetBytesRecvd(
            VOID
            ) = 0;

        virtual DWORD STDMETHODCALLTYPE
        GetWin32Status(
            VOID
            ) = 0;

        virtual DWORD STDMETHODCALLTYPE
        GetProtocolStatus(
            VOID
            ) = 0;

        virtual DWORD STDMETHODCALLTYPE
        GetPortNumber(
            VOID
            ) = 0;

        virtual LPSTR STDMETHODCALLTYPE
        GetVersionString(
            IN PCHAR    pszVersionString,
            IN PDWORD   pcbSize
            ) = 0;


};


//
// CLAPI plugin Interface
//

class ILogPlugin  : public IUnknown
{
    public:

    virtual HRESULT STDMETHODCALLTYPE
    InitializeLog(
        IN LPCSTR SiteName,
        IN LPCSTR MetabasePath,
        IN PCHAR pvIMDCOM ) = 0;

    virtual HRESULT STDMETHODCALLTYPE
    TerminateLog( VOID ) = 0;

    virtual HRESULT STDMETHODCALLTYPE
    LogInformation( IInetLogInformation *pLogObj ) = 0;

    virtual HRESULT STDMETHODCALLTYPE
    SetConfig( IN DWORD cbSize, PBYTE Log ) = 0;

    virtual HRESULT STDMETHODCALLTYPE
    GetConfig( IN DWORD cbSize, PBYTE Log ) = 0;

    virtual HRESULT STDMETHODCALLTYPE
    QueryExtraLoggingFields(PDWORD cbSize, PCHAR szParameters) = 0;
};

//
// logging plugin UI Interface
//

class ILogUIPlugin  : public IUnknown
{
    public:
    virtual HRESULT STDMETHODCALLTYPE
    OnProperties( IN OLECHAR* pocMachineName, IN OLECHAR* pocMetabasePath ) = 0;
};

# endif // _ILOGOBJ_HXX_
