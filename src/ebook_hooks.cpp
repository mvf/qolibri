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

#include <QtCore>

#include "ebook.h"
#include "ebook_hooks.h"
#include <eb/eb.h>

/*
   EB_Error_Code hook_newline(EB_Book *book, EB_Appendix*, void*,
                           EB_Hook_Code, int, const unsigned int*)
   {
    //qDebug() << "Begin Narrow";
    eb_write_text_string(book, "<br>");
    return 0;
   }

   EB_Error_Code hook_iso8859_1(EB_Book*, EB_Appendix*, void*, EB_Hook_Code,
                             int, const unsigned int*)
   {

    qDebug() << "HOOK ISO8859_1";
    return 0;
   }
 */

EB_Error_Code hook_narrow_jisx0208(EB_Book* book, EB_Appendix* appendix,
                                   void* container, EB_Hook_Code code, int argc,
                                   const unsigned int *argv)
{
    if (*argv == 41443) {
        eb_write_text_string(book, "&lt;");
    } else if (*argv == 41444) {
        eb_write_text_string(book, "&gt;");
    } else if (*argv == 41461) {
        eb_write_text_string(book, "&amp;");
    } else {
        eb_hook_euc_to_ascii(book, appendix, container, code, argc, argv);
    }
    return 0;
}

/*
EB_Error_Code hook_wide_jisx0208(EB_Book *book, EB_Appendix*, void*,
                                    EB_Hook_Code, int,
                                    const unsigned int *argv)
{
    //qDebug() << "HOOK WIDE_JISx0208";
    return 0;
}
   EB_Error_Code hook_gb2312(EB_Book *book, EB_Appendix*, void*, EB_Hook_Code,
                          int, const unsigned int*)
   {

    qDebug() << "HOOK gb2312";
    eb_write_text_string(book,"<em class=err>gb2312</em>");
    return 0;
   }
 */
/*
   EB_Error_Code hook_begin_narrow(EB_Book*, EB_Appendix*, void*, EB_Hook_Code,
                                int, const unsigned int*)
   {
    //qDebug() << "Begin Narrow";
    //eb_write_text_string(book, "[Begin Narrow]");
    //eb_write_text_string(book, "<span>");
    return 0;
   }

   EB_Error_Code hook_end_narrow(EB_Book*, EB_Appendix*, void*, EB_Hook_Code,
   int,
                              const unsigned int*)
   {
    //qDebug() << "End Narrow";
    //eb_write_text_string(book, "[End Narrow]");
    //eb_write_text_string(book, "</span>");
    return 0;
   }
 */

EB_Error_Code hook_set_indent(EB_Book *book, EB_Appendix*, void *container,
                                   EB_Hook_Code, int, const unsigned int *argv)
{
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->set_indent(argv[1]));
    return 0;
}

EB_Error_Code hook_begin_subscript(EB_Book *book, EB_Appendix*, void *container,
                                   EB_Hook_Code, int, const unsigned int*)
{
    EBook *eb = static_cast<EBook*>(container);

    //eb_write_text_string(book, "<sub>");
    eb_write_text_string(book, eb->begin_subscript());
    return 0;
}

EB_Error_Code hook_end_subscript(EB_Book *book, EB_Appendix*, void *container,
                                 EB_Hook_Code, int, const unsigned int*)
{
    EBook *eb = static_cast<EBook*>(container);

    //eb_write_text_string(book, "</sub>");
    eb_write_text_string(book, eb->end_subscript());
    return 0;
}

EB_Error_Code hook_begin_superscript(EB_Book *book, EB_Appendix*, void*,
                                     EB_Hook_Code, int, const unsigned int*)
{
    eb_write_text_string(book, "<sup>");
    return 0;
}

EB_Error_Code hook_end_superscript(EB_Book *book, EB_Appendix*, void*,
                                   EB_Hook_Code, int, const unsigned int*)
{
    eb_write_text_string(book, "</sup>");
    return 0;
}

EB_Error_Code hook_begin_emphasize(EB_Book *book, EB_Appendix*, void*,
                                   EB_Hook_Code, int, const unsigned int*)
{
    eb_write_text_string(book, "<em>");
    return 0;
}

EB_Error_Code hook_end_emphasize(EB_Book *book, EB_Appendix*, void*,
                                 EB_Hook_Code, int, const unsigned int*)
{
    eb_write_text_string(book, "</em>");
    return 0;
}

EB_Error_Code hook_begin_candidate(EB_Book *book, EB_Appendix*,
                                   void *container, EB_Hook_Code, int,
                                   const unsigned int*)
{
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->begin_candidate());
    return 0;
}

EB_Error_Code hook_begin_candidate_menu(EB_Book *book, EB_Appendix*,
                                   void *container, EB_Hook_Code, int,
                                   const unsigned int*)
{
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->begin_candidate_menu());
    return 0;
}

EB_Error_Code hook_end_candidate_leaf(EB_Book *book, EB_Appendix*, void*,
                                      EB_Hook_Code, int, const unsigned int*)
{
    qDebug() << "end_candidate_leaf";
    eb_write_text_string(book, "</a>");
    return 0;
}
EB_Error_Code hook_end_candidate_group(EB_Book *book, EB_Appendix*,
                                       void *container, EB_Hook_Code, int,
                                       const unsigned int *argv)
{
    //qDebug() << "end_candidate_group";
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->end_candidate_group(argv[1], argv[2]));

    return 0;
}

EB_Error_Code hook_end_candidate_group_menu(EB_Book *book, EB_Appendix*,
                                            void *container, EB_Hook_Code,
                                            int, const unsigned int *argv)
{
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->end_candidate_group_menu(argv[1], argv[2]));

    return 0;
}


