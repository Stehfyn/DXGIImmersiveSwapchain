#include "DXGIImmersiveSwapchain/DXGIImmersiveSwapchain.h"
#include <stdint.h>
#include <windows.h>
#include "WinUser2/WinUser2.h"
#include "ImmersiveWindow/ImmersiveWindow.h"
#define REGSTR_PATH_PERSONALIZATION ("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize")

typedef
enum D3D11_CREATE_DEVICE_FLAG_2
{
  D3D11_CREATE_DEVICE_2_FORCE_SINGLETHREADED = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS,
  D3D11_CREATE_DEVICE_2_WELL_RESPECTED = D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT | D3D11_CREATE_DEVICE_PREVENT_ALTERING_LAYER_SETTINGS_FROM_REGISTRY,
  D3D11_CREATE_DEVICE_2_D2D_COMPATIBLE = D3D11_CREATE_DEVICE_BGRA_SUPPORT

} 	D3D11_CREATE_DEVICE_FLAG_2;

typedef
enum DXGI_SWAP_CHAIN_FLAG_2
{
  DXGI_SWAPCHAIN_2_ENABLE_IMMEDIATE_PRESENT = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
  DXGI_SWAPCHAIN_2_ENABLE_WAIT_FOR_NEXT_FRAME_RESOURCES = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
} 	DXGI_SWAP_CHAIN_FLAG_2;

typedef uint32_t DxFlags;

#ifdef NDEBUG
  #define D3D11_DEVICE_NO_DEBUG_IN_RELEASE (0)
  #define DXGI_FACTORY_NO_DEBUG_IN_RELEASE (0)
#else
  #define D3D11_DEVICE_NO_DEBUG_IN_RELEASE (D3D11_CREATE_DEVICE_DEBUG)
  #define DXGI_FACTORY_NO_DEBUG_IN_RELEASE (DXGI_CREATE_FACTORY_DEBUG)
#endif

static void ResizeD2DBitmap(ID2D1DeviceContext* pD2D1DeviceContext, ID2D1Bitmap1** pd2dbitmap, LPBYTE lpBuffer, int nWidth, int nHeight);

static
VOID PFORCEINLINE APIPRIVATE 
ResizeBuffer(
    LPBYTE* lpBuffer,
    SIZE_T  cbBytes)
{
    if(*lpBuffer)
    {
      HeapFree(GetProcessHeap(), 0, *lpBuffer);
    }

    *lpBuffer = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbBytes);
}

static
VOID PFORCEINLINE APIPRIVATE
ResizeCanvas(
    LPGDICANVAS pGDICanvas,
    int         nWidth,
    int         nHeight)
{
    if (pGDICanvas->hOldBitmap)
    {
      SelectBitmap(pGDICanvas->hMemDC, pGDICanvas->hOldBitmap);
      DeleteBitmap(pGDICanvas->hBitmap);
    }
    pGDICanvas->hBitmap = CreateCompatibleBitmap(pGDICanvas->hMemDC, nWidth, nHeight);
    pGDICanvas->hOldBitmap = SelectBitmap(pGDICanvas->hMemDC, pGDICanvas->hBitmap);
    SecureZeroMemory(&pGDICanvas->bmih, sizeof(pGDICanvas->bmih));
    pGDICanvas->bmih.biSize = sizeof(BITMAPINFOHEADER);
    pGDICanvas->bmih.biWidth = nWidth;
    pGDICanvas->bmih.biHeight = -nHeight;
    pGDICanvas->bmih.biPlanes = 1;
    pGDICanvas->bmih.biBitCount = 32;
    pGDICanvas->bmih.biCompression = BI_RGB;

    pGDICanvas->bmihBlit = pGDICanvas->bmih;
}
EXTERN_C
VOID PFORCEINLINE APIPRIVATE
DrawImmersiveWindowFrame(
  ID2D1DeviceContext* pD2D1DeviceContext,
  ID2D1SolidColorBrush* brush,
  ID2D1StrokeStyle* stroke,
  RECT rcCaption);
EXTERN_C
VOID PFORCEINLINE APIPRIVATE
DrawImmersiveLightDarkButton(ID2D1DeviceContext* pD2D1DeviceContext, RECT rcCaption, RECT rcLightDarkBtn,
  ID2D1SolidColorBrush* fontBrush, ID2D1StrokeStyle* fontStroke, ID2D1SolidColorBrush* solidColorBrush,
  DWRITE_GLYPH_RUN* glyphRun, IDWriteFontFace1* pFontFace, UINT16* pGlyphIndicesMDL2,
  IDWriteFontFace1* pFontFace2, UINT16* pGlyphIndicesMDL22, INT glyphInset,
  FLOAT fontSize, FLOAT fontScale, BOOL fLightMode, CAPTIONBUTTON eHotCaptionButton);
EXTERN_C
VOID PFORCEINLINE APIPRIVATE
DrawImmersiveIconButton(ID2D1DeviceContext* pD2D1DeviceContext, ID2D1Bitmap* pD2D1BitmapAppIcon, LONG nWidth, LONG nHeight);
EXTERN_C
VOID PFORCEINLINE APIPRIVATE
DrawImmersiveBackground(ID2D1DeviceContext* pD2D1DeviceContext, ID2D1SolidColorBrush* brush,
  ID2D1StrokeStyle* stroke, LONG rcLightDarkkBtnLeft, LONG rcCaptionLeft, LONG nWidth, LONG nHeight);
EXTERN_C
VOID PFORCEINLINE APIPRIVATE
DrawImmersiveCloseButton(
    ID2D1DeviceContext* pD2D1DeviceContext,
    ID2D1SolidColorBrush* fontBrush,
    ID2D1SolidColorBrush* redBrush,
    ID2D1StrokeStyle* stroke,
    DWRITE_GLYPH_RUN* glyphRun,
    IDWriteFontFace1* pFontFace,
    UINT16* pGlyphIndicesMDL2,
    INT glyphInset,
    INT glyphInset2,
    FLOAT fontSize,
    FLOAT fontScale,
    RECT rcCaption,
    RECT rcCloseButton,
    CAPTIONBUTTON eHotCaptionButton);
EXTERN_C
VOID PFORCEINLINE APIPRIVATE
DrawImmersiveMaximizeButton(
  ID2D1DeviceContext* pD2D1DeviceContext,
  ID2D1SolidColorBrush* fontBrush,
  ID2D1SolidColorBrush* brush,
  ID2D1StrokeStyle* stroke,
  DWRITE_GLYPH_RUN* glyphRun,
  IDWriteFontFace1* pFontFace,
  UINT16* pGlyphIndicesMDL2,
  INT glyphInset,
  INT glyphInset2,
  FLOAT fontSize,
  FLOAT fontScale,
  UINT nDpi,
  RECT rcCaption,
  RECT rcMaximizeButton,
  BOOL isMaximized,
  CAPTIONBUTTON eHotCaptionButton);
EXTERN_C
VOID PFORCEINLINE APIPRIVATE
DrawImmersiveMinimizeButton(
    ID2D1DeviceContext* pD2D1DeviceContext,
    ID2D1SolidColorBrush* fontBrush,
    ID2D1SolidColorBrush* brush,
    ID2D1StrokeStyle* stroke,
    DWRITE_GLYPH_RUN* glyphRun,
    IDWriteFontFace1* pFontFace,
    UINT16* pGlyphIndicesMDL2,
    INT glyphInset,
    INT glyphInset2,
    FLOAT fontSize,
    FLOAT fontScale,
    UINT nDpi,
    RECT rcCaption,
    RECT rcMinimizeButton,
    CAPTIONBUTTON eHotCaptionButton);
EXTERN_C FLOAT PFORCEINLINE APIPRIVATE MyRoundf(FLOAT f)
{
  return (FLOAT)max((INT)f, (INT)(f + 0.5F));
}

EXTERN_C
FLOAT PFORCEINLINE APIPRIVATE
GetFontSizeFromWindowDpi(HWND hwnd, UINT nDpi)
{
  return (FLOAT)MulDiv(10, nDpi, 96);
}

EXTERN_C
INT PFORCEINLINE APIPRIVATE
GetGlyphInsetsFromFontSize(FLOAT fontSize)
{
  return (INT)MyRoundf(0.5F * fontSize);
}

EXTERN_C
FLOAT PFORCEINLINE APIPRIVATE
GetFontSizeFromDevice(HWND hwnd)
{
  //CreateIC
  //HDC hDC = GetDC(hWnd);
  ////FLOAT fontSize = (FLOAT)-MulDiv(9.0f, GetDeviceCaps(hDC, LOGPIXELSY), 72);
  //FLOAT fontSize = (FLOAT)MulDiv(9.0f, GetDeviceCaps(hDC, LOGPIXELSY), 96);
  //ReleaseDC(hWnd, hDC);
  return 0.0F;
  UNREFERENCED_PARAMETER(hwnd);
}

