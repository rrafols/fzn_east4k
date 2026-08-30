%define OUT_DEFAULT_PRECIS          0
%define OUT_STRING_PRECIS           1
%define OUT_CHARACTER_PRECIS        2
%define OUT_STROKE_PRECIS           3
%define OUT_TT_PRECIS               4
%define OUT_DEVICE_PRECIS           5
%define OUT_RASTER_PRECIS           6
%define OUT_TT_ONLY_PRECIS          7
%define OUT_OUTLINE_PRECIS          8
%define OUT_SCREEN_OUTLINE_PRECIS   9

%define CLIP_DEFAULT_PRECIS     0
%define CLIP_CHARACTER_PRECIS   1
%define CLIP_STROKE_PRECIS      2
%define CLIP_MASK               0xf
%define CLIP_LH_ANGLES          (1<<4)
%define CLIP_TT_ALWAYS          (2<<4)
%define CLIP_EMBEDDED           (8<<4)

%define DEFAULT_QUALITY         0
%define DRAFT_QUALITY           1
%define PROOF_QUALITY           2
%define NONANTIALIASED_QUALITY  3
%define ANTIALIASED_QUALITY     4

%define DEFAULT_PITCH           0
%define FIXED_PITCH             1
%define VARIABLE_PITCH          2
%define MONO_FONT               8

%define ANSI_CHARSET            0
%define DEFAULT_CHARSET         1
%define SYMBOL_CHARSET          2
%define SHIFTJIS_CHARSET        128
%define HANGEUL_CHARSET         129
%define HANGUL_CHARSET          129
%define GB2312_CHARSET          134
%define CHINESEBIG5_CHARSET     136
%define OEM_CHARSET             255
%define JOHAB_CHARSET           130
%define HEBREW_CHARSET          177
%define ARABIC_CHARSET          178
%define GREEK_CHARSET           161
%define TURKISH_CHARSET         162
%define VIETNAMESE_CHARSET      163
%define THAI_CHARSET            222
%define EASTEUROPE_CHARSET      238
%define RUSSIAN_CHARSET         204

%define MAC_CHARSET             77
%define BALTIC_CHARSET          186

%define FS_LATIN1               0x00000001
%define FS_LATIN2               0x00000002
%define FS_CYRILLIC             0x00000004
%define FS_GREEK                0x00000008
%define FS_TURKISH              0x00000010
%define FS_HEBREW               0x00000020
%define FS_ARABIC               0x00000040
%define FS_BALTIC               0x00000080
%define FS_VIETNAMESE           0x00000100
%define FS_THAI                 0x00010000
%define FS_JISJAPAN             0x00020000
%define FS_CHINESESIMP          0x00040000
%define FS_WANSUNG              0x00080000
%define FS_CHINESETRAD          0x00100000
%define FS_JOHAB                0x00200000
%define FS_SYMBOL               0x80000000

; Font Families */
%define FF_DONTCARE         (0<<4)  ;; Don't care or don't know. */
%define FF_ROMAN            (1<<4)  ;; Variable stroke width, serifed. */
                                    ;; Times Roman, Century Schoolbook, etc. */
%define FF_SWISS            (2<<4)  ;; Variable stroke width, sans-serifed. */
                                    ;; Helvetica, Swiss, etc. */
%define FF_MODERN           (3<<4)  ;; Constant stroke width, serifed or sans-serifed. */
                                    ;; Pica, Elite, Courier, etc. */
%define FF_SCRIPT           (4<<4)  ;; Cursive, etc. */
%define FF_DECORATIVE       (5<<4)  ;; Old English, etc. */

; Font Weights */
%define FW_DONTCARE         0
%define FW_THIN             100
%define FW_EXTRALIGHT       200
%define FW_LIGHT            300
%define FW_NORMAL           400
%define FW_MEDIUM           500
%define FW_SEMIBOLD         600
%define FW_BOLD             700
%define FW_EXTRABOLD        800
%define FW_HEAVY            900

%define FW_ULTRALIGHT       FW_EXTRALIGHT
%define FW_REGULAR          FW_NORMAL
%define FW_DEMIBOLD         FW_SEMIBOLD
%define FW_ULTRABOLD        FW_EXTRABOLD
%define FW_BLACK            FW_HEAVY

%define PANOSE_COUNT               10
%define PAN_FAMILYTYPE_INDEX        0
%define PAN_SERIFSTYLE_INDEX        1
%define PAN_WEIGHT_INDEX            2
%define PAN_PROPORTION_INDEX        3
%define PAN_CONTRAST_INDEX          4
%define PAN_STROKEVARIATION_INDEX   5
%define PAN_ARMSTYLE_INDEX          6
%define PAN_LETTERFORM_INDEX        7
%define PAN_MIDLINE_INDEX           8
%define PAN_XHEIGHT_INDEX           9

%define PAN_CULTURE_LATIN           0

%define PAN_ANY                         0 ; Any                            */
%define PAN_NO_FIT                      1 ; No Fit                         */

%define PAN_FAMILY_TEXT_DISPLAY         2 ; Text and Display               */
%define PAN_FAMILY_SCRIPT               3 ; Script                         */
%define PAN_FAMILY_DECORATIVE           4 ; Decorative                     */
%define PAN_FAMILY_PICTORIAL            5 ; Pictorial                      */

