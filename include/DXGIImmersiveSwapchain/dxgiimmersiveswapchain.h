#pragma once
#include "ImmersiveWindow/framework.h"

#define RECTTOLOGICAL(rcPhysical)                         \
        { ((rcPhysical).right  = RECTWIDTH(rcPhysical));  \
          ((rcPhysical).bottom = RECTHEIGHT(rcPhysical)); \
          ((rcPhysical).left   = 0);                      \
          ((rcPhysical).top    = 0);  }   

#define RECTTORECTF(rc, rcf)                  \
        { ((rcf).left   = (FLOAT)rc.left);    \
          ((rcf).top    = (FLOAT)rc.top);     \
          ((rcf).right  = (FLOAT)rc.right);   \
          ((rcf).bottom = (FLOAT)rc.bottom); }

#define D2D_RGB_FROM_COLORREF(cr, alpha) \
    { GetRValue(cr) / 255.0F, GetGValue(cr) / 255.0F, GetBValue(cr) / 255.0F, CLAMP(alpha, 0.0F, 1.0F) }

#define FONT_SEGOE_UI_SYMBOL
#define FONT_SEGOE_MDL_ASSETS
#define FONT_SEGOE_FLUENT_ICONS
//segoe ui variable 
//light 263c
//segoe ui symbol
//light 263c
//dark e28c
#define CP_SYM_LIGHTMODE          (0x263C)
//#define CP_SYM_LIGHTMODE          (0xE707)
#define CP_SYM_DARKMODE           (0xE28C)
#define CP_SYM_LIGHTMODE2         (0xE706)
#define CP_MDL2_SETTINGS1         (0xE115)
#define CP_MDL2_SETTINGS2         (0xE713)
#define CP_MDL2_SETTINGS3         (0xF8B0)
#define CP_MDL2_LIGHTMODE1        (0xE706)
#define CP_MDL2_LIGHTMODE2        (0xED39)
#define CP_MDL2_DARKMODE1         (0xE708)
#define CP_MDL2_DARKMODE2         (0xEC46)
#define CP_MDL2_DARKMODE3         (0xF0CE)
#define CP_MDL2_LIGHTDARK1        (0xE793)
#define CP_MDL2_LIGHTDARK2        (0xF08C)
#define CP_MDL2_MINIMIZE          (0xE921)
#define CP_MDL2_MAXIMIZE          (0xE922)
#define CP_MDL2_RESTORE           (0xE923)
#define CP_MDL2_CLOSE             (0xE947)
#define CP_MDL2_FEEDBACK          (0xED15)

#define GLYPH_INDEX_MINIMIZE (442)
#define GLYPH_INDEX_MAXIMIZE (443)
#define GLYPH_INDEX_RESTORE  (12)
#define GLYPH_INDEX_CLOSE    (13)
#define GLYPH_INDEX_LIGHT    (3)
#define GLYPH_INDEX_DARK     (5)

#define D3DKMT_PTR(Type, Name) Type Name
typedef LONG NTSTATUS;
typedef UINT D3DKMT_HANDLE;
typedef UINT D3DDDI_VIDEO_PRESENT_SOURCE_ID;
typedef struct _D3DKMT_OPENADAPTERFROMHDC
{
  D3DKMT_PTR(HDC, hDc);           // in:  DC that maps to a single display
  D3DKMT_HANDLE                   hAdapter;       // out: adapter handle
  LUID                            AdapterLuid;    // out: adapter LUID
  D3DDDI_VIDEO_PRESENT_SOURCE_ID  VidPnSourceId;  // out: VidPN source ID for that particular display
} D3DKMT_OPENADAPTERFROMHDC;
typedef struct _D3DKMT_WAITFORVERTICALBLANKEVENT
{
  D3DKMT_HANDLE                   hAdapter;      // in: adapter handle
  D3DKMT_HANDLE                   hDevice;       // in: device handle [Optional]
  D3DDDI_VIDEO_PRESENT_SOURCE_ID  VidPnSourceId; // in: adapter's VidPN Source ID
} D3DKMT_WAITFORVERTICALBLANKEVENT;
typedef struct _D3DKMT_GETSCANLINE
{
  D3DKMT_HANDLE                   hAdapter;           // in: Adapter handle
  D3DDDI_VIDEO_PRESENT_SOURCE_ID  VidPnSourceId;      // in: Adapter's VidPN Source ID
  BOOLEAN                         InVerticalBlank;    // out: Within vertical blank
  UINT                            ScanLine;           // out: Current scan line
} D3DKMT_GETSCANLINE;
typedef struct _D3DKMT_CLOSEADAPTER
{
  D3DKMT_HANDLE   hAdapter;   // in: adapter handle
} D3DKMT_CLOSEADAPTER;