static
void ResizeD2DBitmap(
  ID2D1DeviceContext* pD2D1DeviceContext,
  ID2D1Bitmap1** pd2dbitmap,
  LPBYTE lpBuffer,
  int nWidth,
  int nHeight)
{
  HRESULT hr;
  D2D1_BITMAP_PROPERTIES bitmapProps = { 0 };
  bitmapProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
  bitmapProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
  bitmapProps.dpiX = 96.0f;
  bitmapProps.dpiY = 96.0f;

  D2D1_BITMAP_PROPERTIES1 bitmapProps1 = { 0 };
  bitmapProps1.pixelFormat = bitmapProps.pixelFormat;
  bitmapProps1.dpiX = bitmapProps.dpiX;
  bitmapProps1.dpiY = bitmapProps.dpiY;
  bitmapProps1.bitmapOptions = //D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE|
    D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW; // Allows CPU updates

  if (*pd2dbitmap)
  {
    ID2D1Bitmap1_Release(*pd2dbitmap);
  }
  D2D_SIZE_U size = { nWidth, nHeight };
  hr = ID2D1DeviceContext_CreateBitmap(pD2D1DeviceContext, size, lpBuffer, size.width * 4, &bitmapProps1, pd2dbitmap);
  //ASSERT_W32(SUCCEEDED(hr));
}

static
DXGI_SWAP_CHAIN_DESC1 PFORCEINLINE APIPRIVATE
GetSwapchainDescription(
    HDC hdc)
{
    DXGI_SWAP_CHAIN_DESC1 desc =
    {
      .Width       = GetDeviceCaps(hdc, DESKTOPHORZRES),
      .Height      = GetDeviceCaps(hdc, DESKTOPVERTRES),
      .Format      = DXGI_FORMAT_B8G8R8A8_UNORM,
      .Stereo      = FALSE,
      .SampleDesc  = 
      {
        .Count   = 1,
        .Quality = 0
      },
      .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
      .BufferCount = 3,
      .Scaling     = DXGI_SCALING_STRETCH,
      .SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD,
      //.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
      .AlphaMode   = DXGI_ALPHA_MODE_PREMULTIPLIED,
      .Flags       = DXGI_SWAPCHAIN_2_ENABLE_IMMEDIATE_PRESENT | DXGI_SWAPCHAIN_2_ENABLE_WAIT_FOR_NEXT_FRAME_RESOURCES
    };

    return desc;
}

static
D2D1_BITMAP_PROPERTIES1 PFORCEINLINE APIPRIVATE
GetSurfaceBitmapProperties(
    UINT nDpi)
{
    D2D1_BITMAP_PROPERTIES1 props = 
    {
      .pixelFormat = 
      {
        .format    = DXGI_FORMAT_B8G8R8A8_UNORM,
        .alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED,
      },
      .dpiX = (FLOAT)nDpi,
      .dpiY = (FLOAT)nDpi,
      .bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
      .colorContext = NULL
    };

    return props;
}

static
D2D1_RENDER_TARGET_PROPERTIES PFORCEINLINE APIPRIVATE
GetSurfaceRenderTargetProperties(
  D2D1_RENDER_TARGET_TYPE type,
  UINT nDpi)
{
    D2D1_RENDER_TARGET_PROPERTIES props = 
    {
      .type = type,
      .pixelFormat = 
      {
        .format    = DXGI_FORMAT_B8G8R8A8_UNORM,
        .alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED,
      },
      .dpiX = (FLOAT)nDpi,
      .dpiY = (FLOAT)nDpi,
      .usage = D2D1_RENDER_TARGET_USAGE_NONE,
      .minLevel = D2D1_FEATURE_LEVEL_DEFAULT,
    };

    return props;
}

static
D2D1_HWND_RENDER_TARGET_PROPERTIES PFORCEINLINE APIPRIVATE
GetHwndRenderTargetProperties(
    HWND hwnd)
{
    D2D1_HWND_RENDER_TARGET_PROPERTIES props =
    {
      .hwnd = hwnd,
      .pixelSize = 
      {
        .width = GetDeviceCaps(GetDC(hwnd), DESKTOPHORZRES),
        .height = GetDeviceCaps(GetDC(hwnd), DESKTOPVERTRES)
      },
      .presentOptions = D2D1_PRESENT_OPTIONS_NONE
    };

    return props;
}

EXTERN_C 
LRESULT PFORCEINLINE APIPRIVATE
DestroySwapchain(
    IDXGIImmersiveSwapchain* pImmersiveSwapchain)
{
    UINT32 uRefCnt;
    ID2D1StrokeStyle_Release(pImmersiveSwapchain->pD2D1StrokeStyle);
    ID2D1SolidColorBrush_Release(pImmersiveSwapchain->pD2D1RedBrush);
    ID2D1SolidColorBrush_Release(pImmersiveSwapchain->pD2D1BlackBrush);
    ID2D1SolidColorBrush_Release(pImmersiveSwapchain->pD2D1InactiveBrush);
    ID2D1SolidColorBrush_Release(pImmersiveSwapchain->pD2D1ColorizationColorBrush);
    ID2D1SolidColorBrush_Release(pImmersiveSwapchain->pD2D1FadedColorizationColorBrush);
    ID2D1SolidColorBrush_Release(pImmersiveSwapchain->pD2D1WindowFrameColorBrush);
    uRefCnt = ID2D1SolidColorBrush_Release(pImmersiveSwapchain->pD2D1Brush);
    ASSERT_W32(!uRefCnt);
    uRefCnt = IDCompositionVisual_Release(pImmersiveSwapchain->pDCompVisual);
    ASSERT_W32(!uRefCnt);
    uRefCnt = IDCompositionTarget_Release(pImmersiveSwapchain->pDCompTarget);
    ASSERT_W32(!uRefCnt);
    uRefCnt = IDCompositionDevice_Release(pImmersiveSwapchain->pDCompDevice);
    ASSERT_W32(!uRefCnt);
    uRefCnt = ID2D1Bitmap_Release(pImmersiveSwapchain->pD2D1Bitmap);
    //ASSERT_W32(!uRefCnt);
    uRefCnt = ID2D1Effect_Release(pImmersiveSwapchain->pD2D1EffectBlur);
    ASSERT_W32(!uRefCnt);
    uRefCnt = ID2D1DeviceContext_Release(pImmersiveSwapchain->pD2D1DeviceContext);
    ASSERT_W32(!uRefCnt);
    uRefCnt = ID2D1Device1_Release(pImmersiveSwapchain->pD2D1Device);
    ASSERT_W32(!uRefCnt);
    uRefCnt = ID2D1Factory2_Release(pImmersiveSwapchain->pD2D1Factory2);
    ASSERT_W32(!uRefCnt);
    uRefCnt = IDXGISurface2_Release(pImmersiveSwapchain->pDXGISurface2);
    ASSERT_W32(!uRefCnt);
    uRefCnt = IDXGISwapChain2_Release(pImmersiveSwapchain->pDXGISwapchain2);
    ASSERT_W32(!uRefCnt);

#ifndef NDEBUG
    ID3D11Debug_ReportLiveDeviceObjects(pImmersiveSwapchain->pD3D11Debug, D3D11_RLDO_DETAIL);
    uRefCnt = ID3D11Debug_Release(pImmersiveSwapchain->pD3D11Debug);
#endif
    uRefCnt = ID3D11Device_Release(pImmersiveSwapchain->pD3D11Device);
    
    uRefCnt = IDXGIDevice_Release(pImmersiveSwapchain->pDXGIDevice3);
    //ASSERT_W32(!uRefCnt);
    uRefCnt = IDXGIFactory2_Release(pImmersiveSwapchain->pDXGIFactory2);
    //ASSERT_W32(!uRefCnt);
    
    return S_OK;
}

EXTERN_C
FORCEINLINE BOOL
GetDevice(
    IDXGIImmersiveSwapchain* pImmersiveSwapchain,
    void** pDevice)
{
    (*pDevice) = pImmersiveSwapchain->pD3D11Device;
}

EXTERN_C
FORCEINLINE BOOL
GetDeviceContext(
    IDXGIImmersiveSwapchain* pImmersiveSwapchain,
    void** pDeviceContext)
{
    (*pDeviceContext) = pImmersiveSwapchain->pD3D11DeviceContext;
}

EXTERN_C
FORCEINLINE BOOL
GetSwapchain(
    IDXGIImmersiveSwapchain* pImmersiveSwapchain,
    void** pSwapchain)
{
    (*pSwapchain) = pImmersiveSwapchain->pDXGISwapchain2;
}