%define PAN_SERIF_COVE                  2 ; Cove                           */
%define PAN_SERIF_OBTUSE_COVE           3 ; Obtuse Cove                    */
%define PAN_SERIF_SQUARE_COVE           4 ; Square Cove                    */
%define PAN_SERIF_OBTUSE_SQUARE_COVE    5 ; Obtuse Square Cove             */
%define PAN_SERIF_SQUARE                6 ; Square                         */
%define PAN_SERIF_THIN                  7 ; Thin                           */
%define PAN_SERIF_BONE                  8 ; Bone                           */
%define PAN_SERIF_EXAGGERATED           9 ; Exaggerated                    */
%define PAN_SERIF_TRIANGLE             10 ; Triangle                       */
%define PAN_SERIF_NORMAL_SANS          11 ; Normal Sans                    */
%define PAN_SERIF_OBTUSE_SANS          12 ; Obtuse Sans                    */
%define PAN_SERIF_PERP_SANS            13 ; Prep Sans                      */
%define PAN_SERIF_FLARED               14 ; Flared                         */
%define PAN_SERIF_ROUNDED              15 ; Rounded                        */

%define PAN_WEIGHT_VERY_LIGHT           2 ; Very Light                     */
%define PAN_WEIGHT_LIGHT                3 ; Light                          */
%define PAN_WEIGHT_THIN                 4 ; Thin                           */
%define PAN_WEIGHT_BOOK                 5 ; Book                           */
%define PAN_WEIGHT_MEDIUM               6 ; Medium                         */
%define PAN_WEIGHT_DEMI                 7 ; Demi                           */
%define PAN_WEIGHT_BOLD                 8 ; Bold                           */
%define PAN_WEIGHT_HEAVY                9 ; Heavy                          */
%define PAN_WEIGHT_BLACK               10 ; Black                          */
%define PAN_WEIGHT_NORD                11 ; Nord                           */

%define PAN_PROP_OLD_STYLE              2 ; Old Style                      */
%define PAN_PROP_MODERN                 3 ; Modern                         */
%define PAN_PROP_EVEN_WIDTH             4 ; Even Width                     */
%define PAN_PROP_EXPANDED               5 ; Expanded                       */
%define PAN_PROP_CONDENSED              6 ; Condensed                      */
%define PAN_PROP_VERY_EXPANDED          7 ; Very Expanded                  */
%define PAN_PROP_VERY_CONDENSED         8 ; Very Condensed                 */
%define PAN_PROP_MONOSPACED             9 ; Monospaced                     */

%define PAN_CONTRAST_NONE               2 ; None                           */
%define PAN_CONTRAST_VERY_LOW           3 ; Very Low                       */
%define PAN_CONTRAST_LOW                4 ; Low                            */
%define PAN_CONTRAST_MEDIUM_LOW         5 ; Medium Low                     */
%define PAN_CONTRAST_MEDIUM             6 ; Medium                         */
%define PAN_CONTRAST_MEDIUM_HIGH        7 ; Mediim High                    */
%define PAN_CONTRAST_HIGH               8 ; High                           */
%define PAN_CONTRAST_VERY_HIGH          9 ; Very High                      */

%define PAN_STROKE_GRADUAL_DIAG         2 ; Gradual/Diagonal               */
%define PAN_STROKE_GRADUAL_TRAN         3 ; Gradual/Transitional           */
%define PAN_STROKE_GRADUAL_VERT         4 ; Gradual/Vertical               */
%define PAN_STROKE_GRADUAL_HORZ         5 ; Gradual/Horizontal             */
%define PAN_STROKE_RAPID_VERT           6 ; Rapid/Vertical                 */
%define PAN_STROKE_RAPID_HORZ           7 ; Rapid/Horizontal               */
%define PAN_STROKE_INSTANT_VERT         8 ; Instant/Vertical               */

%define PAN_STRAIGHT_ARMS_HORZ          2 ; Straight Arms/Horizontal       */
%define PAN_STRAIGHT_ARMS_WEDGE         3 ; Straight Arms/Wedge            */
%define PAN_STRAIGHT_ARMS_VERT          4 ; Straight Arms/Vertical         */
%define PAN_STRAIGHT_ARMS_SINGLE_SERIF  5 ; Straight Arms/Single-Serif     */
%define PAN_STRAIGHT_ARMS_DOUBLE_SERIF  6 ; Straight Arms/Double-Serif     */
%define PAN_BENT_ARMS_HORZ              7 ; Non-Straight Arms/Horizontal   */
%define PAN_BENT_ARMS_WEDGE             8 ; Non-Straight Arms/Wedge        */
%define PAN_BENT_ARMS_VERT              9 ; Non-Straight Arms/Vertical     */
%define PAN_BENT_ARMS_SINGLE_SERIF     10 ; Non-Straight Arms/Single-Serif */
%define PAN_BENT_ARMS_DOUBLE_SERIF     11 ; Non-Straight Arms/Double-Serif */

%define PAN_LETT_NORMAL_CONTACT         2 ; Normal/Contact                 */
%define PAN_LETT_NORMAL_WEIGHTED        3 ; Normal/Weighted                */
%define PAN_LETT_NORMAL_BOXED           4 ; Normal/Boxed                   */
%define PAN_LETT_NORMAL_FLATTENED       5 ; Normal/Flattened               */
%define PAN_LETT_NORMAL_ROUNDED         6 ; Normal/Rounded                 */
%define PAN_LETT_NORMAL_OFF_CENTER      7 ; Normal/Off Center              */
%define PAN_LETT_NORMAL_SQUARE          8 ; Normal/Square                  */
%define PAN_LETT_OBLIQUE_CONTACT        9 ; Oblique/Contact                */
%define PAN_LETT_OBLIQUE_WEIGHTED      10 ; Oblique/Weighted               */
%define PAN_LETT_OBLIQUE_BOXED         11 ; Oblique/Boxed                  */
%define PAN_LETT_OBLIQUE_FLATTENED     12 ; Oblique/Flattened              */
%define PAN_LETT_OBLIQUE_ROUNDED       13 ; Oblique/Rounded                */
%define PAN_LETT_OBLIQUE_OFF_CENTER    14 ; Oblique/Off Center             */
%define PAN_LETT_OBLIQUE_SQUARE        15 ; Oblique/Square                 */

