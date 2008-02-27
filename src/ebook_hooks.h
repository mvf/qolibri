/***************************************************************************
*   Copyright (C) 2007 by BOP                                             *
*   polepolek@gmail.com                                                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef EBOOK_HOOKS_H
#define EBOOK_HOOKS_H

#include "ebook.h"
#include <eb/eb.h>
#include <eb/text.h>

#define HookFunc(name) \
    EB_Error_Code name(EB_Book *, EB_Appendix *, void*, EB_Hook_Code, int, \
                       const unsigned int*)

HookFunc( hook_newline );
HookFunc( hook_iso8859_1 );
HookFunc( hook_narrow_jisx0208 );
HookFunc( hook_wide_jisx0208 );
HookFunc( hook_gb2312 );
HookFunc( hook_set_indent );
HookFunc( hook_begin_narrow );
HookFunc( hook_end_narrow );
HookFunc( hook_begin_subscript );
HookFunc( hook_end_subscript );
HookFunc( hook_begin_superscript );
HookFunc( hook_end_superscript );
HookFunc( hook_begin_emphasize );
HookFunc( hook_end_emphasize );
HookFunc( hook_begin_candidate );
HookFunc( hook_begin_candidate_menu );
HookFunc( hook_end_candidate_leaf );
HookFunc( hook_end_candidate_group );
HookFunc( hook_end_candidate_group_menu );
HookFunc( hook_begin_reference );
HookFunc( hook_end_reference );
HookFunc( hook_begin_keyword );
HookFunc( hook_end_keyword );
HookFunc( hook_begin_decoration );
HookFunc( hook_end_decoration );
HookFunc( hook_begin_mono_graphic );
HookFunc( hook_end_mono_graphic );
HookFunc( hook_begin_color_bmp );
HookFunc( hook_begin_color_jpeg );
HookFunc( hook_end_color_graphic );
HookFunc( hook_begin_in_color_bmp );
HookFunc( hook_begin_in_color_jpeg );
HookFunc( hook_end_in_color_graphic );
HookFunc( hook_begin_mpeg );
HookFunc( hook_end_mpeg );
HookFunc( hook_narrow_font );
HookFunc( hook_wide_font );
HookFunc( hook_begin_wave );
HookFunc( hook_end_wave );

const EB_Hook hooks[] = {
    //{EB_HOOK_NEWLINE,  hook_newline},
    //{EB_HOOK_ISO8859_1,  hook_iso8859_1},
    { EB_HOOK_NARROW_JISX0208,      hook_narrow_jisx0208      },
    //{ EB_HOOK_WIDE_JISX0208,        hook_wide_jisx0208        },
    //{EB_HOOK_GB2312,  hook_gb2312},
    //{EB_HOOK_BEGIN_NARROW,  hook_begin_narrow},
    //{EB_HOOK_END_NARROW,  hook_end_narrow},
    { EB_HOOK_SET_INDENT,           hook_set_indent           },
    { EB_HOOK_BEGIN_SUBSCRIPT,      hook_begin_subscript      },
    { EB_HOOK_END_SUBSCRIPT,        hook_end_subscript        },
    { EB_HOOK_BEGIN_SUPERSCRIPT,    hook_begin_superscript    },
    { EB_HOOK_END_SUPERSCRIPT,      hook_end_superscript      },
    { EB_HOOK_BEGIN_EMPHASIS,       hook_begin_emphasize      },
    { EB_HOOK_END_EMPHASIS,         hook_end_emphasize        },
    { EB_HOOK_NARROW_FONT,          hook_narrow_font          },
    { EB_HOOK_WIDE_FONT,            hook_wide_font            },
    { EB_HOOK_BEGIN_CANDIDATE,      hook_begin_candidate      },
    { EB_HOOK_END_CANDIDATE_LEAF,   hook_end_candidate_leaf   },
    { EB_HOOK_END_CANDIDATE_GROUP,  hook_end_candidate_group  },
    { EB_HOOK_BEGIN_REFERENCE,      hook_begin_reference      },
    { EB_HOOK_END_REFERENCE,        hook_end_reference        },
    //{EB_HOOK_BEGIN_KEYWORD,  hook_begin_keyword},
    //{EB_HOOK_END_KEYWORD,  hook_end_keyword},
    { EB_HOOK_BEGIN_DECORATION,     hook_begin_decoration     },
    { EB_HOOK_END_DECORATION,       hook_end_decoration       },
    { EB_HOOK_BEGIN_MONO_GRAPHIC,   hook_begin_mono_graphic   },
    { EB_HOOK_END_MONO_GRAPHIC,     hook_end_mono_graphic     },
    { EB_HOOK_BEGIN_COLOR_BMP,      hook_begin_color_bmp      },
    { EB_HOOK_BEGIN_COLOR_JPEG,     hook_begin_color_jpeg     },
    { EB_HOOK_END_COLOR_GRAPHIC,    hook_end_color_graphic    },
    { EB_HOOK_BEGIN_IN_COLOR_BMP,   hook_begin_in_color_bmp   },
    { EB_HOOK_BEGIN_IN_COLOR_JPEG,  hook_begin_in_color_jpeg  },
    { EB_HOOK_END_IN_COLOR_GRAPHIC, hook_end_in_color_graphic },
    { EB_HOOK_BEGIN_MPEG,           hook_begin_mpeg           },
    { EB_HOOK_END_MPEG,             hook_end_mpeg             },
    { EB_HOOK_BEGIN_WAVE,           hook_begin_wave           },
    { EB_HOOK_END_WAVE,             hook_end_wave             },
    { EB_HOOK_NULL,                 NULL                      }
};

const EB_Hook hooks_cand[] = {
    //{EB_HOOK_NEWLINE,  hook_newline},
    //{EB_HOOK_ISO8859_1,  hook_iso8859_1},
    { EB_HOOK_NARROW_JISX0208,      hook_narrow_jisx0208          },
    //{ EB_HOOK_WIDE_JISX0208,        hook_wide_jisx0208            },
    //{EB_HOOK_GB2312,  hook_gb2312},
    //{EB_HOOK_BEGIN_NARROW,  hook_begin_narrow},
    //{EB_HOOK_END_NARROW,  hook_end_narrow},
    { EB_HOOK_SET_INDENT,           hook_set_indent               },
    { EB_HOOK_BEGIN_SUBSCRIPT,      hook_begin_subscript          },
    { EB_HOOK_END_SUBSCRIPT,        hook_end_subscript            },
    { EB_HOOK_BEGIN_SUPERSCRIPT,    hook_begin_superscript        },
    { EB_HOOK_END_SUPERSCRIPT,      hook_end_superscript          },
    { EB_HOOK_BEGIN_EMPHASIS,       hook_begin_emphasize          },
    { EB_HOOK_END_EMPHASIS,         hook_end_emphasize            },
    { EB_HOOK_NARROW_FONT,          hook_narrow_font              },
    { EB_HOOK_WIDE_FONT,            hook_wide_font                },
    { EB_HOOK_BEGIN_CANDIDATE,      hook_begin_candidate_menu     },
    //{EB_HOOK_END_CANDIDATE_LEAF,  hook_end_candidate_leaf},
    { EB_HOOK_END_CANDIDATE_GROUP,  hook_end_candidate_group_menu },
    { EB_HOOK_BEGIN_REFERENCE,      hook_begin_reference          },
    { EB_HOOK_END_REFERENCE,        hook_end_reference            },
    //{EB_HOOK_BEGIN_KEYWORD,  hook_begin_keyword},
    //{EB_HOOK_END_KEYWORD,  hook_end_keyword},
    { EB_HOOK_BEGIN_DECORATION,     hook_begin_decoration         },
    { EB_HOOK_END_DECORATION,       hook_end_decoration           },
    { EB_HOOK_BEGIN_MONO_GRAPHIC,   hook_begin_mono_graphic       },
    { EB_HOOK_END_MONO_GRAPHIC,     hook_end_mono_graphic         },
    { EB_HOOK_BEGIN_COLOR_BMP,      hook_begin_color_bmp          },
    { EB_HOOK_BEGIN_COLOR_JPEG,     hook_begin_color_jpeg         },
    { EB_HOOK_END_COLOR_GRAPHIC,    hook_end_color_graphic        },
    { EB_HOOK_BEGIN_IN_COLOR_BMP,   hook_begin_in_color_bmp       },
    { EB_HOOK_BEGIN_IN_COLOR_JPEG,  hook_begin_in_color_jpeg      },
    { EB_HOOK_END_IN_COLOR_GRAPHIC, hook_end_in_color_graphic     },
    { EB_HOOK_BEGIN_MPEG,           hook_begin_mpeg               },
    { EB_HOOK_END_MPEG,             hook_end_mpeg                 },
    { EB_HOOK_BEGIN_WAVE,           hook_begin_wave               },
    { EB_HOOK_END_WAVE,             hook_end_wave                 },
    { EB_HOOK_NULL,                 NULL                          }
};
const EB_Hook hooks_font[] = {
    { EB_HOOK_NARROW_FONT,          hook_narrow_font              },
    { EB_HOOK_WIDE_FONT,            hook_wide_font                },
    { EB_HOOK_NULL,                 NULL                          }
};

#endif