EXTERN_C
FORCEINLINE LPVOID WINAPI
BeginImmersivePaintInternal(
    DXGIImmersiveSwapchain* pImmersiveSwapchain)
{
    const D2D1_COLOR_F     c_clear_color  = { 0.0F, 0.0F, 0.0F, 0.0F };
    const D2D_MATRIX_3X2_F c_identity_3x2 = { 1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F };
    const CAPTIONBUTTON    c_hot_caption_button = ((IMMERSIVEDATA*)GetWindowLongPtr(pImmersiveSwapchain->hwnd, 0))->eHotCaptionButton;
    const UINT c_nDpi = GetDpiForWindow(pImmersiveSwapchain->hwnd);
    const BOOL c_fIsMaximized = IsMaximized(pImmersiveSwapchain->hwnd);
    const INT  c_nCaptionButtonWidth = MulDiv(45, c_nDpi, 96);
    DWRITE_GLYPH_OFFSET c_glyph_offsets = { 0 };
    DWRITE_GLYPH_RUN    glyph_run = { 0 };

    RECT rcWindow;
    RECT rcCaption;
    RECT rcCaptionButton;
    RECT rcLightDarkBtn;
    RECT rcLightDarkkBtn;
    RECT rcCloseButton;
    RECT rcMaximizeButton;
    RECT rcMinimizeButton;
    D2D_RECT_F rcfLightDarkButton;
    D2D_RECT_F rcfCaptionButtonClose;

    D2D_POINT_2F pos;
    D2D_POINT_2F pos2;
    SIZE_T  cbNeeded;
    HRESULT hr;

    ASSERT_W32(GetWindowRect(pImmersiveSwapchain->hwnd,  &rcWindow));
    ASSERT_W32(GetCaptionRect(pImmersiveSwapchain->hwnd, &rcCaption));
    rcLightDarkkBtn  = rcCaption;
    rcLightDarkBtn   = rcCaption;
    rcCloseButton    = rcCaption;
    rcMaximizeButton = rcCaption;
    rcMinimizeButton = rcCaption;
    ASSERT_W32(GetCaptionButtonRectFast(&rcLightDarkkBtn,  c_nCaptionButtonWidth, CB_LIGHTDARKBUTTON));
    ASSERT_W32(GetCaptionButtonRectFast(&rcLightDarkBtn,   c_nCaptionButtonWidth, CB_LIGHTDARKBUTTON));
    ASSERT_W32(GetCaptionButtonRectFast(&rcCloseButton,    c_nCaptionButtonWidth, CB_CLOSEBUTTON));
    ASSERT_W32(GetCaptionButtonRectFast(&rcMaximizeButton, c_nCaptionButtonWidth, CB_MAXBUTTON));
    ASSERT_W32(GetCaptionButtonRectFast(&rcMinimizeButton, c_nCaptionButtonWidth, CB_MINBUTTON));

    glyph_run.glyphCount   = 1;
    glyph_run.glyphOffsets = &c_glyph_offsets;

    WaitForSingleObject(pImmersiveSwapchain->hFrameLatencyWaitableObject, INFINITE);
    //D3D11_VIEWPORT vp = {
    //.TopLeftX = 0.0f,
    //.TopLeftY = 0.0f,
    //.Width = RECTWIDTH(rcWindow),
    //.Height = RECTHEIGHT(rcWindow),
    //.MinDepth = 0.0f,
    //.MaxDepth = 1.0f
    //};
    //ID3D11DeviceContext_RSSetViewports(pImmersiveSwapchain->pD3D11DeviceContext, 1, &vp);
    D2D_RECT_F clipRect = { 0.0f, 0.0f, (FLOAT)RECTWIDTH(rcCaption), (FLOAT)RECTHEIGHT(rcCaption) };
    //ID2D1DeviceContext_PushAxisAlignedClip(pImmersiveSwapchain->pD2D1DeviceContext, &clipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    ID2D1DeviceContext_BeginDraw(pImmersiveSwapchain->pD2D1DeviceContext);
    ID2D1DeviceContext_SetTransform(pImmersiveSwapchain->pD2D1DeviceContext, &c_identity_3x2);
    ID2D1DeviceContext_Clear(pImmersiveSwapchain->pD2D1DeviceContext, &c_clear_color);
    //ID2D1DeviceContext_SetTextAntialiasMode(pImmersiveSwapchain->pD2D1DeviceContext, D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
    //ID2D1DeviceContext_SetAntialiasMode(pImmersiveSwapchain->pD2D1DeviceContext, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    //ID2D1DeviceContext_SetPrimitiveBlend(pImmersiveSwapchain->pD2D1DeviceContext, D2D1_PRIMITIVE_BLEND_ADD);

    {
      const FLOAT c_ex_font_scale = 1.50F;
      const FLOAT c_font_size     = GetFontSizeFromWindowDpi(pImmersiveSwapchain->hwnd, c_nDpi);
      const INT   c_glyph_inset   = GetGlyphInsetsFromFontSize(c_font_size);
      const INT   c_glyph_inset2  = GetGlyphInsetsFromFontSize(c_font_size * c_ex_font_scale);

      ID2D1SolidColorBrush* pD2D1SolidColorBrush_BackgroundBrush = (pImmersiveSwapchain->fDarkMode) ?  pImmersiveSwapchain->pD2D1BlackBrush : pImmersiveSwapchain->pD2D1Brush;
      ID2D1SolidColorBrush* fill = /*(fActive) ? pImmersiveSwapchain->pD2D1InactiveBrush :*/pImmersiveSwapchain->pD2D1ColorizationColorBrush;
      ID2D1SolidColorBrush* text = (pImmersiveSwapchain->fDarkMode) ? pImmersiveSwapchain->pD2D1Brush : pImmersiveSwapchain->pD2D1BlackBrush;

      /* Immersive Background ***************************************************************************************/
      {
        DrawImmersiveBackground(
          pImmersiveSwapchain->pD2D1DeviceContext,
          pD2D1SolidColorBrush_BackgroundBrush,
          pImmersiveSwapchain->pD2D1StrokeStyle,
          rcLightDarkkBtn.left,
          rcCaption.left,
          RECTWIDTH(rcCaption),
          RECTHEIGHT(rcCaption)
          //RECTWIDTH(rcWindow),
          //RECTHEIGHT(rcWindow)
        );
      }

      /* Window Frame (aka Border) ***************************************************************************************/
      {
        GUITHREADINFO gti = { sizeof(gti) };

        GetGUIThreadInfo(GetCurrentThreadId(), &gti);

        if (gti.hwndActive != pImmersiveSwapchain->hwnd)
        {
          DrawImmersiveWindowFrame(
            pImmersiveSwapchain->pD2D1DeviceContext,
            pImmersiveSwapchain->pD2D1WindowFrameColorBrush,
            pImmersiveSwapchain->pD2D1StrokeStyle,
            rcCaption
          );
        }
      }

      /* Close Button ***************************************************************************************/
      {
        DrawImmersiveCloseButton(
          pImmersiveSwapchain->pD2D1DeviceContext,
          text,
          pImmersiveSwapchain->pD2D1RedBrush,
          pImmersiveSwapchain->pD2D1StrokeStyle,
          &glyph_run,
          pImmersiveSwapchain->pFontFace,
          pImmersiveSwapchain->pGlyphIndicesMDL2,
          c_glyph_inset,
          c_glyph_inset2,
          c_font_size,
          c_ex_font_scale,
          rcCaption,
          rcCloseButton,
          c_hot_caption_button
        );
      }

      /* Maximize Button ***************************************************************************************/
      {
        DrawImmersiveMaximizeButton(
          pImmersiveSwapchain->pD2D1DeviceContext,
          text,
          pImmersiveSwapchain->pD2D1ColorizationColorBrush,
          pImmersiveSwapchain->pD2D1StrokeStyle,
          &glyph_run,
          pImmersiveSwapchain->pFontFace,
          pImmersiveSwapchain->pGlyphIndicesMDL2,
          c_glyph_inset,
          c_glyph_inset2,
          c_font_size,
          c_ex_font_scale,
          c_nDpi,
          rcCaption,
          rcMaximizeButton,
          c_fIsMaximized,
          c_hot_caption_button
        );
      }

      /* Minimize Button ***************************************************************************************/
      {
        DrawImmersiveMinimizeButton(
          pImmersiveSwapchain->pD2D1DeviceContext,
          text,
          pImmersiveSwapchain->pD2D1ColorizationColorBrush,
          pImmersiveSwapchain->pD2D1StrokeStyle,
          &glyph_run,
          pImmersiveSwapchain->pFontFace,
          pImmersiveSwapchain->pGlyphIndicesMDL2,
          c_glyph_inset,
          c_glyph_inset2,
          c_font_size,
          c_ex_font_scale,
          c_nDpi,
          rcCaption,
          rcMinimizeButton,
          c_hot_caption_button
        );
      }

      /* Light/Dark Button ***************************************************************************************/
      {
        DrawImmersiveLightDarkButton(
          pImmersiveSwapchain->pD2D1DeviceContext,
          rcCaption,
          rcLightDarkBtn,
          text,
          pImmersiveSwapchain->pD2D1StrokeStyle,
          pImmersiveSwapchain->pD2D1ColorizationColorBrush,
          &glyph_run,
          pImmersiveSwapchain->pFontFace,
          pImmersiveSwapchain->pGlyphIndicesMDL2,
          pImmersiveSwapchain->pFontFace2,
          pImmersiveSwapchain->pGlyphIndicesMDL22,
          c_glyph_inset2,
          c_font_size,
          c_ex_font_scale,
          pImmersiveSwapchain->fLightMode,
          c_hot_caption_button
        );
      }

      /* Icon Button ***************************************************************************************/
      //{
      //  DrawImmersiveIconButton(
      //    pImmersiveSwapchain->pD2D1DeviceContext,
      //    pImmersiveSwapchain->pD2D1BitmapAppIcon,
      //    RECTWIDTH(rcCaption),
      //    RECTHEIGHT(rcCaption)
      //  );
      //}
    }

    /* 1. Submit and flush D2D commands */
    {
      ASSERT_W32(SUCCEEDED(ID2D1DeviceContext_EndDraw(
        pImmersiveSwapchain->pD2D1DeviceContext,
        0,
        0
      )));
    }

    return 0;
}

 BOOL SetShit(HWND hWnd, proc pProc)
{
    //DXGIImmersiveSwapchain* pImmersiveSwapchain =
    //  (DXGIImmersiveSwapchain*)GetWindowLongPtr(hWnd, offsetof(IMMERSIVEDATA, pSwapchain));
    //
    //pImmersiveSwapchain->pProc = pProc;

    return TRUE;
}
 