%define PAN_MIDLINE_STANDARD_TRIMMED    2 ; Standard/Trimmed               */
%define PAN_MIDLINE_STANDARD_POINTED    3 ; Standard/Pointed               */
%define PAN_MIDLINE_STANDARD_SERIFED    4 ; Standard/Serifed               */
%define PAN_MIDLINE_HIGH_TRIMMED        5 ; High/Trimmed                   */
%define PAN_MIDLINE_HIGH_POINTED        6 ; High/Pointed                   */
%define PAN_MIDLINE_HIGH_SERIFED        7 ; High/Serifed                   */
%define PAN_MIDLINE_CONSTANT_TRIMMED    8 ; Constant/Trimmed               */
%define PAN_MIDLINE_CONSTANT_POINTED    9 ; Constant/Pointed               */
%define PAN_MIDLINE_CONSTANT_SERIFED   10 ; Constant/Serifed               */
%define PAN_MIDLINE_LOW_TRIMMED        11 ; Low/Trimmed                    */
%define PAN_MIDLINE_LOW_POINTED        12 ; Low/Pointed                    */
%define PAN_MIDLINE_LOW_SERIFED        13 ; Low/Serifed                    */

%define PAN_XHEIGHT_CONSTANT_SMALL      2 ; Constant/Small                 */
%define PAN_XHEIGHT_CONSTANT_STD        3 ; Constant/Standard              */
%define PAN_XHEIGHT_CONSTANT_LARGE      4 ; Constant/Large                 */
%define PAN_XHEIGHT_DUCKING_SMALL       5 ; Ducking/Small                  */
%define PAN_XHEIGHT_DUCKING_STD         6 ; Ducking/Standard               */
%define PAN_XHEIGHT_DUCKING_LARGE       7 ; Ducking/Large                  */


%define ELF_VENDOR_SIZE     4

%define ELF_VERSION         0
%define ELF_CULTURE_LATIN   0

; EnumFonts Masks */
%define RASTER_FONTTYPE     0x0001
%define DEVICE_FONTTYPE     0x002
%define TRUETYPE_FONTTYPE   0x004

%define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
%define PALETTERGB(r,g,b)   (0x02000000 | RGB(r,g,b))
%define PALETTEINDEX(i)     ((COLORREF)(0x01000000 | (DWORD)(WORD)(i)))

; palette entry flags */

%define PC_RESERVED     0x01    ; palette index used for animation */
%define PC_EXPLICIT     0x02    ; palette index is explicit to device */
%define PC_NOCOLLAPSE   0x04    ; do not match color to system palette */

%define GetRValue(rgb)      ((BYTE)(rgb))
%define GetGValue(rgb)      ((BYTE)(((WORD)(rgb)) >> 8))
%define GetBValue(rgb)      ((BYTE)((rgb)>>16))

; Background Modes */
%define TRANSPARENT         1
%define OPAQUE              2
%define BKMODE_LAST         2

; Graphics Modes */

%define GM_COMPATIBLE       1
%define GM_ADVANCED         2
%define GM_LAST             2

; PolyDraw and GetPath point types */
%define PT_CLOSEFIGURE      0x01
%define PT_LINETO           0x02
%define PT_BEZIERTO         0x04
%define PT_MOVETO           0x06

; Mapping Modes */
%define MM_TEXT             1
%define MM_LOMETRIC         2
%define MM_HIMETRIC         3
%define MM_LOENGLISH        4
%define MM_HIENGLISH        5
%define MM_TWIPS            6
%define MM_ISOTROPIC        7
%define MM_ANISOTROPIC      8

; Min and Max Mapping Mode values */
%define MM_MIN              MM_TEXT
%define MM_MAX              MM_ANISOTROPIC
%define MM_MAX_FIXEDSCALE   MM_TWIPS

; Coordinate Modes */
%define ABSOLUTE            1
%define RELATIVE            2

; Stock Logical Objects */
%define WHITE_BRUSH         0
%define LTGRAY_BRUSH        1
%define GRAY_BRUSH          2
%define DKGRAY_BRUSH        3
%define BLACK_BRUSH         4
%define NULL_BRUSH          5
%define HOLLOW_BRUSH        NULL_BRUSH
%define WHITE_PEN           6
%define BLACK_PEN           7
%define NULL_PEN            8
%define OEM_FIXED_FONT      10
%define ANSI_FIXED_FONT     11
%define ANSI_VAR_FONT       12
%define SYSTEM_FONT         13
%define DEVICE_DEFAULT_FONT 14
%define DEFAULT_PALETTE     15
%define SYSTEM_FIXED_FONT   16

%define DEFAULT_GUI_FONT    17

%define DC_BRUSH            18
%define DC_PEN              19


%define STOCK_LAST          19

%define CLR_INVALID     0xFFFFFFFF

; Brush Styles */
%define BS_SOLID            0
%define BS_NULL             1
%define BS_HOLLOW           BS_NULL
%define BS_HATCHED          2
%define BS_PATTERN          3
%define BS_INDEXED          4
%define BS_DIBPATTERN       5
%define BS_DIBPATTERNPT     6
%define BS_PATTERN8X8       7
%define BS_DIBPATTERN8X8    8
%define BS_MONOPATTERN      9

; Hatch Styles */
%define HS_HORIZONTAL       0       ; ----- */
%define HS_VERTICAL         1       ; ||||| */
%define HS_FDIAGONAL        2       ; \\\\\ */
%define HS_BDIAGONAL        3       ; ;;;;/ */
%define HS_CROSS            4       ; +++++ */
%define HS_DIAGCROSS        5       ; xxxxx */

