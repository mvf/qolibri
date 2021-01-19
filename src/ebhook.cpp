/***************************************************************************
*   Copyright (C) 2007 by BOP                                             *
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

#include "qeb.h"
#include "ebcore.h"

#include <ebu/eb.h>
#include <ebu/binary.h>
#include <ebu/text.h>
#include <ebu/font.h>
#include <ebu/appendix.h>
#include <ebu/error.h>

#define HOOK_FUNC(code,class_name,function) \
    EB_Error_Code Hook##code(EB_Book *book, EB_Appendix*, \
        void *classp, EB_Hook_Code, int argc, const unsigned int* argv) \
    { \
        class_name *p = static_cast<class_name*>(classp); \
        QByteArray b =  p->function(argc,argv); \
        if (!b.isEmpty()) { \
            return eb_write_text_string(book, b); \
        } else { \
            return EB_SUCCESS; \
        } \
    }


HOOK_FUNC(BEGIN_SUBSCRIPT, EbCore, hookBeginSubscript)
HOOK_FUNC(END_SUBSCRIPT, EbCore, hookEndSubscript)
HOOK_FUNC(BEGIN_SUPERSCRIPT, EbCore, hookBeginSuperscript)
HOOK_FUNC(END_SUPERSCRIPT, EbCore, hookEndSuperscript)
HOOK_FUNC(BEGIN_EMPHASIS, EbCore, hookBeginEmphasis)
HOOK_FUNC(END_EMPHASIS, EbCore, hookEndEmphasis)
HOOK_FUNC(BEGIN_CANDIDATE, EbCore, hookBeginCandidate)
HOOK_FUNC(END_CANDIDATE_GROUP, EbCore, hookEndCandidateGroup)
HOOK_FUNC(END_CANDIDATE_GROUP2, EbCore, hookEndCandidateGroupMENU)
HOOK_FUNC(END_CANDIDATE_LEAF, EbCore, hookEndCandidateLeaf)
HOOK_FUNC(BEGIN_REFERENCE, EbCore, hookBeginReference)
HOOK_FUNC(END_REFERENCE, EbCore, hookEndReference)
HOOK_FUNC(NARROW_FONT, EbCore, hookNarrowFont)
HOOK_FUNC(WIDE_FONT, EbCore, hookWideFont)
HOOK_FUNC(ISO8859_1, EbCore, hookISO8859_1)
HOOK_FUNC(NARROW_JISX0208, EbCore, hookNarrowJISX0208)
HOOK_FUNC(WIDE_JISX0208, EbCore, hookWideJISX0208)
HOOK_FUNC(GB2312, EbCore, hookGB2312)
HOOK_FUNC(BEGIN_MONO_GRAPHIC, EbCore, hookBeginMonoGraphic)
HOOK_FUNC(END_MONO_GRAPHIC, EbCore, hookEndMonoGraphic)
HOOK_FUNC(BEGIN_GRAY_GRAPHIC, EbCore, hookBeginGrayGraphic)
HOOK_FUNC(END_GRAY_GRAPHIC, EbCore, hookEndGrayGraphic)
HOOK_FUNC(BEGIN_COLOR_BMP, EbCore, hookBeginColorBmp)
HOOK_FUNC(BEGIN_COLOR_JPEG, EbCore, hookBeginColorJpeg)
HOOK_FUNC(BEGIN_IN_COLOR_BMP, EbCore, hookBeginInColorBmp)
HOOK_FUNC(BEGIN_IN_COLOR_JPEG, EbCore, hookBeginInColorJpeg)
HOOK_FUNC(END_COLOR_GRAPHIC, EbCore, hookEndColorGraphic)
HOOK_FUNC(END_IN_COLOR_GRAPHIC, EbCore, hookEndInColorGraphic)
HOOK_FUNC(BEGIN_WAVE, EbCore, hookBeginWave)
HOOK_FUNC(END_WAVE, EbCore, hookEndWave)
HOOK_FUNC(BEGIN_MPEG, EbCore, hookBeginMpeg)
HOOK_FUNC(END_MPEG, EbCore, hookEndMpeg)
HOOK_FUNC(BEGIN_GRAPHIC_REFERENCE, EbCore, hookBeginGraphicReference)
HOOK_FUNC(END_GRAPHIC_REFERENCE, EbCore, hookEndGraphicReference)
HOOK_FUNC(BEGIN_DECORATION, EbCore, hookBeginDecoration)
HOOK_FUNC(END_DECORATION, EbCore, hookEndDecoration)
HOOK_FUNC(BEGIN_IMAGE_PAGE, EbCore, hookBeginImagePage)
HOOK_FUNC(END_IMAGE_PAGE, EbCore, hookEndImagePage)
HOOK_FUNC(BEGIN_CLICKABLE_AREA, EbCore, hookBeginClickableArea)
HOOK_FUNC(END_CLICKABLE_AREA, EbCore, hookEndClickableArea)

#define HOOK_S(code) { EB_HOOK_##code, Hook##code }

EB_Hook hooks[] = {
    HOOK_S(BEGIN_SUBSCRIPT),
    HOOK_S(END_SUBSCRIPT),
    HOOK_S(BEGIN_SUPERSCRIPT),
    HOOK_S(END_SUPERSCRIPT),
    HOOK_S(BEGIN_EMPHASIS),
    HOOK_S(END_EMPHASIS),
    HOOK_S(BEGIN_CANDIDATE),
    HOOK_S(END_CANDIDATE_GROUP),
    HOOK_S(END_CANDIDATE_LEAF),
    HOOK_S(BEGIN_REFERENCE),
    HOOK_S(END_REFERENCE),
    HOOK_S(NARROW_FONT),
    HOOK_S(WIDE_FONT),
    HOOK_S(ISO8859_1),
    HOOK_S(NARROW_JISX0208),
    HOOK_S(WIDE_JISX0208),
    HOOK_S(GB2312),
    HOOK_S(BEGIN_MONO_GRAPHIC),
    HOOK_S(END_MONO_GRAPHIC),
    HOOK_S(BEGIN_GRAY_GRAPHIC),
    HOOK_S(END_GRAY_GRAPHIC),
    HOOK_S(BEGIN_COLOR_BMP),
    HOOK_S(BEGIN_COLOR_JPEG),
    HOOK_S(BEGIN_IN_COLOR_BMP),
    HOOK_S(BEGIN_IN_COLOR_JPEG),
    HOOK_S(END_COLOR_GRAPHIC),
    HOOK_S(END_IN_COLOR_GRAPHIC),
    HOOK_S(BEGIN_WAVE),
    HOOK_S(END_WAVE),
    HOOK_S(BEGIN_MPEG),
    HOOK_S(END_MPEG),
    HOOK_S(BEGIN_GRAPHIC_REFERENCE),
    HOOK_S(END_GRAPHIC_REFERENCE),
    HOOK_S(BEGIN_DECORATION),
    HOOK_S(END_DECORATION),
    HOOK_S(BEGIN_IMAGE_PAGE),
    HOOK_S(END_IMAGE_PAGE),
    HOOK_S(BEGIN_CLICKABLE_AREA),
    HOOK_S(END_CLICKABLE_AREA),
    { EB_HOOK_NULL, NULL }
};
EB_Hook hooks_cand[] = {
    HOOK_S(BEGIN_SUBSCRIPT),
    HOOK_S(END_SUBSCRIPT),
    HOOK_S(BEGIN_SUPERSCRIPT),
    HOOK_S(END_SUPERSCRIPT),
    HOOK_S(BEGIN_EMPHASIS),
    HOOK_S(END_EMPHASIS),
    HOOK_S(END_CANDIDATE_LEAF),
    HOOK_S(BEGIN_REFERENCE),
    HOOK_S(END_REFERENCE),
    HOOK_S(NARROW_FONT),
    HOOK_S(WIDE_FONT),
    HOOK_S(ISO8859_1),
    HOOK_S(NARROW_JISX0208),
    HOOK_S(WIDE_JISX0208),
    HOOK_S(GB2312),
    HOOK_S(BEGIN_MONO_GRAPHIC),
    HOOK_S(END_MONO_GRAPHIC),
    HOOK_S(BEGIN_GRAY_GRAPHIC),
    HOOK_S(END_GRAY_GRAPHIC),
    HOOK_S(BEGIN_COLOR_BMP),
    HOOK_S(BEGIN_COLOR_JPEG),
    HOOK_S(BEGIN_IN_COLOR_BMP),
    HOOK_S(BEGIN_IN_COLOR_JPEG),
    HOOK_S(END_COLOR_GRAPHIC),
    HOOK_S(END_IN_COLOR_GRAPHIC),
    HOOK_S(BEGIN_WAVE),
    HOOK_S(END_WAVE),
    HOOK_S(BEGIN_MPEG),
    HOOK_S(END_MPEG),
    HOOK_S(BEGIN_GRAPHIC_REFERENCE),
    HOOK_S(END_GRAPHIC_REFERENCE),
    HOOK_S(BEGIN_DECORATION),
    HOOK_S(END_DECORATION),
    HOOK_S(BEGIN_IMAGE_PAGE),
    HOOK_S(END_IMAGE_PAGE),
    HOOK_S(BEGIN_CLICKABLE_AREA),
    HOOK_S(END_CLICKABLE_AREA),
    { EB_HOOK_END_CANDIDATE_GROUP, HookEND_CANDIDATE_GROUP2 },
    { EB_HOOK_NULL, NULL }
};

EB_Hook hooks_font[] = {
    HOOK_S(NARROW_FONT),
    HOOK_S(WIDE_FONT),
    { EB_HOOK_NULL, NULL }
};

