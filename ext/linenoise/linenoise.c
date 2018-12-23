#include <ruby.h>
#include <ruby/io.h>
#include <string.h>
#include "line_noise.h"

static VALUE mLinenoise;
static ID id_call, completion_proc;

#define COMPLETION_PROC "completion_proc"

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
    if (!NIL_P(proc) && !rb_respond_to(proc, id_call)) {
        rb_raise(rb_eArgError, "argument must respond to `call'");
    }
    linenoiseSetCompletionCallback(linenoise_attempted_completion_function);
    return rb_ivar_set(mLinenoise, completion_proc, proc);
}

static VALUE
linenoise_get_completion_proc(VALUE self)
{
    return rb_attr_get(mLinenoise, completion_proc);
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
    completion_proc = rb_intern(COMPLETION_PROC);

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
}