EXTERN_C
LRESULT PFORCEINLINE APIPRIVATE
InitD2D(
    DXGIImmersiveSwapchain* pImmersiveSwapchain,
    HWND hWnd)
{
    HRESULT hr;

    ASSERT_W32(SUCCEEDED(hr =
      D3D11CreateDevice(
        0,
        D3D_DRIVER_TYPE_HARDWARE,
        //D3D_DRIVER_TYPE_SOFTWARE,
        //D3D_DRIVER_TYPE_WARP,
        0,
        //(D3D11_CREATE_DEVICE_FLAG)(
          /*(DxFlags)*/D3D11_CREATE_DEVICE_2_WELL_RESPECTED |
          /*(DxFlags)*/D3D11_CREATE_DEVICE_2_FORCE_SINGLETHREADED |
          /*(DxFlags)*/D3D11_CREATE_DEVICE_2_D2D_COMPATIBLE |
          /*(DxFlags)*/D3D11_DEVICE_NO_DEBUG_IN_RELEASE
        //)
        ,
        //c_d3d_feature_levels,
        //ARRAYSIZE(c_d3d_feature_levels),
        NULL,
        0,
        D3D11_SDK_VERSION,
        &pImmersiveSwapchain->pD3D11Device,
        0,
        &pImmersiveSwapchain->pD3D11DeviceContext
      )
    ));
    
#ifndef NDEBUG
    ASSERT_W32(SUCCEEDED(
      ID3D11Device_QueryInterface(
        pImmersiveSwapchain->pD3D11Device,
        &IID_ID3D11Debug,
        (void**)&pImmersiveSwapchain->pD3D11Debug
      )
    ));
#endif

    {
      ASSERT_W32(SUCCEEDED(
        ID3D11Device_QueryInterface(
          pImmersiveSwapchain->pD3D11Device,
          &IID_IDXGIDevice3,
          (void**)&pImmersiveSwapchain->pDXGIDevice3
        )
      ));

      IDXGIDevice3_SetMaximumFrameLatency(pImmersiveSwapchain->pDXGIDevice3, 1);
    }

    {
      HDC hdc = GetDC(0);
      const DXGI_SWAP_CHAIN_DESC1 c_desc = GetSwapchainDescription(hdc);
      const D2D1_FACTORY_OPTIONS  c_d2d_factory_options = { D2D1_DEBUG_LEVEL_NONE };
      const D2D1_BITMAP_PROPERTIES1 c_bitmap_props = GetSurfaceBitmapProperties(GetDpiForWindow(hWnd));
      ReleaseDC(0, hdc);

      IDXGIDevice3_GetAdapter(pImmersiveSwapchain->pDXGIDevice3, &pImmersiveSwapchain->pDXGIAdapter);
      IDXGIAdapter_GetParent(pImmersiveSwapchain->pDXGIAdapter, &IID_IDXGIFactory2, (void**)&pImmersiveSwapchain->pDXGIFactory2);

      //ASSERT_W32(SUCCEEDED(
      //  CreateDXGIFactory2(
      //    DXGI_FACTORY_NO_DEBUG_IN_RELEASE,
      //    &IID_IDXGIFactory2,
      //    (void**)&pImmersiveSwapchain->pDXGIFactory2
      //  )
      //));

      ASSERT_W32(SUCCEEDED(
        IDXGIFactory2_CreateSwapChainForComposition(
          pImmersiveSwapchain->pDXGIFactory2,
          (IUnknown*)pImmersiveSwapchain->pDXGIDevice3,
          &c_desc,
          0,
          &pImmersiveSwapchain->pDXGISwapchain2
          )
        ));
    
      ASSERT_W32(SUCCEEDED(
        D2D1CreateFactory(
          D2D1_FACTORY_TYPE_SINGLE_THREADED,
          //D2D1_FACTORY_TYPE_MULTI_THREADED,
          &IID_ID2D1Factory2,
          &c_d2d_factory_options,
          &pImmersiveSwapchain->pD2D1Factory2
          )
        ));

      ASSERT_W32(SUCCEEDED(
        ID2D1Factory2_CreateDevice1(
          pImmersiveSwapchain->pD2D1Factory2,
          pImmersiveSwapchain->pDXGIDevice3,
          &pImmersiveSwapchain->pD2D1Device
        )
      ));
      //ID2D1Device_SetRenderingPriority(pImmersiveSwapchain->pD2D1Device, D2D1_RENDERING_PRIORITY_IMMEDIATE);

      ASSERT_W32(SUCCEEDED(
        ID2D1Device1_CreateDeviceContext(
          pImmersiveSwapchain->pD2D1Device,
          D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
          //D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
          &pImmersiveSwapchain->pD2D1DeviceContext
        )
      ));
      
      //D2D1_RENDER_TARGET_PROPERTIES c_target_props = GetSurfaceRenderTargetProperties(D2D1_RENDER_TARGET_TYPE_HARDWARE, GetDpiForWindow(hWnd));
      //D2D1_HWND_RENDER_TARGET_PROPERTIES c_hwnd_target_props = GetHwndRenderTargetProperties(hWnd);
      //ASSERT_W32(SUCCEEDED(ID2D1Factory2_CreateHwndRenderTarget(
      //  pImmersiveSwapchain->pD2D1Factory2,
      //  &c_target_props,
      //  &c_hwnd_target_props,
      //  &pImmersiveSwapchain->pD2D1HwndRenderTarget
      //)));
      //pImmersiveSwapchain->pD2D1DeviceContext = &pImmersiveSwapchain->pD2D1HwndRenderTarget;

      ASSERT_W32(SUCCEEDED(
        IDXGISwapChain2_GetBuffer(
          pImmersiveSwapchain->pDXGISwapchain2,
          0,
          &IID_IDXGISurface2,
          (void**)&pImmersiveSwapchain->pDXGISurface2
        )
      ));

      //D2D1_RENDER_TARGET_PROPERTIES c_target_props = GetSurfaceRenderTargetProperties(GetDpiForWindow(hWnd));
      //ID2D1Factory2_CreateDxgiSurfaceRenderTarget(
      //  pImmersiveSwapchain->pD2D1Factory2,
      //  pImmersiveSwapchain->pDXGISurface2,
      //  &c_target_props,
      //  //&pImmersiveSwapchain->pD2D1RenderTarget);
      //  &pImmersiveSwapchain->pD2D1DeviceContext);

      ASSERT_W32(SUCCEEDED(
        ID2D1DeviceContext1_CreateBitmapFromDxgiSurface(
          pImmersiveSwapchain->pD2D1DeviceContext,
          pImmersiveSwapchain->pDXGISurface2,
          &c_bitmap_props,
          &pImmersiveSwapchain->pD2D1Bitmap
        )
      ));

      ID2D1DeviceContext_SetTarget(
        pImmersiveSwapchain->pD2D1DeviceContext,
        pImmersiveSwapchain->pD2D1Bitmap
      );
    
      ASSERT_W32(SUCCEEDED(
        DCompositionCreateDevice(
          pImmersiveSwapchain->pDXGIDevice3,
          &GUID_Impl_IDCompositionDevice_ImmersiveDevice,
          (void**)&pImmersiveSwapchain->pDCompDevice
        )
      ));
    
      ASSERT_W32(SUCCEEDED(
        IDCompositionDevice_CreateTargetForHwnd(
          pImmersiveSwapchain->pDCompDevice,
          hWnd,
          FALSE,
          &pImmersiveSwapchain->pDCompTarget
        )
      ));

      ASSERT_W32(SUCCEEDED(
        IDCompositionDevice_CreateVisual(
          pImmersiveSwapchain->pDCompDevice,
          &pImmersiveSwapchain->pDCompVisual
        )
      ));

      ASSERT_W32(SUCCEEDED(
        IDCompositionVisual_SetContent(
          pImmersiveSwapchain->pDCompVisual,
          pImmersiveSwapchain->pDXGISwapchain2
        )
      ));

      ASSERT_W32(SUCCEEDED(
        IDCompositionTarget_SetRoot(
          pImmersiveSwapchain->pDCompTarget,
          pImmersiveSwapchain->pDCompVisual
        )
      ));

      ASSERT_W32(SUCCEEDED(
        IDCompositionDevice_Commit(
          pImmersiveSwapchain->pDCompDevice
        )
      ));

      pImmersiveSwapchain->hFrameLatencyWaitableObject = 
        IDXGISwapChain2_GetFrameLatencyWaitableObject(
          pImmersiveSwapchain->pDXGISwapchain2
        );
      ASSERT_W32(pImmersiveSwapchain->hFrameLatencyWaitableObject);
      
      //IDXGISwapChain2_Present(pImmersiveSwapchain->pDXGISwapchain2, 1, 0);
    }

    {
      const D2D1_COLOR_F c_light_mode_brush_color        = { 1.00F, 1.00F, 1.00F, 0.95F };
      const D2D1_COLOR_F c_dark_mode_brush_color         = { 43.0F / 255.0F, 43.0F / 255.0F, 43.0F / 255.0F, 0.85F };
      const D2D1_COLOR_F c_red_brush_color               = { 196.0F / 255.0F, 43.0F / 255.0F,28.0F / 255.0F, 1.0F  };
      const D2D1_COLOR_F c_inactive_brush_color          = D2D_RGB_FROM_COLORREF(GetSysColor(COLOR_BTNSHADOW), 1.0F);
      const DWORD        c_dwColor                       = GetSysColor(COLOR_MENUHILIGHT);
      const D2D1_COLOR_F c_colorization_brush_color      = D2D_RGB_FROM_COLORREF(c_dwColor, 1.0F);
      const D2D1_COLOR_F c_colorization_brush_color_fade = D2D_RGB_FROM_COLORREF(c_dwColor, 0.1F);
      const D2D1_COLOR_F c_colorization_window_frame_color = { 100.0F / 255.0F, 100.0F / 255.0F, 100.0F / 255.0F, 0.95F };
      const D2D1_BRUSH_PROPERTIES c_brush_props          = { 1.0F, 0.0F };
      const D2D1_STROKE_STYLE_PROPERTIES1 c_stroke_style_props =
      {
        D2D1_CAP_STYLE_SQUARE,
        D2D1_CAP_STYLE_SQUARE,
        D2D1_CAP_STYLE_SQUARE,
        D2D1_LINE_JOIN_ROUND,
        0.0F,
        D2D1_DASH_STYLE_SOLID,
        0.0F,
        D2D1_STROKE_TRANSFORM_TYPE_HAIRLINE
      };

      ASSERT_W32(SUCCEEDED(
        ID2D1DeviceContext_CreateSolidColorBrush(
          pImmersiveSwapchain->pD2D1DeviceContext,
          &c_light_mode_brush_color,
          &c_brush_props,
          &pImmersiveSwapchain->pD2D1Brush
        )
      ));

      ASSERT_W32(SUCCEEDED(
        ID2D1DeviceContext_CreateSolidColorBrush(
          pImmersiveSwapchain->pD2D1DeviceContext,
          &c_red_brush_color,
          &c_brush_props,
          &pImmersiveSwapchain->pD2D1RedBrush
        )
      ));

      ASSERT_W32(SUCCEEDED(
        ID2D1DeviceContext_CreateSolidColorBrush(
          pImmersiveSwapchain->pD2D1DeviceContext,
          &c_dark_mode_brush_color,
          &c_brush_props,
          &pImmersiveSwapchain->pD2D1BlackBrush
        )
      ));

      ASSERT_W32(SUCCEEDED(
        ID2D1DeviceContext_CreateSolidColorBrush(
          pImmersiveSwapchain->pD2D1DeviceContext,
          &c_inactive_brush_color,
          &c_brush_props,
          &pImmersiveSwapchain->pD2D1InactiveBrush
        )
      ));

      ASSERT_W32(SUCCEEDED(
        ID2D1DeviceContext_CreateSolidColorBrush(
          pImmersiveSwapchain->pD2D1DeviceContext,
          &c_colorization_brush_color,
          &c_brush_props,
          &pImmersiveSwapchain->pD2D1ColorizationColorBrush
        )
      ));

      ASSERT_W32(SUCCEEDED(
        ID2D1DeviceContext_CreateSolidColorBrush(
          pImmersiveSwapchain->pD2D1DeviceContext,
          &c_colorization_brush_color_fade,
          &c_brush_props,
          &pImmersiveSwapchain->pD2D1FadedColorizationColorBrush
        )
      ));

      ASSERT_W32(SUCCEEDED(
        ID2D1DeviceContext_CreateSolidColorBrush(
          pImmersiveSwapchain->pD2D1DeviceContext,
          &c_colorization_window_frame_color,
          &c_brush_props,
          &pImmersiveSwapchain->pD2D1WindowFrameColorBrush
        )
      ));

      ASSERT_W32(SUCCEEDED(
        ID2D1Factory2_CreateStrokeStyle1(
          pImmersiveSwapchain->pD2D1Factory2,
          &c_stroke_style_props,
          0,
          0,
          &pImmersiveSwapchain->pD2D1StrokeStyle
        )
      ));

      ASSERT_W32(SUCCEEDED(
        ID2D1DeviceContext_CreateEffect(
          pImmersiveSwapchain->pD2D1DeviceContext,
          &CLSID_D2D1GaussianBlur,
          &pImmersiveSwapchain->pD2D1EffectBlur
        )
      ));
    }

    ASSERT_W32(SUCCEEDED(
      DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        &IID_IDWriteFactory1,
        &pImmersiveSwapchain->pDWriteFactory1
      )
    ));

    ASSERT_W32(SUCCEEDED(
      IDWriteFactory1_GetSystemFontCollection(
        pImmersiveSwapchain->pDWriteFactory1,
        &pImmersiveSwapchain->pFontCollection,
        TRUE
      )
    ));

    { 
      UINT32 index;
      BOOL exists;
      {
        hr = IDWriteFontCollection1_FindFamilyName(pImmersiveSwapchain->pFontCollection, L"Segoe MDL2 Assets", &index, &exists);
        ASSERT_W32(S_OK == hr);
        hr = IDWriteFontCollection1_GetFontFamily1(pImmersiveSwapchain->pFontCollection, index, &pImmersiveSwapchain->pFontFamily);
        //hr = IDWriteFontCollection1_FindFamilyName(pImmersiveSwapchain->pFontCollection, L"Segoe Fluent Icons", &index, &exists);
        //hr = IDWriteFontCollection1_FindFamilyName(pImmersiveSwapchain->pFontCollection, L"Segoe UI Symbol", &index, &exists);

        ASSERT_W32(S_OK == hr);

        hr = IDWriteFontFamily1_GetFirstMatchingFont(
          pImmersiveSwapchain->pFontFamily,
          DWRITE_FONT_WEIGHT_THIN,
          DWRITE_FONT_STRETCH_NORMAL,
          DWRITE_FONT_STYLE_NORMAL,
          &pImmersiveSwapchain->pFont
        );
        ASSERT_W32(S_OK == hr);

        hr = IDWriteFont1_CreateFontFace(pImmersiveSwapchain->pFont, &pImmersiveSwapchain->pFontFace);
        ASSERT_W32(S_OK == hr);

        pImmersiveSwapchain->pCodePointsMDL2[0]  = CP_MDL2_SETTINGS1;
        pImmersiveSwapchain->pCodePointsMDL2[1]  = CP_MDL2_SETTINGS2;
        pImmersiveSwapchain->pCodePointsMDL2[2]  = CP_MDL2_SETTINGS3;
        pImmersiveSwapchain->pCodePointsMDL2[3]  = CP_MDL2_LIGHTMODE1;
        pImmersiveSwapchain->pCodePointsMDL2[4]  = CP_MDL2_LIGHTMODE2;
        pImmersiveSwapchain->pCodePointsMDL2[5]  = CP_MDL2_DARKMODE1;
        pImmersiveSwapchain->pCodePointsMDL2[6]  = CP_MDL2_DARKMODE2;
        pImmersiveSwapchain->pCodePointsMDL2[7]  = CP_MDL2_DARKMODE3;
        pImmersiveSwapchain->pCodePointsMDL2[8]  = CP_MDL2_LIGHTDARK1;
        pImmersiveSwapchain->pCodePointsMDL2[9]  = CP_MDL2_LIGHTDARK2;
        pImmersiveSwapchain->pCodePointsMDL2[10] = CP_MDL2_MINIMIZE;
        pImmersiveSwapchain->pCodePointsMDL2[11] = CP_MDL2_MAXIMIZE;
        pImmersiveSwapchain->pCodePointsMDL2[12] = CP_MDL2_RESTORE;
        pImmersiveSwapchain->pCodePointsMDL2[13] = CP_MDL2_CLOSE;
        pImmersiveSwapchain->pCodePointsMDL2[14] = CP_MDL2_FEEDBACK;
        hr = IDWriteFontFace1_GetGlyphIndices(pImmersiveSwapchain->pFontFace, &pImmersiveSwapchain->pCodePointsMDL2[0], 15, &pImmersiveSwapchain->pGlyphIndicesMDL2[0]);
      }
      {
        //hr = IDWriteFontCollection1_FindFamilyName(pImmersiveSwapchain->pFontCollection, L"Times New Roman", &index, &exists);
        //hr = IDWriteFontCollection1_FindFamilyName(pImmersiveSwapchain->pFontCollection, L"Tahoma", &index, &exists);
        //hr = IDWriteFontCollection1_FindFamilyName(pImmersiveSwapchain->pFontCollection, L"Century", &index, &exists);
        //hr = IDWriteFontCollection1_FindFamilyName(pImmersiveSwapchain->pFontCollection, L"Segoe UI Variable Text", &index, &exists);
        hr = IDWriteFontCollection1_FindFamilyName(pImmersiveSwapchain->pFontCollection, L"Segoe UI Symbol", &index, &exists);
        ASSERT_W32(S_OK == hr);
        hr = IDWriteFontCollection1_GetFontFamily1(pImmersiveSwapchain->pFontCollection, index, &pImmersiveSwapchain->pFontFamily2);
        ASSERT_W32(S_OK == hr);
        hr = IDWriteFontFamily1_GetFirstMatchingFont(pImmersiveSwapchain->pFontFamily2, DWRITE_FONT_WEIGHT_THIN, DWRITE_FONT_STRETCH_UNDEFINED, DWRITE_FONT_STYLE_NORMAL, &pImmersiveSwapchain->pFont2);
        ASSERT_W32(S_OK == hr);
        hr = IDWriteFont1_CreateFontFace(pImmersiveSwapchain->pFont2, &pImmersiveSwapchain->pFontFace2);
        ASSERT_W32(S_OK == hr);
      
        pImmersiveSwapchain->pCodePointsMDL22[0] = CP_SYM_DARKMODE;
        pImmersiveSwapchain->pCodePointsMDL22[1] = CP_SYM_LIGHTMODE;
        pImmersiveSwapchain->pCodePointsMDL22[2] = CP_SYM_LIGHTMODE2;
        hr = IDWriteFontFace1_GetGlyphIndices(pImmersiveSwapchain->pFontFace2, &pImmersiveSwapchain->pCodePointsMDL22[0], 3, &pImmersiveSwapchain->pGlyphIndicesMDL22[0]);
      }
    }
    hr = IDWriteFactory1_CreateTextFormat(
      pImmersiveSwapchain->pDWriteFactory1,
      L"Segoe MDL2 Assets",
      pImmersiveSwapchain->pFontCollection,
      DWRITE_FONT_WEIGHT_THIN,
      DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL,
      48.0F,
      L"",
      &pImmersiveSwapchain->pTextFormat
    );
    ASSERT_W32(S_OK == hr);
    //hr = IDWriteFactory1_CreateTextFormat(
    //  pImmersiveSwapchain->pDWriteFactory1,
    //  L"Segoe UI Symbol",
    //  pImmersiveSwapchain->pFontCollection,
    //  DWRITE_FONT_WEIGHT_THIN,
    //  DWRITE_FONT_STYLE_NORMAL,
    //  DWRITE_FONT_STRETCH_NORMAL,
    //  48.0F,
    //  L"",
    //  &pImmersiveSwapchain->pTextFormat2
    //);

