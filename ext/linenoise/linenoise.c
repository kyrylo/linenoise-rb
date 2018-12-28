#include <ruby.h>
#include <ruby/io.h>
#include <string.h>
#include "line_noise.h"

static VALUE mLinenoise;
static ID id_call, id_multiline, id_hint_bold, id_hint_color, completion_proc,
          hint_proc;
static VALUE hint_boldness;
static int hint_color;

#define COMPLETION_PROC "completion_proc"
#define HINT_PROC "hint_proc"

/* Hint colors */
enum {red = 31, green, yellow, blue, magenta, cyan, white};

static void
mustbe_callable(VALUE proc)
{
    if (!NIL_P(proc) && !rb_respond_to(proc, id_call))
        rb_raise(rb_eArgError, "argument must respond to `call'");
}

static VALUE
linenoise_linenoise(VALUE self, VALUE prompt)
{
    VALUE result;
    char *line;

    line = linenoise(StringValueCStr(prompt));
    if (line) {
        result = rb_locale_str_new_cstr(line);
    }
    else
        result = Qnil;
    if (line) free(line);

    return result;
}

static void
linenoise_attempted_completion_function(const char *buf, struct linenoiseCompletions *lc)
{
    VALUE proc, ary, str;
    long i, matches;
    rb_encoding *enc;
    VALUE encobj;

    proc = rb_attr_get(mLinenoise, completion_proc);
    if (NIL_P(proc))
        return;

    ary = rb_funcall(proc, id_call, 1, rb_locale_str_new_cstr(buf));
    if (!RB_TYPE_P(ary, T_ARRAY))
        ary = rb_Array(ary);

    matches = RARRAY_LEN(ary);
    if (matches == 0)
        return;

    enc = rb_locale_encoding();
    encobj = rb_enc_from_encoding(enc);
    for (i = 0; i < matches; i++) {
        str = rb_obj_as_string(RARRAY_AREF(ary, i));
        StringValueCStr(str);
        rb_enc_check(encobj, str);
        linenoiseAddCompletion(lc, RSTRING_PTR(str));
    }
}

static VALUE
linenoise_set_completion_proc(VALUE self, VALUE proc)
{
    mustbe_callable(proc);
    linenoiseSetCompletionCallback(linenoise_attempted_completion_function);
    return rb_ivar_set(mLinenoise, completion_proc, proc);
}

static VALUE
linenoise_get_completion_proc(VALUE self)
{
    return rb_attr_get(mLinenoise, completion_proc);
}

static VALUE
linenoise_set_multiline(VALUE self, VALUE vbool)
{
    int i = RTEST(vbool) ? 1 : 0;
    rb_ivar_set(mLinenoise, id_multiline, vbool);
    linenoiseSetMultiLine(i);
    return vbool;
}

static VALUE
linenoise_get_multiline(VALUE self)
{
    return rb_attr_get(mLinenoise, id_multiline);
}

static char *
linenoise_attempted_hint_function(const char *buf, int *color, int *bold)
{
    VALUE proc, str, encobj;
    rb_encoding *enc;

    *bold = RTEST(hint_boldness) ? 1 : 0;
    *color = hint_color;

    proc = rb_attr_get(mLinenoise, hint_proc);
    if (NIL_P(proc))
        return NULL;

    str = rb_funcall(proc, id_call, 1, rb_locale_str_new_cstr(buf));
    enc = rb_locale_encoding();
    encobj = rb_enc_from_encoding(enc);
    StringValueCStr(str);
    rb_enc_check(encobj, str);

    return RSTRING_PTR(str);
}

static VALUE
linenoise_set_hint_proc(VALUE self, VALUE proc)
{
    mustbe_callable(proc);
    linenoiseSetHintsCallback(linenoise_attempted_hint_function);
    return rb_ivar_set(mLinenoise, hint_proc, proc);
}

static VALUE
linenoise_get_hint_proc(VALUE self)
{
    return rb_attr_get(mLinenoise, hint_proc);
}

static VALUE
linenoise_set_hint_color(VALUE self, VALUE color)
{
    int c = 0;

    switch (TYPE(color)) {
      case T_NIL:
        break;
      case T_FIXNUM:
        c = NUM2INT(color);
        break;
      default:
        rb_raise(rb_eTypeError, "hint color is not an Integer");
    }

    if (c == 0 || (c >= 31 && c <= 37)) {
        hint_color = c;
    }
    else
        rb_raise(rb_eArgError, "color '%d' is not in range (31-37)", c);

    return rb_ivar_set(mLinenoise, id_hint_color, color);
}

static VALUE
linenoise_get_hint_color(VALUE self)
{
    return rb_attr_get(mLinenoise, id_hint_color);
}

static VALUE
linenoise_set_hint_boldness(VALUE self, VALUE boldness)
{
    hint_boldness = boldness;
    return rb_ivar_set(mLinenoise, id_hint_bold, boldness);
}

static VALUE
linenoise_get_hint_boldness(VALUE self)
{
    return rb_attr_get(mLinenoise, id_hint_bold);
}

