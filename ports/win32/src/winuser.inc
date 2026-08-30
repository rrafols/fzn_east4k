%define CDS_UPDATEREGISTRY  0x00000001
%define CDS_TEST            0x00000002
%define CDS_FULLSCREEN      0x00000004
%define CDS_GLOBAL          0x00000008
%define CDS_SET_PRIMARY     0x00000010
%define CDS_RESET           0x40000000
%define CDS_SETRECT         0x20000000
%define CDS_NORESET         0x10000000

%define WS_OVERLAPPED       0x00000000
%define WS_POPUP            0x80000000
%define WS_CHILD            0x40000000
%define WS_MINIMIZE         0x20000000
%define WS_VISIBLE          0x10000000
%define WS_DISABLED         0x08000000
%define WS_CLIPSIBLINGS     0x04000000
%define WS_CLIPCHILDREN     0x02000000
%define WS_MAXIMIZE         0x01000000
%define WS_CAPTION          0x00C00000    ; /* WS_BORDER | WS_DLGFRAME  */
%define WS_BORDER           0x00800000
%define WS_DLGFRAME         0x00400000
%define WS_VSCROLL          0x00200000
%define WS_HSCROLL          0x00100000
%define WS_SYSMENU          0x00080000
%define WS_THICKFRAME       0x00040000
%define WS_GROUP            0x00020000
%define WS_TABSTOP          0x00010000

%define WS_EX_DLGMODALFRAME     0x00000001
%define WS_EX_NOPARENTNOTIFY    0x00000004
%define WS_EX_TOPMOST           0x00000008
%define WS_EX_ACCEPTFILES       0x00000010
%define WS_EX_TRANSPARENT       0x00000020
%define WS_EX_MDICHILD          0x00000040
%define WS_EX_TOOLWINDOW        0x00000080
%define WS_EX_WINDOWEDGE        0x00000100
%define WS_EX_CLIENTEDGE        0x00000200
%define WS_EX_CONTEXTHELP       0x00000400

%define WS_EX_RIGHT             0x00001000
%define WS_EX_LEFT              0x00000000
%define WS_EX_RTLREADING        0x00002000
%define WS_EX_LTRREADING        0x00000000
%define WS_EX_LEFTSCROLLBAR     0x00004000
%define WS_EX_RIGHTSCROLLBAR    0x00000000

%define WS_EX_CONTROLPARENT     0x00010000
%define WS_EX_STATICEDGE        0x00020000
%define WS_EX_APPWINDOW         0x00040000


%define WS_EX_OVERLAPPEDWINDOW  (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
%define WS_EX_PALETTEWINDOW     (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST)


%define WM_KEYFIRST                     0x0100
%define WM_KEYDOWN                      0x0100
%define WM_KEYUP                        0x0101
%define WM_CHAR                         0x0102
%define WM_DEADCHAR                     0x0103
%define WM_SYSKEYDOWN                   0x0104
%define WM_SYSKEYUP                     0x0105
%define WM_SYSCHAR                      0x0106
%define WM_SYSDEADCHAR                  0x0107
%define WM_KEYLAST                      0x0108