; Pen Styles */
%define PS_SOLID            0
%define PS_DASH             1       ; -------  */
%define PS_DOT              2       ; .......  */
%define PS_DASHDOT          3       ; _._._._  */
%define PS_DASHDOTDOT       4       ; _.._.._  */
%define PS_NULL             5
%define PS_INSIDEFRAME      6
%define PS_USERSTYLE        7
%define PS_ALTERNATE        8
%define PS_STYLE_MASK       0x0000000F

%define PS_ENDCAP_ROUND     0x00000000
%define PS_ENDCAP_SQUARE    0x00000100
%define PS_ENDCAP_FLAT      0x00000200
%define PS_ENDCAP_MASK      0x00000F00

%define PS_JOIN_ROUND       0x00000000
%define PS_JOIN_BEVEL       0x00001000
%define PS_JOIN_MITER       0x00002000
%define PS_JOIN_MASK        0x0000F000

%define PS_COSMETIC         0x00000000
%define PS_GEOMETRIC        0x00010000
%define PS_TYPE_MASK        0x000F0000

%define AD_COUNTERCLOCKWISE 1
%define AD_CLOCKWISE        2

; Device Parameters for GetDeviceCaps() */
%define DRIVERVERSION 0     ; Device driver version                    */
%define TECHNOLOGY    2     ; Device classification                    */
%define HORZSIZE      4     ; Horizontal size in millimeters           */
%define VERTSIZE      6     ; Vertical size in millimeters             */
%define HORZRES       8     ; Horizontal width in pixels               */
%define VERTRES       10    ; Vertical height in pixels                */
%define BITSPIXEL     12    ; Number of bits per pixel                 */
%define PLANES        14    ; Number of planes                         */
%define NUMBRUSHES    16    ; Number of brushes the device has         */
%define NUMPENS       18    ; Number of pens the device has            */
%define NUMMARKERS    20    ; Number of markers the device has         */
%define NUMFONTS      22    ; Number of fonts the device has           */
%define NUMCOLORS     24    ; Number of colors the device supports     */
%define PDEVICESIZE   26    ; Size required for device descriptor      */
%define CURVECAPS     28    ; Curve capabilities                       */
%define LINECAPS      30    ; Line capabilities                        */
%define POLYGONALCAPS 32    ; Polygonal capabilities                   */
%define TEXTCAPS      34    ; Text capabilities                        */
%define CLIPCAPS      36    ; Clipping capabilities                    */
%define RASTERCAPS    38    ; Bitblt capabilities                      */
%define ASPECTX       40    ; Length of the X leg                      */
%define ASPECTY       42    ; Length of the Y leg                      */
%define ASPECTXY      44    ; Length of the hypotenuse                 */


%define SHADEBLENDCAPS 45   ; Shading and blending caps                */

%define LOGPIXELSX    88    ; Logical pixels/inch in X                 */
%define LOGPIXELSY    90    ; Logical pixels/inch in Y                 */

%define SIZEPALETTE  104    ; Number of entries in physical palette    */
%define NUMRESERVED  106    ; Number of reserved entries in palette    */
%define COLORRES     108    ; Actual color resolution                  */


;; Printing related DeviceCaps. These replace the appropriate Escapes

%define PHYSICALWIDTH   110 ; Physical Width in device units           */
%define PHYSICALHEIGHT  111 ; Physical Height in device units          */
%define PHYSICALOFFSETX 112 ; Physical Printable Area x margin         */
%define PHYSICALOFFSETY 113 ; Physical Printable Area y margin         */
%define SCALINGFACTORX  114 ; Scaling factor x                         */
%define SCALINGFACTORY  115 ; Scaling factor y                         */

;; Display driver specific

%define VREFRESH        116  ; Current vertical refresh rate of the    */
                             ; display device (for displays only) in Hz*/
%define DESKTOPVERTRES  117  ; Horizontal width of entire desktop in   */
                             ; pixels                                  */
%define DESKTOPHORZRES  118  ; Vertical height of entire desktop in    */
                             ; pixels                                  */
%define BLTALIGNMENT    119  ; Preferred blt alignment                 */


; Device Capability Masks: */

; Device Technologies */
%define DT_PLOTTER          0   ; Vector plotter                   */
%define DT_RASDISPLAY       1   ; Raster display                   */
%define DT_RASPRINTER       2   ; Raster printer                   */
%define DT_RASCAMERA        3   ; Raster camera                    */
%define DT_CHARSTREAM       4   ; Character-stream, PLP            */
%define DT_METAFILE         5   ; Metafile, VDM                    */
%define DT_DISPFILE         6   ; Display-file                     */

; Curve Capabilities */
%define CC_NONE             0   ; Curves not supported             */
%define CC_CIRCLES          1   ; Can do circles                   */
%define CC_PIE              2   ; Can do pie wedges                */
%define CC_CHORD            4   ; Can do chord arcs                */
%define CC_ELLIPSES         8   ; Can do ellipese                  */
%define CC_WIDE             16  ; Can do wide lines                */
%define CC_STYLED           32  ; Can do styled lines              */
%define CC_WIDESTYLED       64  ; Can do wide styled lines         */
%define CC_INTERIORS        128 ; Can do interiors                 */
%define CC_ROUNDRECT        256 ;                                  */

; Line Capabilities */
%define LC_NONE             0   ; Lines not supported              */
%define LC_POLYLINE         2   ; Can do polylines                 */
%define LC_MARKER           4   ; Can do markers                   */
%define LC_POLYMARKER       8   ; Can do polymarkers               */
%define LC_WIDE             16  ; Can do wide lines                */
%define LC_STYLED           32  ; Can do styled lines              */
%define LC_WIDESTYLED       64  ; Can do wide styled lines         */
%define LC_INTERIORS        128 ; Can do interiors                 */

