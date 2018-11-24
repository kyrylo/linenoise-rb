#include <ruby.h>
#include "line_noise.h"

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

void
Init_linenoise(void)
{
  VALUE mLinenoise = rb_define_module("Linenoise");
  rb_define_module_function(mLinenoise, "linenoise",
			    linenoise_linenoise, 1);
  rb_define_alias(rb_singleton_class(mLinenoise), "readline", "linenoise");

  /* Version string of Linenoise. */
  rb_define_const(mLinenoise, "VERSION", rb_str_new_cstr("1.0"));

  /* Gem version */
  rb_define_const(mLinenoise, "GEM_VERSION", rb_str_new_cstr("0.0.1"));
}