EXTERN_C _Check_return_ NTSTATUS APIENTRY D3DKMTOpenAdapterFromHdc(_Inout_ D3DKMT_OPENADAPTERFROMHDC*);
EXTERN_C _Check_return_ NTSTATUS APIENTRY D3DKMTWaitForVerticalBlankEvent(_In_ CONST D3DKMT_WAITFORVERTICALBLANKEVENT*);
EXTERN_C _Check_return_ NTSTATUS APIENTRY D3DKMTGetScanLine(_Inout_ D3DKMT_GETSCANLINE*);

typedef struct tagGDICANVAS {
    HDC              hMemDC;
    HBITMAP          hBitmap;
    HBITMAP          hOldBitmap;
    BITMAPINFOHEADER bmih;
    BITMAPINFOHEADER bmihBlit;

} GDICANVAS, *LPGDICANVAS;

typedef struct tagCHILDMETRICS {
    HWND         hwnd;
    int          id;
    POINT        ptOffset;
    SIZE         size;
    HDC          hMemDC;
    HDC          hMemDIB;
    HDC          hMemBitmapOld;
    LPVOID       pMemBits;
    ID2D1Bitmap* pD2D1Bitmap;
    BITMAPINFO   bmi;
} CHILDMETRICS;

typedef void(*proc)(HWND hwnd, void* pUserData);

typedef enum DXGI_IMMERSIVE_SWAPCHAIN_CREATE_FLAGS
{
    DXGI_IMMERSIVE_SWAPCHAIN_CREATE_FLAGS_NONE = 0,
    DXGI_IMMERSIVE_SWAPCHAIN_CREATE_FLAGS_DEBUG = 1 << 0,
    DXGI_IMMERSIVE_SWAPCHAIN_CREATE_FLAGS_SINGLETHREADED = 1 << 1,
    DXGI_IMMERSIVE_SWAPCHAIN_CREATE_FLAGS_USE_FRAME_LATENCY_WAITABLE_OBJECT = 1 << 2,

} DXGI_IMMERSIVE_SWAPCHAIN_CREATE_FLAGS;

typedef struct DXGI_IMMERSIVE_SWAPCHAIN_CREATE_INFO
{
    UINT Width;
    UINT Height;
    HWND hwnd;
    proc pProc;
} DXGI_IMMERSIVE_SWAPCHAIN_CREATE_INFO, * PDXGI_IMMERSIVE_SWAPCHAIN_CREATE_INFO;