#if 0
    WCHAR szImage[_MAX_PATH];
    DWORD cchImage = _countof(szImage);
    QueryFullProcessImageNameW(GetCurrentProcess(), 0, szImage, &cchImage);

    HICON hIcon =
      //LoadImage(0, L".\\aot.ico", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE | LR_SHARED);
      ExtractIcon(&__ImageBase, szImage, 0);
    SendMessage(hWnd, WM_SETICON, ICON_BIG, hIcon);
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, hIcon);

    ICONINFOEX ii;
    ii.cbSize = sizeof(ii);
    
    BITMAP bm;
    
    GetIconInfoEx(hIcon, &ii);
    GetObject(ii.hbmColor, sizeof(bm), &bm);
    HDC hScreenDC = GetDC(0);
    HDC hMemDC = CreateCompatibleDC(hScreenDC);
    ReleaseDC(0, hScreenDC);

    BITMAPINFO bi = { 0 };
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = bm.bmWidth;
    bi.bmiHeader.biHeight = -bm.bmHeight; // top-down
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    void* pBits = NULL;
    HBITMAP hDIB = CreateDIBSection(hMemDC, &bi, DIB_RGB_COLORS, &pBits, NULL, 0);
    HBITMAP hOld = (HBITMAP)SelectObject(hMemDC, hDIB);

    // draw icon into our ARGB DIB section
    DrawIconEx(hMemDC, 0, 0, hIcon, bm.bmWidth, bm.bmHeight, 0, NULL, DI_NORMAL);
    
    GdiFlush();

    ResizeD2DBitmap(pImmersiveSwapchain->pD2D1DeviceContext,
      &pImmersiveSwapchain->pD2D1BitmapAppIcon, pBits, bm.bmWidth, bm.bmHeight);

    DeleteObject(hMemDC);
