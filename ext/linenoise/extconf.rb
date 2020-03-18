require 'mkmf'
dir_config('linenoise')
have_header('linenoise.h')
create_makefile('linenoise/linenoise')
