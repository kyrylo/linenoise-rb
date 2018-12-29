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

/*
 * Document-module: Linenoise
 *
 * The Linenoise module provides interface for the Linenoise library, which is a
 * Readline replacement used in Redis, MongoDB, and Android.
 *
 * This module defines a number of methods to facilitate completion and accesses
 * input history from the Ruby interpreter.
 *
 * Linenoise:: https://github.com/antirez/linenoise
 *
 * Reads one inputted line with line edit via the {Linenoise.linenoise} method.
 *
 *   require 'linenoise'
 *
 *   while buf = Linenoise.linenoise('> ')
 *     p buf
 *   end
 *
 * User input can be persisted via the history feature. The history can be
 * accessed through the {Linenoise::HISTORY} constant.
 *
 *   require 'linenoise'
 *
 *   while buf = Linenoise.linenoise('> ')
 *     p Linenoise::HISTORY.to_a
 *     print('-> ', buf, '\n')
 *   end
 *
 * == Using history
 *
 * History can be accessed through {Linenoise::HISTORY}. It can be saved to a
 * file, or loaded from a file.
 *
 * === Adding lines to the history
 *
 *   Linenoise::HISTORY << '1 + 1'
 *
 *   # Or push multiple items.
 *   Linenoise::HISTORY.push('2', '3')
 *   Linenoise::HISTORY.size
 *   #=> 3
 *
 * === Iterating lines & accessing individual entries
 *
 *   # Read a line at given index.
 *   Linenoise::HISTORY[0]
 *   #=> '1 + 1'
 *
 *   # Replace a line in the history with another one.
 *   Linenoise::HISTORY[0] = 'begin'
 *   Linenoise::HISTORY[0]
 *   #=> 'begin'
 *
 *   # Iterate over lines like an array (HISTORY is enumerable).
 *   Linenoise::HISTORY.each { |line| puts line }
 *
 * === Saving & loading
 *
 *   # Save to file.
 *   Linenoise::HISTORY.save('linenoise_history')
 *
 *   # Load from file.
 *   Linenoise::HISTORY.load('linenoise_history')
 *
 *   # Wipe out current history (doesn't delete the file).
 *   Linenoise::HISTORY.clear
 *   Linenoise::HISTORY.size
 *   #=> 0
 *
 * === Setting maximum size of history
 *
 *   # The cap sets how many entries history can hold. When the capacity is
 *   # exceeded, older entries are removed.
 *   Linenoise::HISTORY.max_size = 3
 */

/* Hint colors */
enum {red = 31, green, yellow, blue, magenta, cyan, white};

static void
mustbe_callable(VALUE proc)
{
    if (!NIL_P(proc) && !rb_respond_to(proc, id_call))
        rb_raise(rb_eArgError, "argument must respond to `call'");
}

/*
 * call-seq:
 *   Linenoise.linenoise(prompt) -> string or nil
 *
 * Shows the +prompt+ and reads the inputted line with line editing.
 *
 * Returns nil when the inputted line is empty and user inputs EOF
 * (Presses ^D on UNIX).
 *
 * Aliased as +readline+ for easier integration with Readline-enabled apps.
 */
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

/*
 * call-seq:
 *   Linenoise.completion_proc = proc
 *
 * Specifies a Proc object +proc+ to determine completion behavior. It should
 * take input string and return an array of completion candidates.
 *
 *   require 'linenoise'
 *
 *   LIST = %w[
 *     search download open help history quit url next clear prev past
 *   ].freeze
 *
 *   Linenoise.completion_proc = proc do |input|
 *     LIST.grep(/\A#{Regexp.escape(input)}/)
 *   end
 *
 *   while line = Linenoise.linenoise('> ')
 *     p line
 *   end
 *
 * @raise ArgumentError if +proc+ is not a Proc
 */
static VALUE
linenoise_set_completion_proc(VALUE self, VALUE proc)
{
    mustbe_callable(proc);
    linenoiseSetCompletionCallback(linenoise_attempted_completion_function);
    return rb_ivar_set(mLinenoise, completion_proc, proc);
}

/*
 * call-seq:
 *   Linenoise.completion_proc -> proc
 *
 * Returns the completion Proc object.
 */
static VALUE
linenoise_get_completion_proc(VALUE self)
{
    return rb_attr_get(mLinenoise, completion_proc);
}

/*
 * call-seq:
 *   Linenoise.multiline = bool -> bool
 *
 * Specifies multiline mode. By default, Linenoise uses single line editing,
 * that is, a single row on the screen will be used, and as the user types more,
 * the text will scroll towards left to make room.
 */
static VALUE
linenoise_set_multiline(VALUE self, VALUE vbool)
{
    int i = RTEST(vbool) ? 1 : 0;
    rb_ivar_set(mLinenoise, id_multiline, vbool);
    linenoiseSetMultiLine(i);
    return vbool;
}

/*
 * call-seq:
 *   Linenoise.multiline?
 *
 * Checks if multiline mode is enabled.
 */
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