; Polygonal Capabilities */
%define PC_NONE             0   ; Polygonals not supported         */
%define PC_POLYGON          1   ; Can do polygons                  */
%define PC_RECTANGLE        2   ; Can do rectangles                */
%define PC_WINDPOLYGON      4   ; Can do winding polygons          */
%define PC_TRAPEZOID        4   ; Can do trapezoids                */
%define PC_SCANLINE         8   ; Can do scanlines                 */
%define PC_WIDE             16  ; Can do wide borders              */
%define PC_STYLED           32  ; Can do styled borders            */
%define PC_WIDESTYLED       64  ; Can do wide styled borders       */
%define PC_INTERIORS        128 ; Can do interiors                 */
%define PC_POLYPOLYGON      256 ; Can do polypolygons              */
%define PC_PATHS            512 ; Can do paths                     */

; Clipping Capabilities */
%define CP_NONE             0   ; No clipping of output            */
%define CP_RECTANGLE        1   ; Output clipped to rects          */
%define CP_REGION           2   ; obsolete                         */

; Text Capabilities */
%define TC_OP_CHARACTER     0x00000001  ; Can do OutputPrecision   CHARACTER      */
%define TC_OP_STROKE        0x00000002  ; Can do OutputPrecision   STROKE         */
%define TC_CP_STROKE        0x00000004  ; Can do ClipPrecision     STROKE         */
%define TC_CR_90            0x00000008  ; Can do CharRotAbility    90             */
%define TC_CR_ANY           0x00000010  ; Can do CharRotAbility    ANY            */
%define TC_SF_X_YINDEP      0x00000020  ; Can do ScaleFreedom      X_YINDEPENDENT */
%define TC_SA_DOUBLE        0x00000040  ; Can do ScaleAbility      DOUBLE         */
%define TC_SA_INTEGER       0x00000080  ; Can do ScaleAbility      INTEGER        */
%define TC_SA_CONTIN        0x00000100  ; Can do ScaleAbility      CONTINUOUS     */
%define TC_EA_DOUBLE        0x00000200  ; Can do EmboldenAbility   DOUBLE         */
%define TC_IA_ABLE          0x00000400  ; Can do ItalisizeAbility  ABLE           */
%define TC_UA_ABLE          0x00000800  ; Can do UnderlineAbility  ABLE           */
%define TC_SO_ABLE          0x00001000  ; Can do StrikeOutAbility  ABLE           */
%define TC_RA_ABLE          0x00002000  ; Can do RasterFontAble    ABLE           */
%define TC_VA_ABLE          0x00004000  ; Can do VectorFontAble    ABLE           */
%define TC_RESERVED         0x00008000
%define TC_SCROLLBLT        0x00010000  ; Don't do text scroll with blt           */


; Raster Capabilities */
%define RC_NONE
%define RC_BITBLT           1       ; Can do standard BLT.             */
%define RC_BANDING          2       ; Device requires banding support  */
%define RC_SCALING          4       ; Device requires scaling support  */
%define RC_BITMAP64         8       ; Device can support >64K bitmap   */
%define RC_GDI20_OUTPUT     0x0010      ; has 2.0 output calls         */
%define RC_GDI20_STATE      0x0020
%define RC_SAVEBITMAP       0x0040
%define RC_DI_BITMAP        0x0080      ; supports DIB to memory       */
%define RC_PALETTE          0x0100      ; supports a palette           */
%define RC_DIBTODEV         0x0200      ; supports DIBitsToDevice      */
%define RC_BIGFONT          0x0400      ; supports >64K fonts          */
%define RC_STRETCHBLT       0x0800      ; supports StretchBlt          */
%define RC_FLOODFILL        0x1000      ; supports FloodFill           */
%define RC_STRETCHDIB       0x2000      ; supports StretchDIBits       */
%define RC_OP_DX_OUTPUT     0x4000
%define RC_DEVBITS          0x8000


; Shading and blending caps                */
%define SB_NONE             0x00000000
%define SB_CONST_ALPHA      0x00000001
%define SB_PIXEL_ALPHA      0x00000002
%define SB_PREMULT_ALPHA    0x00000004

%define SB_GRAD_RECT        0x00000010
%define SB_GRAD_TRI         0x00000020



; DIB color table identifiers */

%define DIB_RGB_COLORS      0 ; color table in RGBs */
%define DIB_PAL_COLORS      1 ; color table in palette indices */

; constants for Get/SetSystemPaletteUse() */

%define SYSPAL_ERROR    0
%define SYSPAL_STATIC   1
%define SYSPAL_NOSTATIC 2
%define SYSPAL_NOSTATIC256 3

; constants for CreateDIBitmap */
%define CBM_INIT        0x04L   ; initialize bitmap */

; ExtFloodFill style flags */
%define  FLOODFILLBORDER   0
%define  FLOODFILLSURFACE  1

; size of a device name string */
%define CCHDEVICENAME 32

; size of a form name string */
%define CCHFORMNAME 32



%define DM_ORIENTATION      0x00000001
%define DM_PAPERSIZE        0x00000002
%define DM_PAPERLENGTH      0x00000004
%define DM_PAPERWIDTH       0x00000008
%define DM_SCALE            0x00000010
%define DM_POSITION         0x00000020
%define DM_COPIES           0x00000100
%define DM_DEFAULTSOURCE    0x00000200
%define DM_PRINTQUALITY     0x00000400
%define DM_COLOR            0x00000800
%define DM_DUPLEX           0x00001000
%define DM_YRESOLUTION      0x00002000
%define DM_TTOPTION         0x00004000
%define DM_COLLATE          0x00008000
%define DM_FORMNAME         0x00010000
%define DM_LOGPIXELS        0x00020000
%define DM_BITSPERPEL       0x00040000
%define DM_PELSWIDTH        0x00080000
%define DM_PELSHEIGHT       0x00100000
%define DM_DISPLAYFLAGS     0x00200000
%define DM_DISPLAYFREQUENCY 0x00400000
%define DM_ICMMETHOD        0x00800000
%define DM_ICMINTENT        0x01000000
%define DM_MEDIATYPE        0x02000000
%define DM_DITHERTYPE       0x04000000
%define DM_PANNINGWIDTH     0x08000000
%define DM_PANNINGHEIGHT    0x10000000