DEFINE_GUID(GUID_Impl_IDCompositionDevice_ImmersiveDevice, 0xC37EA93A, 0xE7AA, 0x450D, 0xB1, 0x6F, 0x97, 0x46, 0xCB, 0x04, 0x07, 0xF3);
typedef struct tagDXGIImmersiveSwapchain {
    ID3D11Device*         pD3D11Device;
    ID3D11DeviceContext*  pD3D11DeviceContext;
    IDXGIAdapter*         pDXGIAdapter;
    IDXGISwapChain2*      pDXGISwapchain2;
    ID2D1DeviceContext*   pD2D1DeviceContext;
    ID2D1SolidColorBrush* pD2D1Brush;
    ID2D1SolidColorBrush* pD2D1RedBrush;
    ID2D1SolidColorBrush* pD2D1BlackBrush;
    ID2D1SolidColorBrush* pD2D1InactiveBrush;
    ID2D1SolidColorBrush* pD2D1ColorizationColorBrush;
    ID2D1SolidColorBrush* pD2D1FadedColorizationColorBrush;
    ID2D1SolidColorBrush* pD2D1WindowFrameColorBrush;
    ID2D1Factory2*        pD2D1Factory2;
    IDCompositionDevice*  pDCompDevice;
    IDCompositionVisual*  pDCompVisual;
    IDCompositionTarget*  pDCompTarget;
    IDXGISurface2*        pDXGISurface2;
    IDXGIFactory2*        pDXGIFactory2;
    IDXGIDevice3*         pDXGIDevice3;
    ID2D1HwndRenderTarget*    pD2D1HwndRenderTarget;
    ID2D1Bitmap*          pD2D1Bitmap;
    ID2D1Device1*         pD2D1Device;
    ID2D1Effect*          pD2D1EffectBlur;
    ID2D1Bitmap*          pD2D1BitmapBlur;
    HANDLE                hFrameLatencyWaitableObject;
    ID2D1StrokeStyle*     pD2D1StrokeStyle;
    proc pProc;
#ifndef NDEBUG
    ID3D11Debug*          pD3D11Debug;
#endif
    IDWriteFactory1*        pDWriteFactory1;
    IDWriteFontCollection1* pFontCollection;
    IDWriteFontFamily1*     pFontFamily;
    IDWriteFont1*           pFont;
    IDWriteFontFace1*       pFontFace;
    IDWriteTextFormat*      pTextFormat;
    IDWriteTextFormat*      pTextFormat2;

    UINT32                  pCodePointsMDL2[15];
    UINT16                  pGlyphIndicesMDL2[15];

    IDWriteFontFamily1*     pFontFamily2;
    IDWriteFont1*           pFont2;
    IDWriteFontFace1*       pFontFace2;
    UINT32                  pCodePointsMDL22[3];
    UINT16                  pGlyphIndicesMDL22[3];

    ID2D1Bitmap*            pD2D1GDIBitmap;
    LPBYTE*                 pGDIBits;
    HWND                    hwnd_button;
    //HWNDBITMAP              hwbm;
    BOOL                    fLightMode;
    BOOL                    fDarkMode;
    ID2D1Bitmap*            pD2D1BitmapAppIcon;
    HDC                     hMemDC;
    HDC                     hMemDIB;
    HDC                     hMemBitmapOld;
    LPVOID                  pMemBits;
    ID2D1Bitmap*            pD2D1BitmapButton;
    CHILDMETRICS            pChildren[16];
    INT                     nChildren;
    D3DKMT_WAITFORVERTICALBLANKEVENT vbe;
    HWND hwnd;
    BOOL init;
    void* pUserData;
} DXGIImmersiveSwapchain, IDXGIImmersiveSwapchain;

EXTERN_C
LRESULT PFORCEINLINE APIPRIVATE
InitD2D(
    DXGIImmersiveSwapchain* pImmersiveSwapchain,
    HWND hWnd
    );

EXTERN_C VOID WINAPI hol_up(HWND hWnd);

EXTERN_C
NTSTATUS PFORCEINLINE WINAPI
D3DKMTInitVerticalBlankEvent(
    HWND                              hWnd,
    D3DKMT_WAITFORVERTICALBLANKEVENT* pVbe
    );

EXTERN_C
BOOL PFORCEINLINE WINAPI
D3DKMTWaitForVerticalBlank(
    D3DKMT_WAITFORVERTICALBLANKEVENT* pVbe
    );

EXTERN_C
HRESULT PFORCEINLINE WINAPI
DXGIImmersiveSwapchain_Init(
    DXGIImmersiveSwapchain* pImmersiveSwapchain,
    HWND                    hwnd
    );

EXTERN_C
VOID PFORCEINLINE WINAPI
DXGIImmersiveSwapchain_Shutdown(
    DXGIImmersiveSwapchain* pImmersiveSwapchain
    );

EXTERN_C
BOOL PFORCEINLINE WINAPI
DXGIImmersiveSwapchain_WaitForVerticalBlank(
    DXGIImmersiveSwapchain* pImmersiveSwapchain
    );

EXTERN_C
BOOL PFORCEINLINE WINAPI
DXGIImmersiveSwapchain_Draw(
    DXGIImmersiveSwapchain* pImmersiveSwapchain
    );

EXTERN_C
HRESULT PFORCEINLINE WINAPI
DXGIImmersiveSwapchain_Present(
    DXGIImmersiveSwapchain* pImmersiveSwapchain,
    BOOL                    fRestart,
    BOOL                    fVsync
    );

EXTERN_C
HRESULT PFORCEINLINE WINAPI
DXGIImmersiveSwapchain_Present2(
    DXGIImmersiveSwapchain* pImmersiveSwapchain,
    BOOL                    fRestart,
    BOOL                    fVsync
    );