EB_Error_Code hook_begin_reference(EB_Book *book, EB_Appendix*,
                                   void *container, EB_Hook_Code, int,
                                   const unsigned int*)
{
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->begin_reference());
    return 0;
}
EB_Error_Code hook_end_reference(EB_Book *book, EB_Appendix*, void *container,
                                 EB_Hook_Code, int, const unsigned int *argv)
{
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->end_reference(argv[1], argv[2]));

    return 0;
}
/*
   EB_Error_Code hook_begin_keyword(EB_Book *book, EB_Appendix*, void*,
                                 EB_Hook_Code, int, const unsigned int*)
   {
    Q_UNUSED(book);
    //eb_write_text_string(book,"<span class=key>");
    return 0;
   }
   EB_Error_Code hook_end_keyword(EB_Book *book, EB_Appendix*, void*,
                               EB_Hook_Code, int, const unsigned int*)
   {
    Q_UNUSED(book);
    //qDebug() << "End Keyword";
    //eb_write_text_string(book,"</span>");
    return 0;
   }
 */
EB_Error_Code hook_begin_decoration(EB_Book *book, EB_Appendix*,
                                    void *container, EB_Hook_Code, int argc,
                                    const unsigned int *argv)
{
    Q_UNUSED(argc);
    //qDebug() << "begin_decoration" << argc << argv[1];
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->begin_decoration(argv[1]));

    return 0;
}
EB_Error_Code hook_end_decoration(EB_Book *book, EB_Appendix*, void *container,
                                  EB_Hook_Code, int, const unsigned int*)
{
    //qDebug() << "end_decoration" << cnt;
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->end_decoration());

    return 0;
}

EB_Error_Code hook_begin_mono_graphic(EB_Book *book, EB_Appendix*,
                                      void *container, EB_Hook_Code, int,
                                      const unsigned int *argv)
{
    EBook *eb = static_cast<EBook*>(container);

    eb->begin_mono_graphic(argv[2], argv[3]);
    eb_write_text_string(book, "\n");
    return 0;
}

EB_Error_Code hook_end_mono_graphic(EB_Book *book, EB_Appendix*,
                                    void *container, EB_Hook_Code, int,
                                    const unsigned int *argv)
{
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->end_mono_graphic(argv[1], argv[2]));

    return 0;
}
EB_Error_Code hook_begin_color_bmp(EB_Book *book, EB_Appendix*,
                                   void *container, EB_Hook_Code, int,
                                   const unsigned int *argv)
{
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, "\n");
    eb_write_text_string(book, eb->begin_color_bmp(argv[2], argv[3]));
    return 0;
}
EB_Error_Code hook_begin_color_jpeg(EB_Book *book, EB_Appendix*,
                                    void *container, EB_Hook_Code, int,
                                    const unsigned int *argv)
{
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, "\n");
    eb_write_text_string(book, eb->begin_color_jpeg(argv[2], argv[3]));

    return 0;
}
EB_Error_Code hook_end_color_graphic(EB_Book *book, EB_Appendix*, void*,
                                     EB_Hook_Code, int, const unsigned int*)
{
    eb_write_text_string(book, "</span>\n");
    return 0;
}

EB_Error_Code hook_begin_in_color_bmp(EB_Book *book, EB_Appendix*,
                                      void *container, EB_Hook_Code, int,
                                      const unsigned int *argv)
{
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->begin_color_bmp(argv[2], argv[3]));
    return 0;
}

EB_Error_Code hook_begin_in_color_jpeg(EB_Book *book, EB_Appendix*,
                                       void *container, EB_Hook_Code, int,
                                       const unsigned int *argv)
{
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->begin_color_jpeg(argv[2], argv[3]));
    return 0;
}

EB_Error_Code hook_end_in_color_graphic(EB_Book *book, EB_Appendix*, void*,
                                        EB_Hook_Code, int, const unsigned int*)
{
    eb_write_text_string(book, "</span>");
    return 0;
}

EB_Error_Code hook_begin_mpeg(EB_Book *book, EB_Appendix*, void *container,
                              EB_Hook_Code, int, const unsigned int*)
{
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->begin_mpeg());

    return 0;
}
EB_Error_Code hook_end_mpeg(EB_Book *book, EB_Appendix*, void *container,
                            EB_Hook_Code, int, const unsigned int *argv)
{
    EBook *eb = static_cast<EBook*>(container);

    eb->end_mpeg(argv + 2);
    eb_write_text_string(book, "</a>");

    return 0;
}

EB_Error_Code hook_narrow_font(EB_Book *book, EB_Appendix*, void *container,
                               EB_Hook_Code, int, const unsigned int *argv)
{
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->narrow_font(argv[0]));
    return 0;
}

EB_Error_Code hook_wide_font(EB_Book *book, EB_Appendix*, void *container,
                             EB_Hook_Code, int, const unsigned int *argv)
{
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->wide_font(argv[0]));
    return 0;
}

EB_Error_Code hook_begin_wave(EB_Book *book, EB_Appendix*, void *container,
                              EB_Hook_Code, int, const unsigned int *argv)
{
    EBook *eb = static_cast<EBook*>(container);

    eb_write_text_string(book, eb->begin_wave(argv[2], argv[3],
                                              argv[4], argv[5]));

    return 0;
}
EB_Error_Code hook_end_wave(EB_Book *book, EB_Appendix*, void*, EB_Hook_Code,
                            int, const unsigned int*)
{
    eb_write_text_string(book, "</a>");
    return 0;
}