; orientation selections */
%define DMORIENT_PORTRAIT   1
%define DMORIENT_LANDSCAPE  2

; paper selections */
%define DMPAPER_FIRST                DMPAPER_LETTER
%define DMPAPER_LETTER               1  ; Letter 8 1/2 x 11 in               */
%define DMPAPER_LETTERSMALL          2  ; Letter Small 8 1/2 x 11 in         */
%define DMPAPER_TABLOID              3  ; Tabloid 11 x 17 in                 */
%define DMPAPER_LEDGER               4  ; Ledger 17 x 11 in                  */
%define DMPAPER_LEGAL                5  ; Legal 8 1/2 x 14 in                */
%define DMPAPER_STATEMENT            6  ; Statement 5 1/2 x 8 1/2 in         */
%define DMPAPER_EXECUTIVE            7  ; Executive 7 1/4 x 10 1/2 in        */
%define DMPAPER_A3                   8  ; A3 297 x 420 mm                    */
%define DMPAPER_A4                   9  ; A4 210 x 297 mm                    */
%define DMPAPER_A4SMALL             10  ; A4 Small 210 x 297 mm              */
%define DMPAPER_A5                  11  ; A5 148 x 210 mm                    */
%define DMPAPER_B4                  12  ; B4 (JIS) 250 x 354                 */
%define DMPAPER_B5                  13  ; B5 (JIS) 182 x 257 mm              */
%define DMPAPER_FOLIO               14  ; Folio 8 1/2 x 13 in                */
%define DMPAPER_QUARTO              15  ; Quarto 215 x 275 mm                */
%define DMPAPER_10X14               16  ; 10x14 in                           */
%define DMPAPER_11X17               17  ; 11x17 in                           */
%define DMPAPER_NOTE                18  ; Note 8 1/2 x 11 in                 */
%define DMPAPER_ENV_9               19  ; Envelope #9 3 7/8 x 8 7/8          */
%define DMPAPER_ENV_10              20  ; Envelope #10 4 1/8 x 9 1/2         */
%define DMPAPER_ENV_11              21  ; Envelope #11 4 1/2 x 10 3/8        */
%define DMPAPER_ENV_12              22  ; Envelope #12 4 \276 x 11           */
%define DMPAPER_ENV_14              23  ; Envelope #14 5 x 11 1/2            */
%define DMPAPER_CSHEET              24  ; C size sheet                       */
%define DMPAPER_DSHEET              25  ; D size sheet                       */
%define DMPAPER_ESHEET              26  ; E size sheet                       */
%define DMPAPER_ENV_DL              27  ; Envelope DL 110 x 220mm            */
%define DMPAPER_ENV_C5              28  ; Envelope C5 162 x 229 mm           */
%define DMPAPER_ENV_C3              29  ; Envelope C3  324 x 458 mm          */
%define DMPAPER_ENV_C4              30  ; Envelope C4  229 x 324 mm          */
%define DMPAPER_ENV_C6              31  ; Envelope C6  114 x 162 mm          */
%define DMPAPER_ENV_C65             32  ; Envelope C65 114 x 229 mm          */
%define DMPAPER_ENV_B4              33  ; Envelope B4  250 x 353 mm          */
%define DMPAPER_ENV_B5              34  ; Envelope B5  176 x 250 mm          */
%define DMPAPER_ENV_B6              35  ; Envelope B6  176 x 125 mm          */
%define DMPAPER_ENV_ITALY           36  ; Envelope 110 x 230 mm              */
%define DMPAPER_ENV_MONARCH         37  ; Envelope Monarch 3.875 x 7.5 in    */
%define DMPAPER_ENV_PERSONAL        38  ; 6 3/4 Envelope 3 5/8 x 6 1/2 in    */
%define DMPAPER_FANFOLD_US          39  ; US Std Fanfold 14 7/8 x 11 in      */
%define DMPAPER_FANFOLD_STD_GERMAN  40  ; German Std Fanfold 8 1/2 x 12 in   */
%define DMPAPER_FANFOLD_LGL_GERMAN  41  ; German Legal Fanfold 8 1/2 x 13 in */
%define DMPAPER_ISO_B4              42  ; B4 (ISO) 250 x 353 mm              */
%define DMPAPER_JAPANESE_POSTCARD   43  ; Japanese Postcard 100 x 148 mm     */
%define DMPAPER_9X11                44  ; 9 x 11 in                          */
%define DMPAPER_10X11               45  ; 10 x 11 in                         */
%define DMPAPER_15X11               46  ; 15 x 11 in                         */
%define DMPAPER_ENV_INVITE          47  ; Envelope Invite 220 x 220 mm       */
%define DMPAPER_RESERVED_48         48  ; RESERVED--DO NOT USE               */
%define DMPAPER_RESERVED_49         49  ; RESERVED--DO NOT USE               */
%define DMPAPER_LETTER_EXTRA        50  ; Letter Extra 9 \275 x 12 in        */
%define DMPAPER_LEGAL_EXTRA         51  ; Legal Extra 9 \275 x 15 in         */
%define DMPAPER_TABLOID_EXTRA       52  ; Tabloid Extra 11.69 x 18 in        */
%define DMPAPER_A4_EXTRA            53  ; A4 Extra 9.27 x 12.69 in           */
%define DMPAPER_LETTER_TRANSVERSE   54  ; Letter Transverse 8 \275 x 11 in   */
%define DMPAPER_A4_TRANSVERSE       55  ; A4 Transverse 210 x 297 mm         */
%define DMPAPER_LETTER_EXTRA_TRANSVERSE 56 ; Letter Extra Transverse 9\275 x 12 in */
%define DMPAPER_A_PLUS              57  ; SuperA/SuperA/A4 227 x 356 mm      */
%define DMPAPER_B_PLUS              58  ; SuperB/SuperB/A3 305 x 487 mm      */
%define DMPAPER_LETTER_PLUS         59  ; Letter Plus 8.5 x 12.69 in         */
%define DMPAPER_A4_PLUS             60  ; A4 Plus 210 x 330 mm               */
%define DMPAPER_A5_TRANSVERSE       61  ; A5 Transverse 148 x 210 mm         */
%define DMPAPER_B5_TRANSVERSE       62  ; B5 (JIS) Transverse 182 x 257 mm   */
%define DMPAPER_A3_EXTRA            63  ; A3 Extra 322 x 445 mm              */
%define DMPAPER_A5_EXTRA            64  ; A5 Extra 174 x 235 mm              */
%define DMPAPER_B5_EXTRA            65  ; B5 (ISO) Extra 201 x 276 mm        */
%define DMPAPER_A2                  66  ; A2 420 x 594 mm                    */
%define DMPAPER_A3_TRANSVERSE       67  ; A3 Transverse 297 x 420 mm         */
%define DMPAPER_A3_EXTRA_TRANSVERSE 68  ; A3 Extra Transverse 322 x 445 mm   */


