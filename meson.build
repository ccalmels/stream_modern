project(
  'stream',
  'cpp', default_options : [ 'cpp_std=c++20', 'werror=true', 'warning_level=3' ],
  version : '0.0.1',
)

incdir = include_directories('include')

lib = library(
  'stream', sources : [
    'src/stream.cpp',
  ],
  include_directories : incdir
)


stream_dep = declare_dependency(
  include_directories : incdir,
  link_with : lib,
)


# tests
gmock_dep = dependency('gmock_main')

e = executable(
  'test_stream', sources : [
    'tests/test_stream.cpp',
  ],
  dependencies : [ gmock_dep, stream_dep ],
)

test('gtest test', e, args : '--gtest_output=xml:report.xml')