#endif
    FLOAT black[4] = { 0.0F, 0.0F, 0.0F, 0.0F };

    return hr;
}

EXTERN_C
VOID PFORCEINLINE APIPRIVATE
DrawImmersiveLightDarkButton(ID2D1DeviceContext* pD2D1DeviceContext, RECT rcCaption, RECT rcLightDarkBtn,
  ID2D1SolidColorBrush* fontBrush, ID2D1StrokeStyle* fontStroke, ID2D1SolidColorBrush* solidColorBrush,
  DWRITE_GLYPH_RUN* glyphRun, IDWriteFontFace1* pFontFace, UINT16* pGlyphIndicesMDL2,
  IDWriteFontFace1* pFontFace2, UINT16* pGlyphIndicesMDL22, INT glyphInset,
  FLOAT fontSize, FLOAT fontScale, BOOL fLightMode, CAPTIONBUTTON eHotCaptionButton)
{
  D2D_POINT_2F pos2;
  D2D_RECT_F rcfLightDarkButton;

  if (fLightMode)
  {
    glyphRun->fontFace = pFontFace;
    glyphRun->glyphIndices = &pGlyphIndicesMDL2[GLYPH_INDEX_DARK + 2];
  }
  else
  {
    glyphRun->fontFace = pFontFace;
    glyphRun->glyphIndices = &pGlyphIndicesMDL2[3];
  }

  glyphRun->fontEmSize = fontSize * fontScale;

  RECTTOLOGICAL(rcLightDarkBtn);

  OffsetRect(&rcLightDarkBtn, (RECTWIDTH(rcCaption) - (4 * RECTWIDTH(rcLightDarkBtn))) - 2, 0);

  RECTTORECTF(rcLightDarkBtn, rcfLightDarkButton);

  pos2.x = rcfLightDarkButton.left + (0.5f * RECTWIDTH(rcfLightDarkButton)) - glyphInset;
  if (!fLightMode)
  {
    pos2.y = rcfLightDarkButton.top + (0.5f * RECTHEIGHT(rcfLightDarkButton)) + glyphInset;
  }
  else
  {
    pos2.y = rcfLightDarkButton.top + (0.5f * RECTHEIGHT(rcfLightDarkButton)) + glyphInset;
  }
  ID2D1DeviceContext_DrawGlyphRun(pD2D1DeviceContext, pos2, glyphRun, fontBrush, DWRITE_MEASURING_MODE_NATURAL);

  if (CB_LIGHTDARKBUTTON == eHotCaptionButton)
  {
    ID2D1DeviceContext_DrawRectangle(pD2D1DeviceContext, &rcfLightDarkButton, solidColorBrush, 1.0F, fontStroke);
    ID2D1DeviceContext_FillRectangle(pD2D1DeviceContext, &rcfLightDarkButton, solidColorBrush);
    ID2D1DeviceContext_DrawGlyphRun(pD2D1DeviceContext, pos2, glyphRun, fontBrush, DWRITE_MEASURING_MODE_NATURAL);
  }
}

EXTERN_C
VOID PFORCEINLINE APIPRIVATE
DrawImmersiveIconButton(ID2D1DeviceContext* pD2D1DeviceContext, ID2D1Bitmap* pD2D1BitmapAppIcon, LONG nWidth, LONG nHeight)
{
  D2D_RECT_F icon_src = { 0 };
  icon_src.right = 48;
  icon_src.bottom = 48;
  D2D_RECT_F icon_dst = { 0 };

  icon_dst.left   = 1;
  icon_dst.top    = 10;
  icon_dst.right  = icon_dst.left + nHeight - 16;
  icon_dst.bottom = icon_dst.top  + nHeight - 16;
  int iconW = GetSystemMetrics(SM_CXSMICON);
  int iconH = GetSystemMetrics(SM_CYSMICON);
  icon_dst.top = (0.5f * (nHeight - iconH)) + 2;
  icon_dst.left = 12.0;
  icon_dst.bottom = icon_dst.top + iconH - 3;
  icon_dst.right = icon_dst.left + iconW - 2;
  ID2D1DeviceContext_DrawBitmap1(pD2D1DeviceContext,
    pD2D1BitmapAppIcon, &icon_dst, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
    &icon_src, 0);

  UNREFERENCED_PARAMETER(nWidth);
}

EXTERN_C
VOID PFORCEINLINE APIPRIVATE
DrawImmersiveBackground(ID2D1DeviceContext* pD2D1DeviceContext, ID2D1SolidColorBrush* brush,
  ID2D1StrokeStyle* stroke, LONG rcLightDarkkBtnLeft, LONG rcCaptionLeft, LONG nWidth, LONG nHeight)
{
  D2D_RECT_F opaque_rect;

  opaque_rect.left   = 0.0F;
  opaque_rect.top    = 1.0F;
  //opaque_rect.left   = min(rcLightDarkkBtnLeft + 1.0F, CLAMP((0.30F * nWidth) + 16, -FLT_MAX, rcLightDarkkBtnLeft - rcCaptionLeft));
  //opaque_rect.left   = min(opaque_rect.left + 1.0F, 320.0F);
  //opaque_rect.bottom = nHeight + 32;
  opaque_rect.bottom = nHeight;
  opaque_rect.right  = nWidth;

  ID2D1DeviceContext_DrawRectangle(pD2D1DeviceContext, &opaque_rect, brush, 1.0F, stroke);
  ID2D1DeviceContext_FillRectangle(pD2D1DeviceContext, &opaque_rect, brush);
}