%define DMPAPER_DBL_JAPANESE_POSTCARD 69 ; Japanese Double Postcard 200 x 148 mm */
%define DMPAPER_A6                  70  ; A6 105 x 148 mm                 */
%define DMPAPER_JENV_KAKU2          71  ; Japanese Envelope Kaku #2       */
%define DMPAPER_JENV_KAKU3          72  ; Japanese Envelope Kaku #3       */
%define DMPAPER_JENV_CHOU3          73  ; Japanese Envelope Chou #3       */
%define DMPAPER_JENV_CHOU4          74  ; Japanese Envelope Chou #4       */
%define DMPAPER_LETTER_ROTATED      75  ; Letter Rotated 11 x 8 1/2 11 in */
%define DMPAPER_A3_ROTATED          76  ; A3 Rotated 420 x 297 mm         */
%define DMPAPER_A4_ROTATED          77  ; A4 Rotated 297 x 210 mm         */
%define DMPAPER_A5_ROTATED          78  ; A5 Rotated 210 x 148 mm         */
%define DMPAPER_B4_JIS_ROTATED      79  ; B4 (JIS) Rotated 364 x 257 mm   */
%define DMPAPER_B5_JIS_ROTATED      80  ; B5 (JIS) Rotated 257 x 182 mm   */
%define DMPAPER_JAPANESE_POSTCARD_ROTATED 81 ; Japanese Postcard Rotated 148 x 100 mm */
%define DMPAPER_DBL_JAPANESE_POSTCARD_ROTATED 82 ; Double Japanese Postcard Rotated 148 x 200 mm */
%define DMPAPER_A6_ROTATED          83  ; A6 Rotated 148 x 105 mm         */
%define DMPAPER_JENV_KAKU2_ROTATED  84  ; Japanese Envelope Kaku #2 Rotated */
%define DMPAPER_JENV_KAKU3_ROTATED  85  ; Japanese Envelope Kaku #3 Rotated */
%define DMPAPER_JENV_CHOU3_ROTATED  86  ; Japanese Envelope Chou #3 Rotated */
%define DMPAPER_JENV_CHOU4_ROTATED  87  ; Japanese Envelope Chou #4 Rotated */
%define DMPAPER_B6_JIS              88  ; B6 (JIS) 128 x 182 mm           */
%define DMPAPER_B6_JIS_ROTATED      89  ; B6 (JIS) Rotated 182 x 128 mm   */
%define DMPAPER_12X11               90  ; 12 x 11 in                      */
%define DMPAPER_JENV_YOU4           91  ; Japanese Envelope You #4        */
%define DMPAPER_JENV_YOU4_ROTATED   92  ; Japanese Envelope You #4 Rotated*/
%define DMPAPER_P16K                93  ; PRC 16K 146 x 215 mm            */
%define DMPAPER_P32K                94  ; PRC 32K 97 x 151 mm             */
%define DMPAPER_P32KBIG             95  ; PRC 32K(Big) 97 x 151 mm        */
%define DMPAPER_PENV_1              96  ; PRC Envelope #1 102 x 165 mm    */
%define DMPAPER_PENV_2              97  ; PRC Envelope #2 102 x 176 mm    */
%define DMPAPER_PENV_3              98  ; PRC Envelope #3 125 x 176 mm    */
%define DMPAPER_PENV_4              99  ; PRC Envelope #4 110 x 208 mm    */
%define DMPAPER_PENV_5              100 ; PRC Envelope #5 110 x 220 mm    */
%define DMPAPER_PENV_6              101 ; PRC Envelope #6 120 x 230 mm    */
%define DMPAPER_PENV_7              102 ; PRC Envelope #7 160 x 230 mm    */
%define DMPAPER_PENV_8              103 ; PRC Envelope #8 120 x 309 mm    */
%define DMPAPER_PENV_9              104 ; PRC Envelope #9 229 x 324 mm    */
%define DMPAPER_PENV_10             105 ; PRC Envelope #10 324 x 458 mm   */
%define DMPAPER_P16K_ROTATED        106 ; PRC 16K Rotated                 */
%define DMPAPER_P32K_ROTATED        107 ; PRC 32K Rotated                 */
%define DMPAPER_P32KBIG_ROTATED     108 ; PRC 32K(Big) Rotated            */
%define DMPAPER_PENV_1_ROTATED      109 ; PRC Envelope #1 Rotated 165 x 102 mm */
%define DMPAPER_PENV_2_ROTATED      110 ; PRC Envelope #2 Rotated 176 x 102 mm */
%define DMPAPER_PENV_3_ROTATED      111 ; PRC Envelope #3 Rotated 176 x 125 mm */
%define DMPAPER_PENV_4_ROTATED      112 ; PRC Envelope #4 Rotated 208 x 110 mm */
%define DMPAPER_PENV_5_ROTATED      113 ; PRC Envelope #5 Rotated 220 x 110 mm */
%define DMPAPER_PENV_6_ROTATED      114 ; PRC Envelope #6 Rotated 230 x 120 mm */
%define DMPAPER_PENV_7_ROTATED      115 ; PRC Envelope #7 Rotated 230 x 160 mm */
%define DMPAPER_PENV_8_ROTATED      116 ; PRC Envelope #8 Rotated 309 x 120 mm */
%define DMPAPER_PENV_9_ROTATED      117 ; PRC Envelope #9 Rotated 324 x 229 mm */
%define DMPAPER_PENV_10_ROTATED     118 ; PRC Envelope #10 Rotated 458 x 324 mm */

