bin_PROGRAMS = sparqlsim

sparqlsim_SOURCES = centeredstrongsimulation.cc \ centeredstrongsimulation.h \
					cmdline.c cmdline.h \
					cmdline-parser.yy cmdline-lexer.ll cmdline-parser.h \
					graph-parser.yy graph-lexer.ll \
					utils.h utils.cc \
					node.h node.cc \
					label.h label.cc \
					graph.h graph.cc \
					graph.io.h graph.io.cc \
					simulations.h simulation.h simulation.cc \
					hhk.h hhk.cc \
					maetal.h maetal.cc \
					strongsimulation.h strongsimulation.cc \
					allds.h allds.cc \
					variable.h variable.cc \
					smatrix.h smatrix.cc \
					reporter.h reporter.cc \
					main.cc

sparqlsim_CPPFLAGS = -I../resources/bm/src

BUILT_SOURCES = cmdline.c
EXTRA_DIST =
MAINTAINERCLEANFILES =

AM_CXXFLAGS = -std=c++11 -pthread

cmdline.c: cmdline.ggo
		gengetopt --input=$<

#--------------------------------------------------------------#
# GENERIC: organize distribution and cleaning of Bison sources #
#--------------------------------------------------------------#

BISON_FILES = $(filter %.yy,$($(PACKAGE)_SOURCES))
BUILT_SOURCES += $(BISON_FILES:.yy=.cc)
EXTRA_DIST += $(BISON_FILES:.yy=.h)
MAINTAINERCLEANFILES += $(BISON_FILES:.yy=.h)

#----------------------------------------------------------#
# GENERIC: use GNU gengetopt to create command line parser #
#----------------------------------------------------------#
.ggo.c:
	$(GENGETOPT) --input=$<
GENGETOPT_FILES = cmdline.ggo
BUILT_SOURCES += $(GENGETOPT_FILES:.ggo=.c)
EXTRA_DIST += $(GENGETOPT_FILES)
MAINTAINERCLEANFILES += $(GENGETOPT_FILES:.ggo=.c) $(GENGETOPT_FILES:.ggo=.h)