static VALUE
hist_set_max_len(VALUE self, VALUE len)
{
    linenoiseHistorySetMaxLen(NUM2INT(len));
    return len;
}

static VALUE
hist_push(VALUE self, VALUE str)
{
    linenoiseHistoryAdd(StringValueCStr(str));
    return self;
}

static VALUE
hist_push_method(int argc, VALUE *argv, VALUE self)
{
    VALUE str;

    while (argc--) {
        str = *argv++;
        linenoiseHistoryAdd(StringValueCStr(str));
    }
    return self;
}

static VALUE
hist_save(VALUE self, VALUE filename)
{
    char *file = StringValueCStr(filename);

    if (linenoiseHistorySave(file) == -1) {
        rb_raise(rb_eArgError,
                 "couldn't save Linenoise history to file '%s'", file);
    }
    return filename;
}


static VALUE
hist_load(VALUE self, VALUE filename)
{
    char *file = StringValueCStr(filename);

    if (linenoiseHistoryLoad(file) == -1) {
        rb_raise(rb_eArgError,
                 "couldn't load Linenoise history from file '%s'", file);
    }
    return filename;
}

static VALUE
hist_length(VALUE self)
{
    return INT2NUM(linenoiseHistorySize());
}

static VALUE
hist_clear(VALUE self)
{
    linenoiseHistoryClear();
    return self;
}

void
Init_linenoise(void)
{
    VALUE history;

    id_call = rb_intern("call");
    id_multiline = rb_intern("multiline");
    id_hint_bold = rb_intern("hint_bold");
    id_hint_color = rb_intern("hint_color");

    completion_proc = rb_intern(COMPLETION_PROC);
    hint_proc = rb_intern(HINT_PROC);

    mLinenoise = rb_define_module("Linenoise");
    /* Version string of Linenoise. */
    rb_define_const(mLinenoise, "VERSION", rb_str_new_cstr("1.0"));
    rb_define_module_function(mLinenoise, "linenoise",
                              linenoise_linenoise, 1);
    rb_define_alias(rb_singleton_class(mLinenoise), "readline", "linenoise");
    rb_define_singleton_method(mLinenoise, "completion_proc=",
                               linenoise_set_completion_proc, 1);
    rb_define_singleton_method(mLinenoise, "completion_proc",
                               linenoise_get_completion_proc, 0);
    rb_define_singleton_method(mLinenoise, "multiline=",
                               linenoise_set_multiline, 1);
    rb_define_singleton_method(mLinenoise, "multiline?",
                               linenoise_get_multiline, 0);
    rb_define_singleton_method(mLinenoise, "hint_proc=",
                               linenoise_set_hint_proc, 1);
    rb_define_singleton_method(mLinenoise, "hint_proc",
                               linenoise_get_hint_proc, 0);
    rb_define_singleton_method(mLinenoise, "hint_color=",
                               linenoise_set_hint_color, 1);
    rb_define_singleton_method(mLinenoise, "hint_color",
                               linenoise_get_hint_color, 0);
    rb_define_singleton_method(mLinenoise, "hint_bold=",
                               linenoise_set_hint_boldness, 1);
    rb_define_singleton_method(mLinenoise, "hint_bold?",
                               linenoise_get_hint_boldness, 0);

    history = rb_obj_alloc(rb_cObject);
    rb_extend_object(history, rb_mEnumerable);
    rb_define_singleton_method(history, "max_len=", hist_set_max_len, 1);
    rb_define_singleton_method(history, "<<", hist_push, 1);
    rb_define_singleton_method(history, "push", hist_push_method, -1);
    rb_define_singleton_method(history, "save", hist_save, 1);
    rb_define_singleton_method(history, "load", hist_load, 1);
    rb_define_singleton_method(history, "size", hist_length, 0);
    rb_define_singleton_method(history, "clear", hist_clear, 0);

    /*
     * The history buffer. It extends Enumerable module, so it behaves just like
     * an array.  For example, gets the fifth content that the user input by
     * HISTORY[4].
     */
    rb_define_const(mLinenoise, "HISTORY", history);

    /* Hint color helpers */
    rb_define_const(mLinenoise, "DEFAULT", Qnil);
    rb_define_const(mLinenoise, "RED", INT2NUM(red));
    rb_define_const(mLinenoise, "GREEN", INT2NUM(green));
    rb_define_const(mLinenoise, "YELLOW", INT2NUM(yellow));
    rb_define_const(mLinenoise, "BLUE", INT2NUM(blue));
    rb_define_const(mLinenoise, "MAGENTA", INT2NUM(magenta));
    rb_define_const(mLinenoise, "CYAN", INT2NUM(cyan));
    rb_define_const(mLinenoise, "WHITE", INT2NUM(white));

    rb_funcall(mLinenoise, rb_intern("multiline="), 1, Qtrue);
    rb_funcall(mLinenoise, rb_intern("hint_color="), 1, Qnil);
    rb_funcall(mLinenoise, rb_intern("hint_bold="), 1, Qfalse);
}
