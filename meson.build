project('openjpeg', 'c', version: '2.5.3', default_options: ['warning_level=3'])

includes = include_directories('.')
conf = configuration_data()

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required: true)

conf.set('OPENJPEG_VERSION_MAJOR', 2)
conf.set('OPENJPEG_VERSION_MINOR', 5)
conf.set('OPENJPEG_VERSION_BUILD', 3)
conf.set('PACKAGE_VERSION', '2.5.3')
conf.set('HAVE_INTTYPES_H', cc.has_header('inttypes.h'))
conf.set('HAVE_STDLIB_H', cc.has_header('stdlib.h'))
conf.set('HAVE_MALLOC_H', cc.has_header('malloc.h'))
conf.set('HAVE_STDINT_H', cc.has_header('stdint.h'))
conf.set('HAVE_SYS_TYPES_H', cc.has_header('sys/types.h'))
conf.set('HAVE_SYS_STAT_H', cc.has_header('sys/stat.h'))
conf.set('HAVE_UNISTD_H', cc.has_header('unistd.h'))
conf.set('HAVE_STRING_H', cc.has_header('string.h'))
conf.set('HAVE_STRINGS_H', cc.has_header('strings.h'))
conf.set('HAVE_MEMORY_H', cc.has_header('memory.h'))

configure_file(
  input: 'opj_config_private.h.cmake.in',
  output: 'opj_config_private.h',
  configuration: conf,
  format: 'cmake@',
)

configure_file(
  input: 'opj_config.h.cmake.in',
  output: 'opj_config.h',
  configuration: conf,
  format: 'cmake@',
)

both_libraries(
  'openjpeg',
  'bio.c',
  'cio.c',
  'dwt.c',
  'event.c',
  'function_list.c',
  'ht_dec.c',
  'image.c',
  'invert.c',
  'j2k.c',
  'jp2.c',
  'mct.c',
  'mqc.c',
  'openjpeg.c',
  'opj_clock.c',
  'opj_malloc.c',
  'pi.c',
  'sparse_array.c',
  't1.c',
  't2.c',
  'tcd.c',
  'tgt.c',
  'thread.c',
  include_directories: includes,
  dependencies: [m_dep],
)