EXTERN_C
VOID PFORCEINLINE APIPRIVATE
DrawImmersiveWindowFrame(
  ID2D1DeviceContext* pD2D1DeviceContext,
  ID2D1SolidColorBrush* brush,
  ID2D1StrokeStyle* stroke,
  RECT rcCaption)
{
  D2D_POINT_2F left_top;
  D2D_POINT_2F right_top;
  D2D_RECT_F rcf;

  rcf.left = 0;
  rcf.right = RECTWIDTH(rcCaption);
  rcf.top = 0.0f;
  rcf.bottom = 0.0f;
  ID2D1DeviceContext_DrawRectangle(pD2D1DeviceContext, &rcf, brush, 1.0f, stroke);
}

EXTERN_C
VOID PFORCEINLINE APIPRIVATE
DrawImmersiveCloseButton(
    ID2D1DeviceContext* pD2D1DeviceContext,
    ID2D1SolidColorBrush* fontBrush,
    ID2D1SolidColorBrush* redBrush,
    ID2D1StrokeStyle* stroke,
    DWRITE_GLYPH_RUN* glyphRun,
    IDWriteFontFace1* pFontFace,
    UINT16* pGlyphIndicesMDL2,
    INT glyphInset,
    INT glyphInset2,
    FLOAT fontSize,
    FLOAT fontScale,
    RECT rcCaption,
    RECT rcCloseButton,
    CAPTIONBUTTON eHotCaptionButton)
{
  D2D_RECT_F rcfCaptionButtonClose;
  D2D_POINT_2F pos;

  RECTTOLOGICAL(rcCaption);
  RECTTOLOGICAL(rcCloseButton);

  OffsetRect(&rcCloseButton, (RECTWIDTH(rcCaption) - RECTWIDTH(rcCloseButton)) - 1, 0);

  RECTTORECTF(rcCloseButton, rcfCaptionButtonClose);

  if (CB_CLOSEBUTTON == eHotCaptionButton)
  {
    rcfCaptionButtonClose.left  += 0.5F;
    rcfCaptionButtonClose.right += 0.5F;

    ID2D1DeviceContext_DrawRectangle(pD2D1DeviceContext, &rcfCaptionButtonClose, redBrush, 1.0F, stroke);
    ID2D1DeviceContext_FillRectangle(pD2D1DeviceContext, &rcfCaptionButtonClose, redBrush);

    glyphRun->fontFace     = pFontFace;
    glyphRun->fontEmSize   = fontSize * 1.25F;
    glyphRun->glyphIndices = &pGlyphIndicesMDL2[GLYPH_INDEX_CLOSE];

    pos.x = rcCloseButton.left + (0.5F * (RECTWIDTH(rcCloseButton) - 1)) - glyphInset;
    pos.y = rcCloseButton.top  + (0.5F * RECTHEIGHT(rcCloseButton)) + glyphInset2 - 1;
    ID2D1DeviceContext_DrawGlyphRun(pD2D1DeviceContext, pos, glyphRun, fontBrush, DWRITE_MEASURING_MODE_GDI_NATURAL);
    ID2D1DeviceContext_DrawGlyphRun(pD2D1DeviceContext, pos, glyphRun, fontBrush, DWRITE_MEASURING_MODE_GDI_NATURAL);
  }
  else
  {
    glyphRun->fontFace     = pFontFace;
    glyphRun->fontEmSize   = fontSize * 1.25F;
    glyphRun->glyphIndices = &pGlyphIndicesMDL2[GLYPH_INDEX_CLOSE];

    pos.x = rcCloseButton.left + (0.5F * (RECTWIDTH(rcCloseButton) - 1)) - glyphInset;
    pos.y = rcCloseButton.top + (0.5F * RECTHEIGHT(rcCloseButton)) + glyphInset2 - 1;
    ID2D1DeviceContext_DrawGlyphRun(pD2D1DeviceContext, pos, glyphRun, fontBrush, DWRITE_MEASURING_MODE_GDI_NATURAL);
    ID2D1DeviceContext_DrawGlyphRun(pD2D1DeviceContext, pos, glyphRun, fontBrush, DWRITE_MEASURING_MODE_GDI_NATURAL);
  }
}

EXTERN_C
VOID PFORCEINLINE APIPRIVATE
DrawImmersiveMaximizeButton(
  ID2D1DeviceContext* pD2D1DeviceContext,
  ID2D1SolidColorBrush* fontBrush,
  ID2D1SolidColorBrush* brush,
  ID2D1StrokeStyle* stroke,
  DWRITE_GLYPH_RUN* glyphRun,
  IDWriteFontFace1* pFontFace,
  UINT16* pGlyphIndicesMDL2,
  INT glyphInset,
  INT glyphInset2,
  FLOAT fontSize,
  FLOAT fontScale,
  UINT nDpi,
  RECT rcCaption,
  RECT rcMaximizeButton, 
  BOOL isMaximized,
  CAPTIONBUTTON eHotCaptionButton)
{
  SIZE szSymbolPadding;
  D2D_RECT_F rcfCaptionButtonMax;
  D2D_RECT_F rcfCaptionButtonMaxSymbol;
  D2D_POINT_2F pos;

  RECTTOLOGICAL(rcMaximizeButton);

  OffsetRect(&rcMaximizeButton, (RECTWIDTH(rcCaption) - (2 * RECTWIDTH(rcMaximizeButton))) - 2, 0);

  RECTTORECTF(rcMaximizeButton, rcfCaptionButtonMax);

  szSymbolPadding.cx = MulDiv(18, nDpi, 96);
  szSymbolPadding.cy = MulDiv(10, nDpi, 96);
  InsetRect(&rcMaximizeButton, szSymbolPadding.cx, szSymbolPadding.cy);
  //rcCaptionButton.top   += 0.5f;
  rcMaximizeButton.bottom -= 1.5f;
  rcMaximizeButton.right  -= 1;
  rcMaximizeButton.left   -= 1;
  RECTTORECTF(rcMaximizeButton, rcfCaptionButtonMaxSymbol);

  if (isMaximized)
  {
    if (CB_MAXBUTTON == eHotCaptionButton)
    {
      ID2D1DeviceContext_DrawRectangle(pD2D1DeviceContext, &rcfCaptionButtonMax, brush, 1.0F, stroke);
      ID2D1DeviceContext_FillRectangle(pD2D1DeviceContext, &rcfCaptionButtonMax, brush);
    }

    glyphRun->fontFace     = pFontFace;
    glyphRun->fontEmSize   = fontSize;
    glyphRun->glyphIndices = &pGlyphIndicesMDL2[12];

    pos.x = rcMaximizeButton.left;
    pos.y = rcMaximizeButton.top + (0.5F * RECTHEIGHT(rcMaximizeButton)) + glyphInset;
    ID2D1DeviceContext_DrawGlyphRun(pD2D1DeviceContext, pos, glyphRun, fontBrush, DWRITE_MEASURING_MODE_NATURAL);
  }
  else
  {
    if (CB_MAXBUTTON == eHotCaptionButton)
    {
      ID2D1DeviceContext_DrawRectangle(pD2D1DeviceContext, &rcfCaptionButtonMax, brush, 1.0F, stroke);
      ID2D1DeviceContext_FillRectangle(pD2D1DeviceContext, &rcfCaptionButtonMax, brush);

      glyphRun->fontFace     = pFontFace;
      glyphRun->fontEmSize   = fontSize;
      glyphRun->glyphIndices = &pGlyphIndicesMDL2[11];

      pos.x = rcMaximizeButton.left;
      pos.y = rcMaximizeButton.top + (0.5f * RECTHEIGHT(rcMaximizeButton)) + glyphInset;
      ID2D1DeviceContext_DrawGlyphRun(pD2D1DeviceContext, pos, glyphRun, fontBrush, DWRITE_MEASURING_MODE_NATURAL);
    }
    else
    {
      glyphRun->fontFace     = pFontFace;
      glyphRun->fontEmSize   = fontSize;
      glyphRun->glyphIndices = &pGlyphIndicesMDL2[11];

      pos.x = rcMaximizeButton.left;
      pos.y = rcMaximizeButton.top + (0.5f * RECTHEIGHT(rcMaximizeButton)) + glyphInset;
      ID2D1DeviceContext_DrawGlyphRun(pD2D1DeviceContext, pos, glyphRun, fontBrush, DWRITE_MEASURING_MODE_NATURAL);
    }
  }
}