/*
 * call-seq:
 *   Linenoise.hint_proc = proc
 *
 * Specifies a Proc object +proc+ to determine hint behavior. It should take
 * input string and return the completion according to the input.
 *
 *   require 'linenoise'
 *
 *   Linenoise.hint_proc = proc do |input|
 *     case input
 *     when /git show/
 *       ' [<options>] [<object>...]'
 *     when /git log/
 *       ' [<options>] [<revision range>]'
 *     else
 *       ' --help'
 *     end
 *   end
 *
 *   while line = Linenoise.linenoise('> ')
 *     p line
 *   end
 *
 * @raise ArgumentError if +proc+ is not a Proc
 */
static VALUE
linenoise_set_hint_proc(VALUE self, VALUE proc)
{
    mustbe_callable(proc);
    linenoiseSetHintsCallback(linenoise_attempted_hint_function);
    return rb_ivar_set(mLinenoise, hint_proc, proc);
}

/*
 * call-seq:
 *   Linenoise.hint_proc -> proc
 *
 * Returns the hint Proc object.
 */
static VALUE
linenoise_get_hint_proc(VALUE self)
{
    return rb_attr_get(mLinenoise, hint_proc);
}

/*
 * call-seq:
 *   Linenoise.hint_color = Integer -> Integer
 *
 * Sets the hint color. Allowed values are in a range from 31 to 37. Setting
 * this option to 0 removes the color and uses the default font color.
 *
 * There are convenience constants for setting colors:
 *
 *   # Make the hint red.
 *   Linenoise.hint_color = Linenoise::RED
 *
 *   # Remove the color.
 *   Linenoise.hint_color = Linenoise::DEFAULT
 *
 * @raise TypeError if +color+ is not an Integer
 * @raise ArgumentError if +color+ is not 0 or in range of 31-37
 */
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

/*
 * call-seq:
 *   Linenoise.hint_color -> Integer
 *
 * Checks hint font color.
 */
static VALUE
linenoise_get_hint_color(VALUE self)
{
    return rb_attr_get(mLinenoise, id_hint_color);
}

/*
 * call-seq:
 *   Linenoise.hint_bold = bool -> bool
 *
 * Sets hint boldness. +false+ means normal text, +true+ means bold. Defults to
 * +false+.
 *
 *   Linenoise.hint_bold = true
 */
static VALUE
linenoise_set_hint_boldness(VALUE self, VALUE boldness)
{
    hint_boldness = boldness;
    return rb_ivar_set(mLinenoise, id_hint_bold, boldness);
}

/*
 * call-seq:
 *   Linenoise.hint_bold? -> bool
 *
 * Checks if the hint font is bold.
 */
static VALUE
linenoise_get_hint_boldness(VALUE self)
{
    return rb_attr_get(mLinenoise, id_hint_bold);
}

/*
 * call-seq:
 *   Linenoise.clear_screen -> self
 *
 * Clears screen from characters.
 */
static VALUE
linenoise_clear_screen(VALUE self)
{
    linenoiseClearScreen();
    return self;
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
hist_each(VALUE self)
{
    char *line;
    int i;

    RETURN_ENUMERATOR(self, 0, 0);

    for (i = 0; i < linenoiseHistorySize(); i++) {
        line = linenoiseHistoryGet(i);
        if (line == NULL)
            break;
        rb_yield(rb_locale_str_new_cstr(line));
    }
    return self;
}

static VALUE
hist_get(VALUE self, VALUE index)
{
    char *line = NULL;
    int i;

    i = NUM2INT(index);
    if (i < 0) {
        i += linenoiseHistorySize();
    }
    if (i >= 0) {
        line = linenoiseHistoryGet(i);
    }
    if (line == NULL) {
        rb_raise(rb_eIndexError, "invalid index");
    }
    return rb_locale_str_new_cstr(line);
}

static VALUE
hist_set(VALUE self, VALUE index, VALUE str)
{
    char *old_line = NULL;
    int i;

    i = NUM2INT(index);
    StringValueCStr(str);
    if (i < 0) {
        i += linenoiseHistorySize();
    }
    if (i >= 0) {
        old_line = linenoiseHistoryReplaceLine(i, RSTRING_PTR(str));
    }
    if (old_line == NULL) {
        rb_raise(rb_eIndexError, "invalid index");
    }
    return str;
}

/*
 * call-seq:
 *   Linenoise.hist_clear -> self
 *
 * Clears input history.
 */
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
    rb_define_singleton_method(mLinenoise, "clear_screen",
                               linenoise_clear_screen, 0);

    history = rb_obj_alloc(rb_cObject);
    rb_extend_object(history, rb_mEnumerable);
    rb_define_singleton_method(history, "max_size=", hist_set_max_len, 1);
    rb_define_singleton_method(history, "<<", hist_push, 1);
    rb_define_singleton_method(history, "push", hist_push_method, -1);
    rb_define_singleton_method(history, "save", hist_save, 1);
    rb_define_singleton_method(history, "load", hist_load, 1);
    rb_define_singleton_method(history, "size", hist_length, 0);
    rb_define_singleton_method(history, "clear", hist_clear, 0);
    rb_define_singleton_method(history, "each", hist_each, 0);
    rb_define_singleton_method(history, "[]", hist_get, 1);
    rb_define_singleton_method(history, "[]=", hist_set, 2);

    /*
     * The history buffer. It extends Enumerable module, so it behaves just like
     * an array. For example, gets the fifth content that the user input by
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