%define DMPAPER_LAST                DMPAPER_PENV_10_ROTATED

%define DMPAPER_USER                256

; bin selections */
%define DMBIN_FIRST         DMBIN_UPPER
%define DMBIN_UPPER         1
%define DMBIN_ONLYONE       1
%define DMBIN_LOWER         2
%define DMBIN_MIDDLE        3
%define DMBIN_MANUAL        4
%define DMBIN_ENVELOPE      5
%define DMBIN_ENVMANUAL     6
%define DMBIN_AUTO          7
%define DMBIN_TRACTOR       8
%define DMBIN_SMALLFMT      9
%define DMBIN_LARGEFMT      10
%define DMBIN_LARGECAPACITY 11
%define DMBIN_CASSETTE      14
%define DMBIN_FORMSOURCE    15
%define DMBIN_LAST          DMBIN_FORMSOURCE

%define DMBIN_USER          256     ; device specific bins start here */

; print qualities */
%define DMRES_DRAFT         (-1)
%define DMRES_LOW           (-2)
%define DMRES_MEDIUM        (-3)
%define DMRES_HIGH          (-4)

; color enable/disable for color printers */
%define DMCOLOR_MONOCHROME  1
%define DMCOLOR_COLOR       2

; duplex enable */
%define DMDUP_SIMPLEX    1
%define DMDUP_VERTICAL   2
%define DMDUP_HORIZONTAL 3

; TrueType options */
%define DMTT_BITMAP     1       ; print TT fonts as graphics */
%define DMTT_DOWNLOAD   2       ; download TT fonts as soft fonts */
%define DMTT_SUBDEV     3       ; substitute device fonts for TT fonts */
%define DMTT_DOWNLOAD_OUTLINE 4 ; download TT fonts as outline soft fonts */

; Collation selections */
%define DMCOLLATE_FALSE  0
%define DMCOLLATE_TRUE   1

; DEVMODE dmDisplayFlags flags */

;; %define DM_GRAYSCALE            0x00000001 ; This flag is no longer valid */
;; %define DM_INTERLACED           0x00000002 ; This flag is no longer valid */
%define DMDISPLAYFLAGS_TEXTMODE 0x00000004


; ICM methods */
%define DMICMMETHOD_NONE    1   ; ICM disabled */
%define DMICMMETHOD_SYSTEM  2   ; ICM handled by system */
%define DMICMMETHOD_DRIVER  3   ; ICM handled by driver */
%define DMICMMETHOD_DEVICE  4   ; ICM handled by device */

%define DMICMMETHOD_USER  256   ; Device-specific methods start here */

; ICM Intents */
%define DMICM_SATURATE          1   ; Maximize color saturation */
%define DMICM_CONTRAST          2   ; Maximize color contrast */
%define DMICM_COLORIMETRIC       3   ; Use specific color metric */
%define DMICM_ABS_COLORIMETRIC   4   ; Use specific color metric */

%define DMICM_USER        256   ; Device-specific intents start here */

; Media types */

%define DMMEDIA_STANDARD      1   ; Standard paper */
%define DMMEDIA_TRANSPARENCY  2   ; Transparency */
%define DMMEDIA_GLOSSY        3   ; Glossy paper */

%define DMMEDIA_USER        256   ; Device-specific media start here */

; Dither types */
%define DMDITHER_NONE       1      ; No dithering */
%define DMDITHER_COARSE     2      ; Dither with a coarse brush */
%define DMDITHER_FINE       3      ; Dither with a fine brush */
%define DMDITHER_LINEART    4      ; LineArt dithering */
%define DMDITHER_ERRORDIFFUSION 5  ; LineArt dithering */
%define DMDITHER_RESERVED6      6      ; LineArt dithering */
%define DMDITHER_RESERVED7      7      ; LineArt dithering */
%define DMDITHER_RESERVED8      8      ; LineArt dithering */
%define DMDITHER_RESERVED9      9      ; LineArt dithering */
%define DMDITHER_GRAYSCALE  10     ; Device does grayscaling */

%define DMDITHER_USER     256   ; Device-specific dithers start here */

%define DISPLAY_DEVICE_ATTACHED_TO_DESKTOP 0x00000001
%define DISPLAY_DEVICE_MULTI_DRIVER        0x00000002
%define DISPLAY_DEVICE_PRIMARY_DEVICE      0x00000004
%define DISPLAY_DEVICE_MIRRORING_DRIVER    0x00000008
%define DISPLAY_DEVICE_VGA_COMPATIBLE      0x00000010

%define WGL_FONT_LINES      0
%define WGL_FONT_POLYGONS   1