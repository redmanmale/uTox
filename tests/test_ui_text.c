#include "test.h"
#include "mock/mock_ui.h"

#include <stdio.h>
#include <string.h>

#include "../src/ui/text.h"
#include "../src/ui/scrollable.h"

#include "../src/text.c"
#include "../src/ui/text.c"

bool test_ui_text_height_and_hit(void) {
    mock_ui_reset();
    const uint16_t lh = 12;

    if (text_height(100, lh, "", 0) != lh) {
        FAIL("empty text is one line");
    }
    if (text_height(100, lh, "hello", 5) != lh) {
        FAIL("short unwrapped line");
    }

    char nl[] = "ab\ncd";
    if (text_height(100, lh, nl, 5) != 2 * lh) {
        FAIL("newline should add a line");
    }

    char wrap[] = "abcdef";
    if (text_height(3, lh, wrap, 6) != 2 * lh) {
        FAIL("1px/char wrap at width 3 should be two lines, got %d", text_height(3, lh, wrap, 6));
    }

    if (hittextmultiline(0, 100, -1, 100, lh, "hello", 5, true) != 0) {
        FAIL("above box -> 0");
    }
    if (hittextmultiline(0, 100, 100, 100, lh, "hello", 5, true) != 5) {
        FAIL("below box -> length");
    }

    uint16_t at2 = hittextmultiline(2, 100, 0, 100, lh, "hello", 5, true);
    if (at2 != 2) {
        FAIL("hit x=2 on 1px/char should be index 2, got %u", at2);
    }

    uint16_t wrapped = hittextmultiline(1, 3, lh, 100, lh, wrap, 6, true);
    if (wrapped != 4) {
        FAIL("second wrapped line x=1 should be index 4, got %u", wrapped);
    }

    uint16_t at_nl = hittextmultiline(0, 100, 0, 100, lh, nl, 5, true);
    if (at_nl != 0) {
        FAIL("first line of ab\\ncd, got %u", at_nl);
    }
    uint16_t second = hittextmultiline(0, 100, lh, 100, lh, nl, 5, true);
    if (second != 3) {
        FAIL("start of second line should be 3, got %u", second);
    }

    /* "0000" fits in width 6. The URL wraps, so the gap on the first line is empty. */
    char gap[] = "0000 http://abcdefghij";
    uint16_t gap_hit = hittextmultiline(5, 6, 0, 100, lh, gap, (uint16_t)strlen(gap), true);
    if (gap_hit > 4) {
        FAIL("gap after text must not hit the wrapped URL, got %u", gap_hit);
    }
    uint16_t url_hit = hittextmultiline(1, 6, lh, 100, lh, gap, (uint16_t)strlen(gap), true);
    if (url_hit < 5) {
        FAIL("wrapped URL text should still hit, got %u", url_hit);
    }
    return true;
}

bool test_ui_text_lineup_linedown(void) {
    mock_ui_reset();
    SCROLLABLE scroll;
    memset(&scroll, 0, sizeof scroll);
    char str[] = "ab\ncd";
    const uint16_t lh = 12;

    uint16_t down = text_linedown(100, 40, 0, lh, str, 5, &scroll);
    if (down < 3) {
        FAIL("linedown from start should reach second line, got %u", down);
    }

    uint16_t up = text_lineup(100, 40, 4, lh, str, 5, &scroll);
    if (up >= 3) {
        FAIL("lineup from end should reach first line, got %u", up);
    }

    uint16_t top = text_lineup(100, 40, 0, lh, str, 5, &scroll);
    if (top != 0 || scroll.d != 0.0) {
        FAIL("lineup at top stays at 0");
    }
    return true;
}

bool test_ui_text_draw_smoke(void) {
    mock_ui_reset();
    const uint16_t lh = 12;
    mock_draw_text_count = 0;

    utox_draw_text_multiline_within_box(0, 0, 80, 0, 80, lh, "hello", 5, 1, 2, 0, 0, false);
    utox_draw_text_multiline_within_box(0, 0, 80, 0, 80, lh, ">quote", 6, 0, 0, 0, 0, true);
    utox_draw_text_multiline_within_box(0, 0, 120, 0, 80, lh, "http://x https://y tox:z", 24, 0, 0, 0, 0, true);
    utox_draw_text_multiline_within_box(0, 0, 80, 0, 80, lh, "red<\nnext", 9, 0, 0, 1, 1, true);

    if (mock_draw_text_count < 1) {
        FAIL("draw helpers should call drawtext");
    }
    return true;
}

/* Issue 768: a message that begins with '\n' made the red-text scan read the
 * byte before the buffer. A '<' planted there used to paint the line red. */
bool test_ui_text_leading_newline(void) {
    mock_ui_reset();
    COLOR_MAIN_TEXT_RED = 0x00FF0001;
    const uint16_t lh = 12;

    char only_nl[] = "<\n";
    char *nl       = only_nl + 1;
    if (text_height(100, lh, nl, 1) != 2 * lh) {
        FAIL("lone leading newline should be two lines, got %d", text_height(100, lh, nl, 1));
    }
    utox_draw_text_multiline_within_box(0, 0, 80, 0, 80, lh, nl, 1, 0, 0, 0, 0, true);
    if (mock_setcolor_red_count != 0) {
        FAIL("newline-only message must not turn red from the byte before it, got %d",
             mock_setcolor_red_count);
    }

    mock_ui_reset();
    COLOR_MAIN_TEXT_RED = 0x00FF0001;
    char hello[] = "<\nhello";
    char *msg    = hello + 1;
    if (text_height(100, lh, msg, 6) != 2 * lh) {
        FAIL("\\nhello should be two lines, got %d", text_height(100, lh, msg, 6));
    }
    uint16_t second = hittextmultiline(0, 100, lh, 100, lh, msg, 6, true);
    if (second != 1) {
        FAIL("second line of \\nhello should start at 1, got %u", second);
    }
    int y = utox_draw_text_multiline_within_box(0, 0, 80, 0, 80, lh, msg, 6, 0, 0, 0, 0, true);
    if (y != 2 * lh) {
        FAIL("draw height of \\nhello should be two lines, got %d", y);
    }
    if (mock_setcolor_red_count != 0) {
        FAIL("leading newline must not turn red from the byte before the message, got %d",
             mock_setcolor_red_count);
    }

    /* The line after the leading newline really does end with '<'. That is one
     * red set. The byte before the buffer must not add a second one. */
    mock_ui_reset();
    COLOR_MAIN_TEXT_RED = 0x00FF0001;
    char red[] = "<\nline<";
    char *redmsg = red + 1;
    utox_draw_text_multiline_within_box(0, 0, 80, 0, 80, lh, redmsg, 6, 0, 0, 0, 0, true);
    if (mock_setcolor_red_count != 1) {
        FAIL("line ending in < after a leading newline should set red once, got %d",
             mock_setcolor_red_count);
    }
    return true;
}

int main(void) {
    int result = 0;
    RUN_TEST(test_ui_text_height_and_hit)
    RUN_TEST(test_ui_text_lineup_linedown)
    RUN_TEST(test_ui_text_draw_smoke)
    RUN_TEST(test_ui_text_leading_newline)
    return result;
}