EXTERN_C
VOID PFORCEINLINE APIPRIVATE
DrawImmersiveMinimizeButton(
    ID2D1DeviceContext* pD2D1DeviceContext,
    ID2D1SolidColorBrush* fontBrush,
    ID2D1SolidColorBrush* brush,
    ID2D1StrokeStyle* stroke,
    DWRITE_GLYPH_RUN* glyphRun,
    IDWriteFontFace1* pFontFace,
    UINT16* pGlyphIndicesMDL2,
    INT glyphInset,
    INT glyphInset2,
    FLOAT fontSize,
    FLOAT fontScale,
    UINT nDpi,
    RECT rcCaption,
    RECT rcMinimizeButton,
    CAPTIONBUTTON eHotCaptionButton)
{
  D2D_POINT_2F pos2;
  D2D_RECT_F rcfCaptionButtonMin;
  D2D_RECT_F rcfCaptionButtonMinSymbol;

  SIZE szSymbolPadding;
  RECTTOLOGICAL(rcMinimizeButton);

  OffsetRect(&rcMinimizeButton, (RECTWIDTH(rcCaption) - (3 * RECTWIDTH(rcMinimizeButton))) - 3, 0);

  RECTTORECTF(rcMinimizeButton, rcfCaptionButtonMin);

  int nWidthSymbol  = MulDiv(9, nDpi, 96);
  int nHeightSymbol = MulDiv(0, nDpi, 96);
  szSymbolPadding.cx = (LONG)((0.5F * MulDiv(45, nDpi, 96)) - (0.5F * nWidthSymbol));
  szSymbolPadding.cy = (LONG)((0.5F * MulDiv(30, nDpi, 96)) - (0.5F * nHeightSymbol));

  InsetRect(&rcMinimizeButton, szSymbolPadding.cx, szSymbolPadding.cy);

  RECTTORECTF(rcMinimizeButton, rcfCaptionButtonMinSymbol);

  if (CB_MINBUTTON == eHotCaptionButton)
  {
    glyphRun->fontEmSize   = fontSize;
    glyphRun->fontFace     = pFontFace;
    glyphRun->glyphIndices = &pGlyphIndicesMDL2[10];

    pos2.x = rcfCaptionButtonMin.left + (0.5F * RECTWIDTH(rcfCaptionButtonMin))  - glyphInset;
    pos2.y = rcfCaptionButtonMin.top  + (0.5F * RECTHEIGHT(rcfCaptionButtonMin)) + glyphInset;

    ID2D1DeviceContext_DrawRectangle(pD2D1DeviceContext, &rcfCaptionButtonMin, brush, 1.0F, stroke);
    ID2D1DeviceContext_FillRectangle(pD2D1DeviceContext, &rcfCaptionButtonMin, brush);
    ID2D1DeviceContext_DrawGlyphRun(pD2D1DeviceContext, pos2, glyphRun, fontBrush, DWRITE_MEASURING_MODE_NATURAL);
  }
  else
  {
    glyphRun->fontEmSize   = fontSize;
    glyphRun->fontFace     = pFontFace;
    glyphRun->glyphIndices = &pGlyphIndicesMDL2[10];
    pos2.x = rcfCaptionButtonMin.left + (0.5F * RECTWIDTH(rcfCaptionButtonMin))  - glyphInset;
    pos2.y = rcfCaptionButtonMin.top  + (0.5F * RECTHEIGHT(rcfCaptionButtonMin)) + glyphInset;
    ID2D1DeviceContext_DrawGlyphRun(pD2D1DeviceContext, pos2, glyphRun, fontBrush, DWRITE_MEASURING_MODE_NATURAL);
  }
}

EXTERN_C VOID WINAPI hol_up(HWND hWnd)
{
  //IDXGIImmersiveSwapchain pImmersiveSwapchain =
  //  (IDXGIImmersiveSwapchain)GetWindowLongPtr(hWnd, offsetof(IMMERSIVEDATA, pSwapchain));
  //WaitForSingleObject(pImmersiveSwapchain->hFrameLatencyWaitableObject, 0);
}

EXTERN_C
NTSTATUS PFORCEINLINE WINAPI
D3DKMTInitVerticalBlankEvent(
    HWND                              hWnd,
    D3DKMT_WAITFORVERTICALBLANKEVENT* pVbe)
{
  NTSTATUS status;
  D3DKMT_OPENADAPTERFROMHDC oa = { GetDC(hWnd) };

  status = D3DKMTOpenAdapterFromHdc(&oa);
  
  if (0 == status)
  {
    pVbe->hAdapter = oa.hAdapter;
    pVbe->VidPnSourceId = oa.VidPnSourceId;
    pVbe->hDevice = 0;
  }

  ReleaseDC(hWnd, oa.hDc);

  return status;
}

EXTERN_C
BOOL PFORCEINLINE WINAPI
D3DKMTWaitForVerticalBlank(
    D3DKMT_WAITFORVERTICALBLANKEVENT* pVbe)
{
    NTSTATUS status;
    //D3DKMT_GETSCANLINE gsl;

    if ((!pVbe) || (!pVbe->hAdapter))
      return FALSE;

    status = D3DKMTWaitForVerticalBlankEvent(pVbe);

    if (0 != status)
      return FALSE;

    //gsl = (D3DKMT_GETSCANLINE){ pVbe->hAdapter, pVbe->VidPnSourceId, 0, 0 };
    //
    //do
    //{
    //  status = D3DKMTGetScanLine(&gsl);
    //
    //} while ((0 != status) || (gsl.InVerticalBlank));

    return TRUE;
}

EXTERN_C
HRESULT PFORCEINLINE WINAPI
DXGIImmersiveSwapchain_Init(
    DXGIImmersiveSwapchain* pImmersiveSwapchain,
    HWND                    hwnd)
{
    HRESULT hr = E_INVALIDARG;

    if (pImmersiveSwapchain && hwnd)
    {
      if (0 == D3DKMTInitVerticalBlankEvent(hwnd, &pImmersiveSwapchain->vbe))
      {
        hr = InitD2D(pImmersiveSwapchain, hwnd);
        pImmersiveSwapchain->hwnd = hwnd;
      }
    }

    return hr;
}

EXTERN_C
VOID PFORCEINLINE WINAPI
DXGIImmersiveSwapchain_Shutdown(
    DXGIImmersiveSwapchain* pImmersiveSwapchain)
{
    DestroySwapchain(pImmersiveSwapchain);
}

EXTERN_C
BOOL PFORCEINLINE WINAPI
DXGIImmersiveSwapchain_WaitForVerticalBlank(
    DXGIImmersiveSwapchain* pImmersiveSwapchain)
{
    if (pImmersiveSwapchain)
      return D3DKMTWaitForVerticalBlank(&pImmersiveSwapchain->vbe);
    return FALSE;
}

EXTERN_C
BOOL PFORCEINLINE WINAPI
DXGIImmersiveSwapchain_Draw(
    DXGIImmersiveSwapchain* pImmersiveSwapchain)
{
    return BeginImmersivePaintInternal(pImmersiveSwapchain);
}

EXTERN_C
HRESULT PFORCEINLINE WINAPI
DXGIImmersiveSwapchain_Present(
    DXGIImmersiveSwapchain* pImmersiveSwapchain,
    BOOL                    fRestart,
    BOOL                    fVsync)
{
    /* 1. Do any pre-present stuff */
    
    //if (pImmersiveSwapchain->pProc)
    //  pImmersiveSwapchain->pProc(pImmersiveSwapchain->hwnd);

    /* 2. Present the D2D front buffer */
    {
      ASSERT_W32(SUCCEEDED(IDXGISwapChain2_Present(
        pImmersiveSwapchain->pDXGISwapchain2,
        0,                                                         /* No sync interval */
        (DXGI_PRESENT_ALLOW_TEARING | DXGI_PRESENT_DO_NOT_WAIT |   /* Present immediately */
          (fRestart * (DXGI_PRESENT_RESTART)))                     /* If restarting this frame, cancel any queued frame */
      )));
    }

    /* 3. Present either: */
    {
        if (fRestart || fVsync)
        {
          /* 3 (cont). The current D2D back buffer */
          ASSERT_W32(SUCCEEDED(IDXGISwapChain2_Present(
            pImmersiveSwapchain->pDXGISwapchain2,
            1,                                                       /* Sync this present to the next frame */
            DXGI_PRESENT_DO_NOT_SEQUENCE                             /* Present the current buffer, *ahead and instead of,* the would-be next back buffer */
          )));
        }
        else
        {
          /* 3 (cont). The next D2D back buffer */
          ASSERT_W32(SUCCEEDED(IDXGISwapChain2_Present(
            pImmersiveSwapchain->pDXGISwapchain2,
            0,                                                       /* No sync interval */
            (DXGI_PRESENT_ALLOW_TEARING | DXGI_PRESENT_DO_NOT_WAIT)  /* Present immediately */
          )));
        }
    }

    EnableBlurBehind(pImmersiveSwapchain->hwnd);

    return S_OK;
}

EXTERN_C
HRESULT PFORCEINLINE WINAPI
DXGIImmersiveSwapchain_Present2(
    DXGIImmersiveSwapchain* pImmersiveSwapchain,
    BOOL                    fRestart,
    BOOL                    fVsync)
{
    HRESULT hr;
    UINT sync = !!!fRestart;
    UINT flags = 0;
    DXGI_PRESENT_PARAMETERS presentParameters = { 0 };

    if (!fVsync)
      flags |= DXGI_PRESENT_RESTART | DXGI_PRESENT_ALLOW_TEARING;

    flags |= DXGI_PRESENT_DO_NOT_WAIT;

    if (fRestart)
      hr = IDXGISwapChain2_Present1(pImmersiveSwapchain->pDXGISwapchain2, sync, DXGI_PRESENT_DO_NOT_SEQUENCE, &presentParameters);
    hr = IDXGISwapChain2_Present1(pImmersiveSwapchain->pDXGISwapchain2, sync, flags, &presentParameters);

    return hr;

}